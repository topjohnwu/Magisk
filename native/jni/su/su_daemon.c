/* su_daemon.c - The entrypoint for su, connect to daemon and send correct info
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "magisk.h"
#include "daemon.h"
#include "utils.h"
#include "su.h"
#include "pts.h"
#include "selinux.h"

#define TIMEOUT     3

#define LOCK_CACHE()   pthread_mutex_lock(&cache_lock)
#define LOCK_INFO()    pthread_mutex_lock(&info->lock)
#define UNLOCK_CACHE() pthread_mutex_unlock(&cache_lock)
#define UNLOCK_INFO()  pthread_mutex_unlock(&info->lock)

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
static struct su_info *cache;

static void *info_collector(void *node) {
	struct su_info *info = node;
	while (1) {
		sleep(1);
		if (info->life) {
			LOCK_CACHE();
			if (--info->life == 0 && cache && info->uid == cache->uid)
				cache = NULL;
			UNLOCK_CACHE();
		}
		if (!info->life && !info->ref) {
			pthread_mutex_destroy(&info->lock);
			free(info);
			return NULL;
		}
	}
}

static void database_check(struct su_info *info) {
	int uid = info->uid;
	sqlite3 *db = get_magiskdb();
	if (db) {
		get_db_settings(db, -1, &info->dbs);
		get_db_strings(db, -1, &info->str);

		// Check multiuser settings
		switch (DB_SET(info, SU_MULTIUSER_MODE)) {
			case MULTIUSER_MODE_OWNER_ONLY:
				if (info->uid / 100000) {
					uid = -1;
					info->access = NO_SU_ACCESS;
				}
				break;
			case MULTIUSER_MODE_OWNER_MANAGED:
				uid = info->uid % 100000;
				break;
			case MULTIUSER_MODE_USER:
			default:
				break;
		}

		if (uid > 0)
			get_uid_policy(db, uid, &info->access);
		sqlite3_close_v2(db);
	}

	// We need to check our manager
	if (info->access.log || info->access.notify)
		validate_manager(DB_STR(info, SU_MANAGER), uid / 100000, &info->mgr_st);
}

static struct su_info *get_su_info(unsigned uid) {
	struct su_info *info;
	int cache_miss = 0;

	LOCK_CACHE();

	if (cache && cache->uid == uid) {
		info = cache;
	} else {
		cache_miss = 1;
		info = xcalloc(1, sizeof(*info));
		info->uid = uid;
		info->dbs = DEFAULT_DB_SETTINGS;
		info->access = DEFAULT_SU_ACCESS;
		pthread_mutex_init(&info->lock, NULL);
		cache = info;
	}

	// Update the cache status
	info->life = TIMEOUT;
	++info->ref;

	// Start a thread to maintain the cache
	if (cache_miss) {
		pthread_t thread;
		xpthread_create(&thread, NULL, info_collector, info);
		pthread_detach(thread);
	}

	UNLOCK_CACHE();

	LOGD("su: request from uid=[%d] (#%d)\n", info->uid, ++info->count);

	// Lock before the policy is determined
	LOCK_INFO();

	if (info->access.policy == QUERY) {
		//  Not cached, get data from database
		database_check(info);

		// Check su access settings
		switch (DB_SET(info, ROOT_ACCESS)) {
			case ROOT_ACCESS_DISABLED:
				LOGW("Root access is disabled!\n");
				info->access = NO_SU_ACCESS;
				break;
			case ROOT_ACCESS_ADB_ONLY:
				if (info->uid != UID_SHELL) {
					LOGW("Root access limited to ADB only!\n");
					info->access = NO_SU_ACCESS;
				}
				break;
			case ROOT_ACCESS_APPS_ONLY:
				if (info->uid == UID_SHELL) {
					LOGW("Root access is disabled for ADB!\n");
					info->access = NO_SU_ACCESS;
				}
				break;
			case ROOT_ACCESS_APPS_AND_ADB:
			default:
				break;
		}

		// If it's the manager, allow it silently
		if ((info->uid % 100000) == (info->mgr_st.st_uid % 100000))
			info->access = SILENT_SU_ACCESS;

		// Allow if it's root
		if (info->uid == UID_ROOT)
			info->access = SILENT_SU_ACCESS;

		// If still not determined, check if manager exists
		if (info->access.policy == QUERY && DB_STR(info, SU_MANAGER)[0] == '\0')
			info->access = NO_SU_ACCESS;
	}

	// If still not determined, ask manager
	if (info->access.policy == QUERY) {
		// Create random socket
		struct sockaddr_un addr;
		int sockfd = create_rand_socket(&addr);

		// Connect manager
		app_connect(addr.sun_path + 1, info);
		int fd = socket_accept(sockfd, 60);
		if (fd < 0) {
			info->access.policy = DENY;
		} else {
			socket_send_request(fd, info);
			int ret = read_int_be(fd);
			info->access.policy = ret < 0 ? DENY : ret;
			close(fd);
		}
		close(sockfd);
	}

	// Unlock
	UNLOCK_INFO();

	return info;
}

static void populate_environment(struct su_request *req) {
	struct passwd *pw;

	if (req->keepenv)
		return;

	pw = getpwuid(req->uid);
	if (pw) {
		setenv("HOME", pw->pw_dir, 1);
		if (req->shell)
			setenv("SHELL", req->shell, 1);
		else
			setenv("SHELL", DEFAULT_SHELL, 1);
		if (req->login || req->uid) {
			setenv("USER", pw->pw_name, 1);
			setenv("LOGNAME", pw->pw_name, 1);
		}
	}
}

static void set_identity(unsigned uid) {
	/*
	 * Set effective uid back to root, otherwise setres[ug]id will fail
	 * if uid isn't root.
	 */
	if (seteuid(0)) {
		PLOGE("seteuid (root)");
	}
	if (setresgid(uid, uid, uid)) {
		PLOGE("setresgid (%u)", uid);
	}
	if (setresuid(uid, uid, uid)) {
		PLOGE("setresuid (%u)", uid);
	}
}

