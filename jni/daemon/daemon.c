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
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"
#include "magiskpolicy.h"

pthread_t sepol_patch;

static void *request_handler(void *args) {
	// Setup the default error handler for threads
	err_handler = exit_thread;

	int client = *((int *) args);
	free(args);
	client_request req = read_int(client);

	struct ucred credentials;
	get_client_cred(client, &credentials);

	switch (req) {
	case LAUNCH_MAGISKHIDE:
	case STOP_MAGISKHIDE:
	case ADD_HIDELIST:
	case RM_HIDELIST:
	case POST_FS:
	case POST_FS_DATA:
	case LATE_START:
		if (credentials.uid != 0) {
			write_int(client, ROOT_REQUIRED);
			close(client);
			return NULL;
		}
	default:
		break;
	}

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
	default:
		break;
	}
	return NULL;
}

/* Setup the address and return socket fd */
static int setup_socket(struct sockaddr_un *sun) {
	int fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	memset(sun, 0, sizeof(*sun));
	sun->sun_family = AF_LOCAL;
	memcpy(sun->sun_path, REQUESTOR_DAEMON_PATH, sizeof(REQUESTOR_DAEMON_PATH) - 1);
	return fd;
}

static void *large_sepol_patch(void *args) {
	LOGD("sepol: Starting large patch thread\n");
	// Patch su to everything
	sepol_allow("su", ALL, ALL, ALL);
	dump_policydb(SELINUX_LOAD);
	LOGD("sepol: Large patch done\n");
	destroy_policydb();
	return NULL;
}

void start_daemon(int client) {
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

	// First close the client, it's useless for us
	close(client);
	xsetsid();
	setcon("u:r:su:s0");
	umask(022);
	int fd = xopen("/dev/null", O_RDWR | O_CLOEXEC);
	xdup2(fd, STDIN_FILENO);
	xdup2(fd, STDOUT_FILENO);
	xdup2(fd, STDERR_FILENO);
	close(fd);

	// Patch selinux with medium patch before we do anything
	load_policydb(SELINUX_POLICY);
	sepol_med_rules();
	dump_policydb(SELINUX_LOAD);

	// Continue the larger patch in another thread, we will join later
	pthread_create(&sepol_patch, NULL, large_sepol_patch, NULL);

	struct sockaddr_un sun;
	fd = setup_socket(&sun);

	xbind(fd, (struct sockaddr*) &sun, sizeof(sun));
	xlisten(fd, 10);

	// Change process name
	strcpy(argv0, "magisk_daemon");
	// The root daemon should not do anything if an error occurs
	// It should stay intact under any circumstances
	err_handler = do_nothing;

	LOGI("Magisk v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") daemon started\n");

	// Unlock all blocks for rw
	unlock_blocks();

	// Setup links under /sbin
	xmount(NULL, "/", NULL, MS_REMOUNT, NULL);
	create_links(NULL, "/sbin");
	xchmod("/sbin", 0755);
	xmkdir("/magisk", 0755);
	xchmod("/magisk", 0755);
	xmount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL);

	// Loop forever to listen for requests
	while(1) {
		int *client = xmalloc(sizeof(int));
		*client = xaccept4(fd, NULL, NULL, SOCK_CLOEXEC);
		pthread_t thread;
		xpthread_create(&thread, NULL, request_handler, client);
		// Detach the thread, we will never join it
		pthread_detach(thread);
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
		LOGD("client: connect fail, try launching new daemon process\n");
		start_daemon(fd);
		do {
			// Wait for 10ms
			usleep(10);
		} while (connect(fd, (struct sockaddr*) &sun, sizeof(sun)));
	}
	return fd;
}
