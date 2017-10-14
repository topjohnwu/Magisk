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
#include "resetprop.h"

pthread_t sepol_patch;
int is_restart = 0;

static void *request_handler(void *args) {
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
	case LS_HIDELIST:
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
	case LS_HIDELIST:
		ls_hide_list(client);
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

static void *start_magisk_hide(void *args) {
	launch_magiskhide(-1);
	return NULL;
}

void auto_start_magiskhide() {
	char *hide_prop = getprop2(MAGISKHIDE_PROP, 1);
	if (hide_prop == NULL || strcmp(hide_prop, "0") != 0) {
		pthread_t thread;
		xpthread_create(&thread, NULL, start_magisk_hide, NULL);
		pthread_detach(thread);
	}
	free(hide_prop);
}

void start_daemon() {
	setcon("u:r:su:s0");
	umask(0);
	int fd = xopen("/dev/null", O_RDWR | O_CLOEXEC);
	xdup2(fd, STDIN_FILENO);
	xdup2(fd, STDOUT_FILENO);
	xdup2(fd, STDERR_FILENO);
	close(fd);

	// Patch selinux with medium patch before we do anything
	load_policydb(SELINUX_POLICY);
	sepol_med_rules();
	dump_policydb(SELINUX_LOAD);

	struct sockaddr_un sun;
	fd = setup_socket(&sun);

	if (xbind(fd, (struct sockaddr*) &sun, sizeof(sun)) == -1)
		exit(1);
	xlisten(fd, 10);

	if ((is_restart = access(UNBLOCKFILE, F_OK) == 0)) {
		// Restart stuffs if the daemon is restarted
		exec_command_sync("logcat", "-b", "all", "-c", NULL);
		auto_start_magiskhide();
		start_debug_log();
	}

	// Start the log monitor
	monitor_logs();

	// Continue the larger patch in another thread, we will join later
	xpthread_create(&sepol_patch, NULL, large_sepol_patch, NULL);

	LOGI("Magisk v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") daemon started\n");

	// Change process name
	strcpy(argv0, "magisk_daemon");

	// Unlock all blocks for rw
	unlock_blocks();

	// Notifiy init the daemon is started
	close(xopen(UNBLOCKFILE, O_RDONLY | O_CREAT));

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
	if (xconnect(fd, (struct sockaddr*) &sun, sizeof(sun))) {
		// If we cannot access the daemon, we start a daemon in the child process if possible

		if (getuid() != UID_ROOT || getgid() != UID_ROOT) {
			fprintf(stderr, "No daemon is currently running!\n");
			exit(1);
		}

		if (xfork() == 0) {
			LOGD("client: connect fail, try launching new daemon process\n");
			close(fd);
			xsetsid();
			start_daemon();
		}

		do {
			// Wait for 10ms
			usleep(10);
		} while (connect(fd, (struct sockaddr*) &sun, sizeof(sun)));
	}
	return fd;
}
