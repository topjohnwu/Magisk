#include <fcntl.h>
#include <endian.h>

#include <socket.hpp>
#include <utils.hpp>

using namespace std;

static size_t socket_len(sockaddr_un *sun) {
    if (sun->sun_path[0])
        return sizeof(sa_family_t) + strlen(sun->sun_path) + 1;
    else
        return sizeof(sa_family_t) + strlen(sun->sun_path + 1) + 1;
}

socklen_t setup_sockaddr(sockaddr_un *sun, const char *name) {
    memset(sun, 0, sizeof(*sun));
    sun->sun_family = AF_UNIX;
    strcpy(sun->sun_path + 1, name);
    return socket_len(sun);
}

int socket_accept(int sockfd, int timeout) {
    struct pollfd pfd = {
        .fd = sockfd,
        .events = POLL_IN
    };
    return xpoll(&pfd, 1, timeout * 1000) <= 0 ? -1 : xaccept4(sockfd, nullptr, nullptr, SOCK_CLOEXEC);
}

void get_client_cred(int fd, struct ucred *cred) {
    socklen_t ucred_length = sizeof(*cred);
    getsockopt(fd, SOL_SOCKET, SO_PEERCRED, cred, &ucred_length);
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

    if (cmsg             == nullptr                  ||
        cmsg->cmsg_len   != CMSG_LEN(sizeof(int)) ||
        cmsg->cmsg_level != SOL_SOCKET            ||
        cmsg->cmsg_type  != SCM_RIGHTS) {
error:
        LOGE("unable to read fd\n");
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

void read_string(int fd, std::string &str) {
    int len = read_int(fd);
    str.clear();
    str.resize(len);
    xxread(fd, str.data(), len);
}

string read_string(int fd) {
    string str;
    read_string(fd, str);
    return str;
}

void write_string(int fd, string_view str) {
    if (fd < 0) return;
    write_int(fd, str.size());
    xwrite(fd, str.data(), str.size());
}
