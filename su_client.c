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

// Constants for the atty bitfield
#define ATTY_IN     1
#define ATTY_OUT    2
#define ATTY_ERR    4

struct ucred su_credentials;

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

static void sigpipe_handler(int sig) {
	LOGD("su: Client killed unexpectedly\n");
}

void su_daemon_receiver(int client) {
	LOGD("su: get request from client: %d\n", client);

	// Fork a new process, the child process will need to setsid,
	// open a pseudo-terminal if needed, and will eventually run exec
	// The parent process will wait for the result and
	// send the return code back to our client
	int child = fork();
	if (child < 0) {
		write(client, &child, sizeof(child));
		close(client);
		PLOGE("fork");
		return;
	} else if (child != 0) {

		// Wait result
		LOGD("su: wait_result waiting for %d\n", child);
		int status, code;

		// Handle SIGPIPE, since we don't want to crash our daemon
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = sigpipe_handler;
		sigaction(SIGPIPE, &act, NULL);

		if (waitpid(child, &status, 0) > 0)
			code = WEXITSTATUS(status);
		else
			code = -1;

		// Pass the return code back to the client
		write_int(client, code);
		LOGD("su: return code to client: %d\n", code);
		close(client);

		return;
	}

	LOGD("su: child process started\n");

	// ack
	write_int(client, 1);

	// Become session leader
	xsetsid();

	// Get the credentials
	get_client_cred(client, &su_credentials);

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
		if(stbuf.st_uid != su_credentials.uid && su_credentials.uid != 0) {
			LOGE("su: Wrong permission of pts_slave");
			exit(1);
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
