#pragma once

#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <poll.h>
#include <fcntl.h>

extern "C" {

FILE *xfopen(const char *pathname, const char *mode);
FILE *xfdopen(int fd, const char *mode);
int xopen(const char *pathname, int flags, mode_t mode = 0);
int xopenat(int dirfd, const char *pathname, int flags, mode_t mode = 0);
ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t xxread(int fd, void *buf, size_t count);
off64_t xlseek64(int fd, off64_t offset, int whence);
int xsetns(int fd, int nstype);
int xunshare(int flags);
DIR *xopendir(const char *name);
DIR *xfdopendir(int fd);
dirent *xreaddir(DIR *dirp);
pid_t xsetsid();
int xsocket(int domain, int type, int protocol);
int xbind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int xlisten(int sockfd, int backlog);
int xaccept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
ssize_t xsendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t xrecvmsg(int sockfd, struct msghdr *msg, int flags);
int xaccess(const char *path, int mode);
int xfaccessat(int dirfd, const char *pathname, int mode, int flags);
int xstat(const char *pathname, struct stat *buf);
int xlstat(const char *pathname, struct stat *buf);
int xfstat(int fd, struct stat *buf);
int xfstatat(int dirfd, const char *pathname, struct stat *buf, int flags);
int xdup(int fd);
int xdup2(int oldfd, int newfd);
int xdup3(int oldfd, int newfd, int flags);
ssize_t xreadlink(const char * __restrict__ pathname, char * __restrict__ buf, size_t bufsiz);
ssize_t xreadlinkat(
        int dirfd, const char * __restrict__ pathname, char * __restrict__ buf, size_t bufsiz);
int xsymlink(const char *target, const char *linkpath);
int xsymlinkat(const char *target, int newdirfd, const char *linkpath);
int xlinkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags);
int xmount(const char *source, const char *target,
           const char *filesystemtype, unsigned long mountflags,
           const void *data);
int xumount(const char *target);
int xumount2(const char *target, int flags);
int xrename(const char *oldpath, const char *newpath);
int xmkdir(const char *pathname, mode_t mode);
int xmkdirs(const char *pathname, mode_t mode);
int xmkdirat(int dirfd, const char *pathname, mode_t mode);
void *xmmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
ssize_t xsendfile(int out_fd, int in_fd, off_t *offset, size_t count);
pid_t xfork();
int xpoll(pollfd *fds, nfds_t nfds, int timeout);
ssize_t xrealpath(const char * __restrict__ path, char * __restrict__ buf, size_t bufsiz);
int xmknod(const char * pathname, mode_t mode, dev_t dev);

} // extern "C"
