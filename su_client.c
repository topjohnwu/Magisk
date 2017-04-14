/* su_client.c - The entrypoint for su, connect to daemon and send correct info
 */


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

int from_uid, from_pid;

struct su_daemon_info {
	int child;
	int client;
};

static void *wait_result(void *args) {
	struct su_daemon_info *info = args;
	LOGD("su: wait_result waiting for %d\n", info->child);
	err_handler = exit_thread;
	int status, code;

	if (waitpid(info->child, &status, 0) > 0)
		code = WEXITSTATUS(status);
	else
		code = -1;

	// Pass the return code back to the client
	write_int(info->client, code);
	LOGD("su: return code to client: %d\n", code);
	close(info->client);
	free(info);
	return NULL;
}

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

void su_daemon_receiver(int client) {
	LOGD("su: get request\n");
	// Fork a new process, the child process will need to setsid,
	// open a pseudo-terminal, and will eventually run exec
	// The parent process (the root daemon) will open a new thread to
	// send the return code back to our client
	int child = fork();
	if (child < 0) {
		PLOGE("fork");
		write(client, &child, sizeof(child));
		close(client);
		return;
	} else if (child != 0) {
		pthread_t wait_thread;

		struct su_daemon_info *info = xmalloc(sizeof(*info));
		info->client = client;
		info->child = child;

		// In parent, open a new thread to wait for the child to exit,
		// and send the exit code across the wire.
		xpthread_create(&wait_thread, NULL, wait_result, info);
		return;
	}

	LOGD("su: child process started\n");

	// ack
	write_int(client, 1);

	// Here we're in the child
	// Set the error handler back to normal
	err_handler = exit_proc;
	// Become session leader
	xsetsid();

	// Check the credentials
	struct ucred credentials;
	socklen_t ucred_length = sizeof(struct ucred);
	if(getsockopt(client, SOL_SOCKET, SO_PEERCRED, &credentials, &ucred_length))
		PLOGE("getsockopt");

	from_uid = credentials.uid;
	from_pid = credentials.pid;

	// Let's read some info from the socket
	int argc = read_int(client);
	if (argc < 0 || argc > 512) {
		LOGE("unable to allocate args: %d", argc);
		exit(1);
	}
	LOGD("su: argc=[%d]\n", argc);

	char **argv = (char**) xmalloc(sizeof(char*) * (argc + 1));
	argv[argc] = NULL;
	for (int i = 0; i < argc; i++) {
		argv[i] = read_string(client);
		LOGD("su: argv[%d]=[%s]\n", i, argv[i]);
	}

	// Change directory to cwd
	char *cwd = read_string(client);
	LOGD("su: cwd=[%s]\n", cwd);
	chdir(cwd);
	free(cwd);

	// Get pts_slave
	char *pts_slave = read_string(client);
	LOGD("su: pts_slave=[%s]\n", pts_slave);

	// We no longer need the access to socket in the child, close it
	close(client);

	//Check pts_slave file is owned by daemon_from_uid
	int ptsfd;
	struct stat stbuf;
	int res = xstat(pts_slave, &stbuf);

	//If caller is not root, ensure the owner of pts_slave is the caller
	if(stbuf.st_uid != credentials.uid && credentials.uid != 0) {
		LOGE("Wrong permission of pts_slave");
		exit(1);
	}

	// Opening the TTY has to occur after the
	// fork() and setsid() so that it becomes
	// our controlling TTY and not the daemon's
	ptsfd = xopen(pts_slave, O_RDWR);

	free(pts_slave);

	// Swap out stdin, stdout, stderr
	xdup2(ptsfd, STDIN_FILENO);
	xdup2(ptsfd, STDOUT_FILENO);
	xdup2(ptsfd, STDERR_FILENO);

	close(ptsfd);

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

	// We need a PTY. Get one.
	ptmx = pts_open(buffer, sizeof(buffer));
	watch_sigwinch_async(STDOUT_FILENO, ptmx);

	// Send the slave path to the daemon
	write_string(socketfd, buffer);

	// Wait for acknowledgement from daemon
	read_int(socketfd);

	setup_sighandlers(sighandler);
	pump_stdin_async(ptmx);
	pump_stdout_blocking(ptmx);

	// Get the exit code
	int code = read_int(socketfd);
	close(socketfd);

	return code;
}
