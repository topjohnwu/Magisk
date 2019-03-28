#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <magisk.h>
#include <daemon.h>
#include <utils.h>
#include <selinux.h>

#include "su.h"
#include "pts.h"

#define LOCK_CACHE()   pthread_mutex_lock(&cache_lock)
#define UNLOCK_CACHE() pthread_mutex_unlock(&cache_lock)

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
static su_info *cache;

su_info::su_info(unsigned uid) :
		uid(uid), access(DEFAULT_SU_ACCESS), _lock(PTHREAD_MUTEX_INITIALIZER),
		count(0), ref(0), timestamp(0), mgr_st({}) {}

su_info::~su_info() {
	pthread_mutex_destroy(&_lock);
}

void su_info::lock() {
	pthread_mutex_lock(&_lock);
}

void su_info::unlock() {
	pthread_mutex_unlock(&_lock);
}

bool su_info::isFresh() {
	return time(nullptr) - timestamp < 3;  /* 3 seconds */
}

void su_info::newRef() {
	timestamp = time(nullptr);
	++ref;
}

void su_info::deRef() {
	LOCK_CACHE();
	--ref;
	if (ref == 0 && !isFresh()) {
		if (cache == this)
			cache = nullptr;
		delete this;
	}
	UNLOCK_CACHE();
}

static void database_check(su_info *info) {
	int uid = info->uid;
	get_db_settings(info->cfg);
	get_db_strings(info->str);

	// Check multiuser settings
	switch (info->cfg[SU_MULTIUSER_MODE]) {
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
		get_uid_policy(uid, info->access);

	// We need to check our manager
	if (info->access.log || info->access.notify)
		validate_manager(info->str[SU_MANAGER], uid / 100000, &info->mgr_st);
}

static struct su_info *get_su_info(unsigned uid) {
	su_info *info = nullptr;

	// Get from cache or new instance
	LOCK_CACHE();
	if (cache && cache->uid == uid && cache->isFresh()) {
		info = cache;
	} else {
		if (cache && cache->ref == 0)
			delete cache;
		cache = info = new su_info(uid);
	}
	info->newRef();
	UNLOCK_CACHE();

	LOGD("su: request from uid=[%d] (#%d)\n", info->uid, ++info->count);

	// Lock before the policy is determined
	info->lock();

	if (info->access.policy == QUERY) {
		//  Not cached, get data from database
		database_check(info);

		// Check su access settings
		switch (info->cfg[ROOT_ACCESS]) {
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
		if (info->access.policy == QUERY && info->str[SU_MANAGER][0] == '\0')
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
			info->access.policy = ret < 0 ? DENY : static_cast<policy_t>(ret);
			close(fd);
		}
		close(sockfd);
	}

	// Unlock
	info->unlock();

	return info;
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
	LOGD("su: request from pid=[%d], client=[%d]\n", credential->pid, client);
	
	su_info *info = get_su_info(credential->uid);

	// Fail fast
	if (info->access.policy == DENY && info->str[SU_MANAGER][0] == '\0') {
		LOGD("su: fast deny\n");
		info->deRef();
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
		info->deRef();

		// Wait result
		LOGD("su: waiting child pid=[%d]\n", child);
		int status, code;

		if (waitpid(child, &status, 0) > 0)
			code = WEXITSTATUS(status);
		else
			code = -1;

		LOGD("su: return code=[%d]\n", code);
		write(client, &code, sizeof(code));
		close(client);
		return;
	}

	LOGD("su: fork handler\n");

	// Abort upon any error occurred
	log_cb.ex = exit;

	struct su_context ctx = {
		.info = info,
		.pid = credential->pid
	};

	// ack
	write_int(client, 0);

	// Become session leader
	xsetsid();

	// Read su_request
	xxread(client, &ctx.req, sizeof(su_req_base));
	ctx.req.shell = read_string(client);
	ctx.req.command = read_string(client);

	// Get pts_slave
	char *pts_slave = read_string(client);

	// The FDs for each of the streams
	int infd  = recv_fd(client);
	int outfd = recv_fd(client);
	int errfd = recv_fd(client);

	if (pts_slave[0]) {
		LOGD("su: pts_slave=[%s]\n", pts_slave);
		// Check pts_slave file is owned by daemon_from_uid
		struct stat st;
		xstat(pts_slave, &st);

		// If caller is not root, ensure the owner of pts_slave is the caller
		if(st.st_uid != info->uid && info->uid != 0)
			LOGE("su: Wrong permission of pts_slave");

		// Opening the TTY has to occur after the
		// fork() and setsid() so that it becomes
		// our controlling TTY and not the daemon's
		int ptsfd = xopen(pts_slave, O_RDWR);

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

	// Unleash all streams from SELinux hell
	setfilecon("/proc/self/fd/0", "u:object_r:" SEPOL_FILE_DOMAIN ":s0");
	setfilecon("/proc/self/fd/1", "u:object_r:" SEPOL_FILE_DOMAIN ":s0");
	setfilecon("/proc/self/fd/2", "u:object_r:" SEPOL_FILE_DOMAIN ":s0");

	close(infd);
	close(outfd);
	close(errfd);
	close(client);

	// Handle namespaces
	if (ctx.req.mount_master)
		info->cfg[SU_MNT_NS] = NAMESPACE_MODE_GLOBAL;
	switch (info->cfg[SU_MNT_NS]) {
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

	if (info->access.log)
		app_log(&ctx);
	else if (info->access.notify)
		app_notify(&ctx);

	if (info->access.policy == ALLOW) {
		const char *argv[] = { nullptr, nullptr, nullptr, nullptr };

		argv[0] = ctx.req.login ? "-" : ctx.req.shell;

		if (ctx.req.command[0]) {
			argv[1] = "-c";
			argv[2] = ctx.req.command;
		}

		// Setup environment
		umask(022);
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
		if (!ctx.req.keepenv) {
			struct passwd *pw;
			pw = getpwuid(ctx.req.uid);
			if (pw) {
				setenv("HOME", pw->pw_dir, 1);
				if (ctx.req.login || ctx.req.uid) {
					setenv("USER", pw->pw_name, 1);
					setenv("LOGNAME", pw->pw_name, 1);
				}
				setenv("SHELL", ctx.req.shell, 1);
			}
		}

		set_identity(ctx.req.uid);
		execvp(ctx.req.shell, (char **) argv);
		fprintf(stderr, "Cannot execute %s: %s\n", ctx.req.shell, strerror(errno));
		PLOGE("exec");
		exit(EXIT_FAILURE);
	} else {
		LOGW("su: request rejected (%u->%u)", info->uid, ctx.req.uid);
		fprintf(stderr, "%s\n", strerror(EACCES));
		exit(EXIT_FAILURE);
	}
}
