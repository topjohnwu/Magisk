/* su_daemon.c - The entrypoint for su, connect to daemon and send correct info
 */

#define _GNU_SOURCE
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "magisk.h"
#include "daemon.h"
#include "utils.h"
#include "su.h"
#include "pts.h"
#include "list.h"

// Constants for the atty bitfield
#define ATTY_IN     1
#define ATTY_OUT    2
#define ATTY_ERR    4

#define TIMEOUT     3

#define LOCK_LIST()    pthread_mutex_lock(&list_lock)
#define LOCK_UID()     pthread_mutex_lock(&info->lock)
#define UNLOCK_LIST()  pthread_mutex_unlock(&list_lock)
#define UNLOCK_UID()   pthread_mutex_unlock(&ctx.info->lock)

static struct list_head info_cache = { .prev = &info_cache, .next = &info_cache };
static pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

static void sighandler(int sig) {
	restore_stdin();

	// Assume we'll only be called before death
	// See note before sigaction() in set_stdin_raw()
	//
	// Now, close all standard I/O to cause the pumps
	// to exit so we can continue and retrieve the exit
	// code
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// Put back all the default handlers
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = SIG_DFL;
	for (int i = 0; quit_signals[i]; ++i) {
		sigaction(quit_signals[i], &act, NULL);
	}
}

