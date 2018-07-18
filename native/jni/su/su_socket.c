/* su_socket.c - Functions for communication to client
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"
#include "su.h"
#include "magiskpolicy.h"

int socket_create_temp(char *path, size_t len) {
    int fd;
    struct sockaddr_un sun;

    fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fcntl(fd, F_SETFD, FD_CLOEXEC)) {
        PLOGE("fcntl FD_CLOEXEC");
    }

    memset(&sun, 0, sizeof(sun));
    sun.sun_family = AF_LOCAL;
    snprintf(path, len, "/dev/.socket%d", getpid());
    strcpy(sun.sun_path, path);

    /*
     * Delete the socket to protect from situations when
     * something bad occured previously and the kernel reused pid from that process.
     * Small probability, isn't it.
     */
    unlink(path);

    xbind(fd, (struct sockaddr*) &sun, sizeof(sun));
    xlisten(fd, 1);

    // Set attributes so requester can access it
    setfilecon(path, "u:object_r:"SEPOL_FILE_DOMAIN":s0");
    chown(path, su_ctx->info->manager_stat.st_uid, su_ctx->info->manager_stat.st_gid);

    return fd;
}

int socket_accept(int serv_fd) {
    struct timeval tv;
    fd_set fds;
    int rc;

    /* Wait 60 seconds for a connection, then give up. */
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(serv_fd, &fds);
    do {
        rc = select(serv_fd + 1, &fds, NULL, NULL, &tv);
    } while (rc < 0 && errno == EINTR);
    if (rc < 1) {
        PLOGE("select");
    }

    return xaccept4(serv_fd, NULL, NULL, SOCK_CLOEXEC);
}

#define write_data(fd, data, data_len)              \
do {                                                \
    uint32_t __len = htonl(data_len);               \
    __len = write((fd), &__len, sizeof(__len));     \
    if (__len != sizeof(__len)) {                   \
        PLOGE("write(" #data ")");                  \
    }                                               \
    __len = write((fd), data, data_len);            \
    if (__len != data_len) {                        \
        PLOGE("write(" #data ")");                  \
    }                                               \
} while (0)

#define write_string_data(fd, name, data)        \
do {                                        \
    write_data(fd, name, strlen(name));     \
    write_data(fd, data, strlen(data));     \
} while (0)

// stringify everything.
#define write_token(fd, name, data)         \
do {                                        \
    char buf[16];                           \
    snprintf(buf, sizeof(buf), "%d", data); \
    write_string_data(fd, name, buf);            \
} while (0)

void socket_send_request(int fd, const struct su_context *ctx) {
    write_token(fd, "uid", ctx->info->uid);
    write_token(fd, "eof", 1);
}

void socket_receive_result(int fd, char *result, ssize_t result_len) {
    ssize_t len;
    len = xread(fd, result, result_len - 1);
    result[len] = '\0';
}
