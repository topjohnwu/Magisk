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
#define UNLOCK_UID()   pthread_mutex_unlock(&info->lock)

static struct list_head active_list, waiting_list;
static pthread_t su_collector = 0;
static pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

int pipefd[2];

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

// Maintain the lists periodically
static void *collector(void *args) {
	LOGD("su: collector started\n");
	struct su_info *node;
	while(1) {
		sleep(1);
		LOCK_LIST();
		list_for_each(node, &active_list, struct su_info, pos) {
			if (--node->clock == 0) {
				// Timeout, move to waiting list
				__ = list_pop(&node->pos);
				list_insert_end(&waiting_list, &node->pos);
			}
		}
		list_for_each(node, &waiting_list, struct su_info, pos) {
			if (node->ref == 0) {
				// Nothing is using the info, remove it
				__ = list_pop(&node->pos);
				pthread_mutex_destroy(&node->lock);
				free(node);
			}
		}
		UNLOCK_LIST();
	}
}

void su_daemon_receiver(int client) {
	LOGD("su: request from client: %d\n", client);

	struct su_info *info = NULL, *node;
	int new_request = 0;

	LOCK_LIST();

	if (!su_collector) {
		init_list_head(&active_list);
		init_list_head(&waiting_list);
		xpthread_create(&su_collector, NULL, collector, NULL);
	}

	// Get client credential
	struct ucred credential;
	get_client_cred(client, &credential);

	// Search for existing in the active list
	list_for_each(node, &active_list, struct su_info, pos) {
		if (node->uid == credential.uid) {
			info = node;
			break;
		}
	}

	// If no exist, create a new request
	if (info == NULL) {
		new_request = 1;
		info = malloc(sizeof(*info));
		info->uid = credential.uid;
		info->policy = QUERY;
		info->ref = 0;
		info->count = 0;
		pthread_mutex_init(&info->lock, NULL);
		list_insert_end(&active_list, &info->pos);
	}
	info->clock = TIMEOUT;  /* Reset timer */
	++info->ref;  /* Increment reference count */

	UNLOCK_LIST();

	LOGD("su: request from uid=[%d] (#%d)\n", info->uid, ++info->count);

	// Default values
	struct su_context ctx = {
		.info = info,
		.to = {
			.uid = UID_ROOT,
			.login = 0,
			.keepenv = 0,
			.shell = DEFAULT_SHELL,
			.command = NULL,
		},
		.pid = credential.pid,
		.umask = 022,
		.notify = new_request,
	};

	// Lock before the policy is determined
	LOCK_UID();

	// Not cached, do the checks
	if (info->policy == QUERY) {
		// Get data from database
		database_check(&ctx);

		// Check requester
		if (info->policy == QUERY) {
			if (ctx.st.st_gid != ctx.st.st_uid) {
				LOGE("Bad uid/gid %d/%d for Superuser Requestor", ctx.st.st_uid, ctx.st.st_gid);
				info->policy = DENY;
				ctx.notify = 0;
			} else if ((info->uid % 100000) == (ctx.st.st_uid % 100000)) {
				info->policy = ALLOW;
				info->root_access = ROOT_ACCESS_APPS_AND_ADB;
				ctx.notify = 0;
			}
		}

		// always allow if it's root
		if (info->uid == UID_ROOT) {
			info->policy = ALLOW;
			info->root_access = ROOT_ACCESS_APPS_AND_ADB;
			ctx.notify = 0;
		}

		// If still not determined, open a pipe and wait for results
		if (info->policy == QUERY)
			xpipe2(ctx.pipefd, O_CLOEXEC);
	}

	// Fork a new process, the child process will need to setsid,
	// open a pseudo-terminal if needed, and will eventually run exec
	// The parent process will wait for the result and
	// send the return code back to our client
	int child = fork();
	if (child < 0)
		PLOGE("fork");

	if (child) {
		// Wait for results
		if (info->policy == QUERY) {
			xxread(ctx.pipefd[0], &info->policy, sizeof(info->policy));
			close(ctx.pipefd[0]);
			close(ctx.pipefd[1]);
		}

		// The policy is determined, unlock
		UNLOCK_UID();

		// Wait result
		LOGD("su: waiting child: [%d]\n", child);
		int status, code;

		if (waitpid(child, &status, 0) > 0)
			code = WEXITSTATUS(status);
		else
			code = -1;

		/* Passing the return code back to the client:
		 * The client might be closed unexpectedly (e.g. swipe a root app out of recents)
		 * In that case, writing to the client (which doesn't exist) will result in SIGPIPE
		 * Here we simply just ignore the situation.
		 */
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = SIG_IGN;
		sigaction(SIGPIPE, &act, NULL);

		LOGD("su: return code: [%d]\n", code);
		write(client, &code, sizeof(code));
		close(client);

		// Restore default handler for SIGPIPE
		act.sa_handler = SIG_DFL;
		sigaction(SIGPIPE, &act, NULL);

		// Decrement reference count
		LOCK_LIST();
		--info->ref;
		UNLOCK_LIST();

		return;
	}

	LOGD("su: child process started\n");
	UNLOCK_UID();

	// ack
	write_int(client, 1);

	// Become session leader
	xsetsid();

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

	// Get cwd
	ctx.cwd = read_string(client);
	LOGD("su: cwd=[%s]\n", ctx.cwd);

	// Get pts_slave
	char *pts_slave = read_string(client);
	LOGD("su: pts_slave=[%s]\n", pts_slave);

	// The the FDs for each of the streams
	int infd  = recv_fd(client);
	int outfd = recv_fd(client);
	int errfd = recv_fd(client);
	int ptsfd = -1;

	// We no longer need the access to socket in the child, close it
	close(client);

	if (pts_slave[0]) {
		//Check pts_slave file is owned by daemon_from_uid
		struct stat stbuf;
		xstat(pts_slave, &stbuf);

		//If caller is not root, ensure the owner of pts_slave is the caller
		if(stbuf.st_uid != info->uid && info->uid != 0) {
			LOGE("su: Wrong permission of pts_slave");
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

	// Give main the reference
	su_ctx = &ctx;
	su_daemon_main(argc, argv);
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

	// CWD
	getcwd(buffer, sizeof(buffer));
	write_string(socketfd, buffer);

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
	read_int(socketfd);

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