void su_daemon_handler(int client, struct ucred *credential) {
	LOGD("su: request from client: %d\n", client);
	
	struct su_info *info = get_su_info(credential->uid);

	// Fail fast
	if (info->access.policy == DENY && DB_STR(info, SU_MANAGER)[0] == '\0') {
		LOGD("su: fast deny\n");
		write_int(client, DENY);
		close(client);
		return;
	}

	/* Fork a new process, the child process will need to setsid,
	 * open a pseudo-terminal if needed, and will eventually run exec
	 * The parent process will wait for the result and
	 * send the return code back to our client
	 */
	int child = xfork();
	if (child) {
		// Decrement reference count
		--info->ref;

		// Wait result
		LOGD("su: waiting child: [%d]\n", child);
		int status, code;

		if (waitpid(child, &status, 0) > 0)
			code = WEXITSTATUS(status);
		else
			code = -1;

		LOGD("su: return code: [%d]\n", code);
		write(client, &code, sizeof(code));
		close(client);
		return;
	}

	LOGD("su: fork handler\n");

	struct su_context ctx = {
		.info = info,
		.pid = credential->pid
	};

	// ack
	write_int(client, 0);

	// Become session leader
	xsetsid();

	// Migrate environment from client
	char path[32], buf[4096];
	snprintf(path, sizeof(path), "/proc/%d/cwd", ctx.pid);
	xreadlink(path, buf, sizeof(buf));
	chdir(buf);
	snprintf(path, sizeof(path), "/proc/%d/environ", ctx.pid);
	memset(buf, 0, sizeof(buf));
	int fd = open(path, O_RDONLY);
	read(fd, buf, sizeof(buf));
	close(fd);
	clearenv();
	for (size_t pos = 0; buf[pos];) {
		putenv(buf + pos);
		pos += strlen(buf + pos) + 1;
	}

	// Read su_request
	xxread(client, &ctx.req, 4 * sizeof(unsigned));
	ctx.req.shell = read_string(client);
	ctx.req.command = read_string(client);

	// Get pts_slave
	char *pts_slave = read_string(client);

	// The FDs for each of the streams
	int infd  = recv_fd(client);
	int outfd = recv_fd(client);
	int errfd = recv_fd(client);
	int ptsfd = -1;

	if (pts_slave[0]) {
		LOGD("su: pts_slave=[%s]\n", pts_slave);
		// Check pts_slave file is owned by daemon_from_uid
		struct stat st;
		xstat(pts_slave, &st);

		// If caller is not root, ensure the owner of pts_slave is the caller
		if(st.st_uid != info->uid && info->uid != 0) {
			LOGE("su: Wrong permission of pts_slave");
			info->access.policy = DENY;
			exit(1);
		}

		// Set our pts_slave to devpts, same restriction as adb shell
		lsetfilecon(pts_slave, "u:object_r:devpts:s0");

		// Opening the TTY has to occur after the
		// fork() and setsid() so that it becomes
		// our controlling TTY and not the daemon's
		ptsfd = xopen(pts_slave, O_RDWR);

		if (infd < 0)
			infd = ptsfd;
		if (outfd < 0)
			outfd = ptsfd;
		if (errfd < 0)
			errfd = ptsfd;
	}

	free(pts_slave);

	// Swap out stdin, stdout, stderr
	xdup2(infd, STDIN_FILENO);
	xdup2(outfd, STDOUT_FILENO);
	xdup2(errfd, STDERR_FILENO);

	close(ptsfd);
	close(client);

	// Handle namespaces
	if (ctx.req.mount_master)
		DB_SET(info, SU_MNT_NS) = NAMESPACE_MODE_GLOBAL;
	switch (DB_SET(info, SU_MNT_NS)) {
		case NAMESPACE_MODE_GLOBAL:
			LOGD("su: use global namespace\n");
			break;
		case NAMESPACE_MODE_REQUESTER:
			LOGD("su: use namespace of pid=[%d]\n", ctx.pid);
			if (switch_mnt_ns(ctx.pid)) {
				LOGD("su: setns failed, fallback to isolated\n");
				xunshare(CLONE_NEWNS);
			}
			break;
		case NAMESPACE_MODE_ISOLATE:
			LOGD("su: use new isolated namespace\n");
			xunshare(CLONE_NEWNS);
			break;
	}

	if (info->access.notify || info->access.log)
		app_log(&ctx);

	if (info->access.policy == ALLOW) {
		char* argv[] = { NULL, NULL, NULL, NULL };

		argv[0] = ctx.req.login ? "-" : ctx.req.shell;

		if (ctx.req.command[0]) {
			argv[1] = "-c";
			argv[2] = ctx.req.command;
		}

		// Setup shell
		umask(022);
		populate_environment(&ctx.req);
		set_identity(ctx.req.uid);

		execvp(ctx.req.shell, argv);
		fprintf(stderr, "Cannot execute %s: %s\n", ctx.req.shell, strerror(errno));
		PLOGE("exec");
		exit(EXIT_FAILURE);
	} else {
		LOGW("su: request rejected (%u->%u)", info->uid, ctx.req.uid);
		fprintf(stderr, "%s\n", strerror(EACCES));
		exit(EXIT_FAILURE);
	}
}

