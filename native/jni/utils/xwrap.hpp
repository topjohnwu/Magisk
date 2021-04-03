#pragma once

#include <dirent.h>
#include <stdio.h>
#include <poll.h>
#include <fcntl.h>

FILE *xfopen(const char *pathname, const char *mode);
FILE *xfdopen(int fd, const char *mode);
int xopen(const char *pathname, int flags);
int xopen(const char *pathname, int flags, mode_t mode);
int xopenat(int dirfd, const char *pathname, int flags);
int xopenat(int dirfd, const char *pathname, int flags, mode_t mode);
ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);
ssize_t xxread(int fd, void *buf, size_t count);
int xpipe2(int pipefd[2], int flags);
int xsetns(int fd, int nstype);
int xunshare(int flags);
DIR *xopendir(const char *name);
DIR *xfdopendir(int fd);
struct dirent *xreaddir(DIR *dirp);
pid_t xsetsid();
int xsocket(int domain, int type, int protocol);
int xbind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int xconnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int xlisten(int sockfd, int backlog);
int xaccept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
extern "C" void *xmalloc(size_t size);
extern "C" void *xcalloc(size_t nmemb, size_t size);
extern "C" void *xrealloc(void *ptr, size_t size);
ssize_t xsendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t xrecvmsg(int sockfd, struct msghdr *msg, int flags);
int xpthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*start_routine) (void *), void *arg);
int xaccess(const char *path, int mode);
int xstat(const char *pathname, struct stat *buf);
int xlstat(const char *pathname, struct stat *buf);
int xfstat(int fd, struct stat *buf);
int xdup(int fd);
int xdup2(int oldfd, int newfd);
int xdup3(int oldfd, int newfd, int flags);
ssize_t xreadlink(const char *pathname, char *buf, size_t bufsiz);
ssize_t xreadlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
int xfaccessat(int dirfd, const char *pathname);
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
int xpoll(struct pollfd *fds, nfds_t nfds, int timeout);
int xinotify_init1(int flags);
char *xrealpath(const char *path, char *resolved_path);
int xmknod(const char *pathname, mode_t mode, dev_t dev);
long xptrace(int request, pid_t pid, void *addr = nullptr, void *data = nullptr);
static inline long xptrace(int request, pid_t pid, void *addr, uintptr_t data) {
    return xptrace(request, pid, addr, reinterpret_cast<void *>(data));
}
#define WEVENT(s) (((s) & 0xffff0000) >> 16)
