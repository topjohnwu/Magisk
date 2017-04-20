/* daemon.c - Magisk Daemon
 *
 * Start the daemon and wait for requests
 * Connect the daemon and send requests through sockets
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"
#include "magiskpolicy.h"

pthread_t sepol_patch;

static void request_handler(int client) {
	client_request req = read_int(client);
	char *s;
	int pid, status, code;
	switch (req) {
	case LAUNCH_MAGISKHIDE:
		launch_magiskhide(client);
		break;
	case STOP_MAGISKHIDE:
		stop_magiskhide(client);
		break;
	case ADD_HIDELIST:
		add_hide_list(client);
		break;
	case RM_HIDELIST:
		rm_hide_list(client);
		break;
	case SUPERUSER:
		su_daemon_receiver(client);
		break;
	case CHECK_VERSION:
		write_string(client, MAGISK_VER_STR);
		close(client);
		break;
	case CHECK_VERSION_CODE:
		write_int(client, MAGISK_VER_CODE);
		close(client);
		break;
	case POST_FS:
		post_fs(client);
		break;
	case POST_FS_DATA:
		post_fs_data(client);
		break;
	case LATE_START:
		late_start(client);
		break;
	case TEST:
		s = read_string(client);
		LOGI("%s\n", s);
		free(s);
		write_int(client, 0);
		close(client);
		break;
	}
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

static void do_nothing() {}

static void *large_sepol_patch(void *args) {
	LOGD("sepol: Starting large patch thread\n");
	// Patch su to everything
	sepol_allow("su", ALL, ALL, ALL);
	dump_policydb("/sys/fs/selinux/load");
	LOGD("sepol: Large patch done\n");
	destroy_policydb();
	return NULL;
}

void start_daemon() {
	// Launch the daemon, create new session, set proper context
	if (getuid() != UID_ROOT || getgid() != UID_ROOT) {
		fprintf(stderr, "Starting daemon requires root: %s\n", strerror(errno));
		PLOGE("start daemon");
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

	// Patch selinux with medium patch, blocking
	load_policydb("/sys/fs/selinux/policy");
	sepol_med_rules();
	dump_policydb("/sys/fs/selinux/load");

	// Continue the larger patch in another thread, will join later
	pthread_create(&sepol_patch, NULL, large_sepol_patch, NULL);

	struct sockaddr_un sun;
	int fd = setup_socket(&sun);
	
	xbind(fd, (struct sockaddr*) &sun, sizeof(sun));
	xlisten(fd, 10);

	// Change process name
	strcpy(argv0, "magisk_daemon");
	// The root daemon should not do anything if an error occurs
	// It should stay intact under any circumstances
	err_handler = do_nothing;

	// Start log monitor
	monitor_logs();

	LOGI("Magisk v" xstr(MAGISK_VERSION) " daemon started\n");

	// Unlock all blocks for rw
	unlock_blocks();

	// Setup links under /sbin
	xmount(NULL, "/", NULL, MS_REMOUNT, NULL);
	create_links(NULL, "/sbin");
	chmod("/sbin", 0755);
	xmount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL);

	// Loop forever to listen to requests
	while(1) {
		request_handler(xaccept(fd, NULL, NULL));
	}
}

/* Connect the daemon, and return a socketfd */
int connect_daemon() {
	struct sockaddr_un sun;
	int fd = setup_socket(&sun);
	// LOGD("client: trying to connect socket\n");
	if (connect(fd, (struct sockaddr*) &sun, sizeof(sun))) {
		/* If we cannot access the daemon, we start the daemon
		 * since there is no clear entry point when the daemon should be started
		 */
		LOGD("client: connect fail, try launching new daemon process\n");
		start_daemon();
		do {
			// Wait for 10ms
			usleep(10);
		} while (connect(fd, (struct sockaddr*) &sun, sizeof(sun)));
	}
	return fd;
}
