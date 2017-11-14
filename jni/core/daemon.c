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
#include <signal.h>
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

	// Block user signals
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGUSR1);
	sigaddset(&block_set, SIGUSR2);
	pthread_sigmask(SIG_SETMASK, &block_set, NULL);

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

/*
 * Receive a file descriptor from a Unix socket.
 * Contributed by @mkasick
 *
 * Returns the file descriptor on success, or -1 if a file
 * descriptor was not actually included in the message
 *
 * On error the function terminates by calling exit(-1)
 */
int recv_fd(int sockfd) {
    // Need to receive data from the message, otherwise don't care about it.
    char iovbuf;

    struct iovec iov = {
        .iov_base = &iovbuf,
        .iov_len  = 1,
    };

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    struct msghdr msg = {
        .msg_iov        = &iov,
        .msg_iovlen     = 1,
        .msg_control    = cmsgbuf,
        .msg_controllen = sizeof(cmsgbuf),
    };

    xrecvmsg(sockfd, &msg, MSG_WAITALL);

    // Was a control message actually sent?
    switch (msg.msg_controllen) {
    case 0:
        // No, so the file descriptor was closed and won't be used.
        return -1;
    case sizeof(cmsgbuf):
        // Yes, grab the file descriptor from it.
        break;
    default:
        goto error;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    if (cmsg             == NULL                  ||
        cmsg->cmsg_len   != CMSG_LEN(sizeof(int)) ||
        cmsg->cmsg_level != SOL_SOCKET            ||
        cmsg->cmsg_type  != SCM_RIGHTS) {
error:
        LOGE("unable to read fd");
        exit(-1);
    }

    return *(int *)CMSG_DATA(cmsg);
}

/*
 * Send a file descriptor through a Unix socket.
 * Contributed by @mkasick
 *
 * On error the function terminates by calling exit(-1)
 *
 * fd may be -1, in which case the dummy data is sent,
 * but no control message with the FD is sent.
 */
void send_fd(int sockfd, int fd) {
    // Need to send some data in the message, this will do.
    struct iovec iov = {
        .iov_base = "",
        .iov_len  = 1,
    };

    struct msghdr msg = {
        .msg_iov        = &iov,
        .msg_iovlen     = 1,
    };

    char cmsgbuf[CMSG_SPACE(sizeof(int))];

    if (fd != -1) {
        // Is the file descriptor actually open?
        if (fcntl(fd, F_GETFD) == -1) {
            if (errno != EBADF) {
                PLOGE("unable to send fd");
            }
            // It's closed, don't send a control message or sendmsg will EBADF.
        } else {
            // It's open, send the file descriptor in a control message.
            msg.msg_control    = cmsgbuf;
            msg.msg_controllen = sizeof(cmsgbuf);

            struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

            cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type  = SCM_RIGHTS;

            *(int *)CMSG_DATA(cmsg) = fd;
        }
    }

    xsendmsg(sockfd, &msg, 0);
}

int read_int(int fd) {
    int val;
    xxread(fd, &val, sizeof(int));
    return val;
}

void write_int(int fd, int val) {
    if (fd < 0) return;
    xwrite(fd, &val, sizeof(int));
}

char* read_string(int fd) {
    int len = read_int(fd);
    if (len > PATH_MAX || len < 0) {
        LOGE("invalid string length %d", len);
        exit(1);
    }
    char* val = xmalloc(sizeof(char) * (len + 1));
    xxread(fd, val, len);
    val[len] = '\0';
    return val;
}

void write_string(int fd, const char* val) {
    if (fd < 0) return;
    int len = strlen(val);
    write_int(fd, len);
    xwrite(fd, val, len);
}
