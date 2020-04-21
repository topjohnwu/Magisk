#pragma once

#include <sys/syscall.h>
#include <unistd.h>
#include <mntent.h>

#define getline       __getline
#define getdelim      __getdelim
#define setns         __setns
#define unshare       __unshare
#define accept4       __accept4
#define dup3          __dup3
#define readlinkat    __readlinkat
#define symlinkat     __symlinkat
#define linkat        __linkat
#define inotify_init1 __inotify_init1
#define getmntent_r   __getmntent_r
#define setmntent     __setmntent
#define endmntent     __endmntent
#define hasmntopt     __hasmntopt
#define faccessat     __faccessat

ssize_t __getline(char **lineptr, size_t *n, FILE *stream);
ssize_t __getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
struct mntent *__getmntent_r(FILE* fp, struct mntent* e, char* buf, int buf_len);
FILE *__setmntent(const char* path, const char* mode);
int __endmntent(FILE* fp);
char *__hasmntopt(const struct mntent* mnt, const char* opt);

static inline int __setns(int fd, int nstype) {
	return syscall(__NR_setns, fd, nstype);
}

static inline int __unshare(int flags) {
	return syscall(__NR_unshare, flags);
}

static inline int __accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
	return syscall(__NR_accept4, sockfd, addr, addrlen, flags);
}

static inline int __dup3(int oldfd, int newfd, int flags) {
	return syscall(__NR_dup3, oldfd, newfd, flags);
}

static inline ssize_t __readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) {
	return syscall(__NR_readlinkat, dirfd, pathname, buf, bufsiz);
}

static inline int __symlinkat(const char *target, int newdirfd, const char *linkpath) {
	return syscall(__NR_symlinkat, target, newdirfd, linkpath);
}

static inline int __linkat(int olddirfd, const char *oldpath,
		int newdirfd, const char *newpath, int flags) {
	return syscall(__NR_linkat, olddirfd, oldpath, newdirfd, newpath, flags);
}

static inline int __inotify_init1(int flags) {
	return syscall(__NR_inotify_init1, flags);
}

static inline int __faccessat(int dirfd, const char *pathname, int mode, int flags) {
	return syscall(__NR_faccessat, dirfd, pathname, mode, flags);
}
