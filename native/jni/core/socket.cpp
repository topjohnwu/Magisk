/* socket.c - All socket related operations
 */

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <endian.h>

#include "daemon.h"
#include "logging.h"
#include "utils.h"
#include "magisk.h"

#define ABS_SOCKET_LEN(sun) (sizeof(sun->sun_family) + strlen(sun->sun_path + 1) + 1)

socklen_t setup_sockaddr(struct sockaddr_un *sun, const char *name) {
	memset(sun, 0, sizeof(*sun));
	sun->sun_family = AF_LOCAL;
	strcpy(sun->sun_path + 1, name);
	return ABS_SOCKET_LEN(sun);
}

int create_rand_socket(struct sockaddr_un *sun) {
	memset(sun, 0, sizeof(*sun));
	sun->sun_family = AF_LOCAL;
	gen_rand_str(sun->sun_path + 1, 9);
	int fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	xbind(fd, (struct sockaddr*) sun, ABS_SOCKET_LEN(sun));
	xlisten(fd, 1);
	return fd;
}

int socket_accept(int sockfd, int timeout) {
	struct pollfd pfd = {
		.fd = sockfd,
		.events = POLL_IN
	};
	return xpoll(&pfd, 1, timeout * 1000) <= 0 ? -1 : xaccept4(sockfd, NULL, NULL, SOCK_CLOEXEC);
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
	struct cmsghdr *cmsg;

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

	cmsg = CMSG_FIRSTHDR(&msg);

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
	char junk[] = { '\0' };
	struct iovec iov = {
		.iov_base = junk,
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
	if (xxread(fd, &val, sizeof(val)) != sizeof(val))
		return -1;
	return val;
}

int read_int_be(int fd) {
	uint32_t val;
	if (xxread(fd, &val, sizeof(val)) != sizeof(val))
		return -1;
	return ntohl(val);
}

void write_int(int fd, int val) {
	if (fd < 0) return;
	xwrite(fd, &val, sizeof(val));
}

void write_int_be(int fd, int val) {
	uint32_t nl = htonl(val);
	xwrite(fd, &nl, sizeof(nl));
}

static char *rd_str(int fd, int len) {
	char *val = (char *) xmalloc(sizeof(char) * (len + 1));
	xxread(fd, val, len);
	val[len] = '\0';
	return val;
}

char* read_string(int fd) {
	int len = read_int(fd);
	return rd_str(fd, len);
}

char* read_string_be(int fd) {
	int len = read_int_be(fd);
	return rd_str(fd, len);
}

void write_string(int fd, const char *val) {
	if (fd < 0) return;
	int len = strlen(val);
	write_int(fd, len);
	xwrite(fd, val, len);
}

void write_string_be(int fd, const char *val) {
	int len = strlen(val);
	write_int_be(fd, len);
	xwrite(fd, val, len);
}

void write_key_value(int fd, const char *key, const char *val) {
	write_string_be(fd, key);
	write_string_be(fd, val);
}

void write_key_token(int fd, const char *key, int tok) {
	char val[16];
	sprintf(val, "%d", tok);
	write_key_value(fd, key, val);
}
