/* daemon.c - Magisk Daemon
 *
 * Start the daemon and wait for requests
 * Connect the daemon and send requests through sockets
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"

static void request_handler(int client) {
	/* TODO: Put all function entrypoints here
	   it'll currently just log the input string */
	char *s = read_string(client);
	close(client);
	LOGI("%s\n", s);
	free(s);
}

/* Setup the address and return socket fd */
static int setup_socket(struct sockaddr_un *sun) {
	int fd = xsocket(AF_LOCAL, SOCK_STREAM, 0);
	if (fcntl(fd, F_SETFD, FD_CLOEXEC)) {
	    PLOGE("fcntl FD_CLOEXEC");
	}

	memset(sun, 0, sizeof(*sun));
	sun->sun_family = AF_LOCAL;
	memcpy(sun->sun_path, REQUESTOR_DAEMON_PATH, REQUESTOR_DAEMON_PATH_LEN);
	return fd;
}

void start_daemon() {
	// Launch the daemon, create new session, set proper context
	if (getuid() != AID_ROOT || getgid() != AID_ROOT) {
		PLOGE("daemon requires root. uid/gid not root");
	}
	switch (fork()) {
	case -1:
		PLOGE("fork");
	case 0:
		break;
	default:
		return;
	}
	xsetsid();
	xsetcon("u:r:su:s0");

	struct sockaddr_un sun;
	int fd = setup_socket(&sun);
	
	xbind(fd, (struct sockaddr*) &sun, sizeof(sun));
	xlisten(fd, 10);

	// Change process name
	strcpy(argv0, "magisk_daemon");

	// Loop forever to listen to requests
	while(1) {
		request_handler(xaccept(fd, NULL, NULL));
	}
}

/* Connect the daemon, and return a socketfd */
int connect_daemon() {
	struct sockaddr_un sun;
	int fd = setup_socket(&sun);
	if (connect(fd, (struct sockaddr*) &sun, sizeof(sun))) {
		/* If we cannot access the daemon, we start the daemon
		 * since there is no clear entry point when the daemon should be started
		 */
		start_daemon();
		do {
			// Wait for 10ms
			usleep(10000);
		} while (connect(fd, (struct sockaddr*) &sun, sizeof(sun)));
	}
	return fd;
}