static void *info_collector(void *node) {
	struct su_info *info = node;
	while (1) {
		sleep(1);
		if (info->clock && --info->clock == 0) {
			LOCK_LIST();
			list_pop(&info->pos);
			UNLOCK_LIST();
		}
		if (!info->clock && !info->ref) {
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
		switch (info->dbs.v[SU_MULTIUSER_MODE]) {
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
		sqlite3_close(db);
	}

	// We need to check our manager
	if (info->access.log || info->access.notify)
		validate_manager(info->str.s[SU_MANAGER], uid / 100000, &info->manager_stat);
}

static struct su_info *get_su_info(unsigned uid) {
	struct su_info *info = NULL, *node;

	LOCK_LIST();

	// Search for existing info in cache
	list_for_each(node, &info_cache, struct su_info, pos) {
		if (node->uid == uid) {
			info = node;
			break;
		}
	}

	int cache_miss = info == NULL;

	if (cache_miss) {
		// If cache miss, create a new one and push to cache
		info = malloc(sizeof(*info));
		info->uid = uid;
		info->dbs = DEFAULT_DB_SETTINGS;
		info->access = DEFAULT_SU_ACCESS;
		INIT_DB_STRINGS(&info->str);
		info->ref = 0;
		info->count = 0;
		pthread_mutex_init(&info->lock, NULL);
		list_insert_end(&info_cache, &info->pos);
	}

	// Update the cache status
	info->clock = TIMEOUT;
	++info->ref;

	// Start a thread to maintain the info cache
	if (cache_miss) {
		pthread_t thread;
		xpthread_create(&thread, NULL, info_collector, info);
		pthread_detach(thread);
	}

	UNLOCK_LIST();

	LOGD("su: request from uid=[%d] (#%d)\n", info->uid, ++info->count);

	// Lock before the policy is determined
	LOCK_UID();

	if (info->access.policy == QUERY) {
		//  Not cached, get data from database
		database_check(info);

		// Check su access settings
		switch (info->dbs.v[ROOT_ACCESS]) {
			case ROOT_ACCESS_DISABLED:
				LOGE("Root access is disabled!\n");
				info->access = NO_SU_ACCESS;
				break;
			case ROOT_ACCESS_ADB_ONLY:
				if (info->uid != UID_SHELL) {
					LOGE("Root access limited to ADB only!\n");
					info->access = NO_SU_ACCESS;
				}
				break;
			case ROOT_ACCESS_APPS_ONLY:
				if (info->uid == UID_SHELL) {
					LOGE("Root access is disabled for ADB!\n");
					info->access = NO_SU_ACCESS;
				}
				break;
			case ROOT_ACCESS_APPS_AND_ADB:
			default:
				break;
		}

		// If it's the manager, allow it silently
		if ((info->uid % 100000) == (info->manager_stat.st_uid % 100000))
			info->access = SILENT_SU_ACCESS;

		// Allow if it's root
		if (info->uid == UID_ROOT)
			info->access = SILENT_SU_ACCESS;

		// If still not determined, check if manager exists
		if (info->access.policy == QUERY && info->str.s[SU_MANAGER][0] == '\0')
			info->access = NO_SU_ACCESS;
	}
	return info;
}

static void su_executor(int client) {
	LOGD("su: executor started\n");

	// ack
	write_int(client, 0);

	// Become session leader
	xsetsid();

	// Migrate environment from client
	char path[32], buf[4096];
	snprintf(path, sizeof(path), "/proc/%d/cwd", su_ctx->pid);
	xreadlink(path, su_ctx->cwd, sizeof(su_ctx->cwd));
	snprintf(path, sizeof(path), "/proc/%d/environ", su_ctx->pid);
	memset(buf, 0, sizeof(buf));
	int fd = open(path, O_RDONLY);
	read(fd, buf, sizeof(buf));
	clearenv();
	for (size_t pos = 0; buf[pos];) {
		putenv(buf + pos);
		pos += strlen(buf + pos) + 1;
	}

	// Let's read some info from the socket
	int argc = read_int(client);
	if (argc < 0 || argc > 512) {
		LOGE("unable to allocate args: %d", argc);
		exit2(1);
	}
	LOGD("su: argc=[%d]\n", argc);

	char **argv = (char**) xmalloc(sizeof(char*) * (argc + 1));
	argv[argc] = NULL;
	for (int i = 0; i < argc; i++) {
		argv[i] = read_string(client);
		LOGD("su: argv[%d]=[%s]\n", i, argv[i]);
		// Replace -cn with -z, -mm with -M for supporting getopt_long
		if (strcmp(argv[i], "-cn") == 0)
			strcpy(argv[i], "-z");
		else if (strcmp(argv[i], "-mm") == 0)
			strcpy(argv[i], "-M");
	}

	// Get pts_slave
	char *pts_slave = read_string(client);

	// The FDs for each of the streams
	int infd  = recv_fd(client);
	int outfd = recv_fd(client);
	int errfd = recv_fd(client);
	int ptsfd = -1;

	// We no longer need the access to socket in the child, close it
	close(client);

	if (pts_slave[0]) {
		LOGD("su: pts_slave=[%s]\n", pts_slave);
		// Check pts_slave file is owned by daemon_from_uid
		struct stat st;
		xstat(pts_slave, &st);

		// If caller is not root, ensure the owner of pts_slave is the caller
		if(st.st_uid != su_ctx->info->uid && su_ctx->info->uid != 0) {
			LOGE("su: Wrong permission of pts_slave");
			su_ctx->info->access.policy = DENY;
			exit2(1);
		}

		// Opening the TTY has to occur after the
		// fork() and setsid() so that it becomes
		// our controlling TTY and not the daemon's
		ptsfd = xopen(pts_slave, O_RDWR);

		if (infd < 0)  {
			LOGD("su: stdin using PTY");
			infd  = ptsfd;
		}
		if (outfd < 0) {
			LOGD("su: stdout using PTY");
			outfd = ptsfd;
		}
		if (errfd < 0) {
			LOGD("su: stderr using PTY");
			errfd = ptsfd;
		}
	}

	free(pts_slave);

	// Swap out stdin, stdout, stderr
	xdup2(infd, STDIN_FILENO);
	xdup2(outfd, STDOUT_FILENO);
	xdup2(errfd, STDERR_FILENO);

	close(ptsfd);

	// Run the actual main
	su_daemon_main(argc, argv);
}

void su_daemon_receiver(int client, struct ucred *credential) {
	LOGD("su: request from client: %d\n", client);

	// Default values
	struct su_context ctx = {
			.info = get_su_info(credential->uid),
			.to = {
					.uid = UID_ROOT,
					.login = 0,
					.keepenv = 0,
					.shell = DEFAULT_SHELL,
					.command = NULL,
			},
			.pid = credential->pid,
			.pipefd = { -1, -1 }
	};

	// Fail fast
	if (ctx.info->access.policy == DENY && !ctx.info->access.log && !ctx.info->access.notify) {
		UNLOCK_UID();
		write_int(client, DENY);
		return;
	}

	// If still not determined, open a pipe and wait for results
	if (ctx.info->access.policy == QUERY)
		xpipe2(ctx.pipefd, O_CLOEXEC);

	/* Fork a new process, the child process will need to setsid,
	 * open a pseudo-terminal if needed, and will eventually run exec
	 * The parent process will wait for the result and
	 * send the return code back to our client
	 */
	int child = xfork();
	if (child == 0) {
		su_ctx = &ctx;
		su_executor(client);
	}

	// Wait for results
	if (ctx.pipefd[0] >= 0) {
		xxread(ctx.pipefd[0], &ctx.info->access.policy, sizeof(policy_t));
		close(ctx.pipefd[0]);
		close(ctx.pipefd[1]);
	}

	// The policy is determined, unlock
	UNLOCK_UID();

	// Info is now useless to us, decrement reference count
	--ctx.info->ref;

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

/*
 * Connect daemon, send argc, argv, cwd, pts slave
 */
int su_client_main(int argc, char *argv[]) {
	char buffer[PATH_MAX];
	int ptmx, socketfd;

	// Connect to client
	socketfd = connect_daemon();

	// Tell the daemon we are su
	write_int(socketfd, SUPERUSER);

	// Number of command line arguments
	write_int(socketfd, argc);

	// Command line arguments
	for (int i = 0; i < argc; i++) {
		write_string(socketfd, argv[i]);
	}

	// Determine which one of our streams are attached to a TTY
	int atty = 0;
	if (isatty(STDIN_FILENO))  atty |= ATTY_IN;
	if (isatty(STDOUT_FILENO)) atty |= ATTY_OUT;
	if (isatty(STDERR_FILENO)) atty |= ATTY_ERR;

	if (atty) {
		// We need a PTY. Get one.
		ptmx = pts_open(buffer, sizeof(buffer));
	} else {
		buffer[0] = '\0';
	}

	// Send the pts_slave path to the daemon
	write_string(socketfd, buffer);

	// Send stdin
	if (atty & ATTY_IN) {
		// Using PTY
		send_fd(socketfd, -1);
	} else {
		send_fd(socketfd, STDIN_FILENO);
	}

	// Send stdout
	if (atty & ATTY_OUT) {
		// Forward SIGWINCH
		watch_sigwinch_async(STDOUT_FILENO, ptmx);

		// Using PTY
		send_fd(socketfd, -1);
	} else {
		send_fd(socketfd, STDOUT_FILENO);
	}

	// Send stderr
	if (atty & ATTY_ERR) {
		// Using PTY
		send_fd(socketfd, -1);
	} else {
		send_fd(socketfd, STDERR_FILENO);
	}

	// Wait for acknowledgement from daemon
	if (read_int(socketfd)) {
		// Fast fail
		fprintf(stderr, "%s\n", strerror(EACCES));
		return DENY;
	}

	if (atty & ATTY_IN) {
		setup_sighandlers(sighandler);
		pump_stdin_async(ptmx);
	}
	if (atty & ATTY_OUT) {
		pump_stdout_blocking(ptmx);
	}

	// Get the exit code
	int code = read_int(socketfd);
	close(socketfd);

	return code;
}
