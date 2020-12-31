#pragma once

#include <sys/syscall.h>
#include <unistd.h>
#include <mntent.h>

// Missing libc functions
#define getline       compat_getline
#define getdelim      compat_getdelim
#define getmntent_r   compat_getmntent_r
#define setmntent     compat_setmntent
#define endmntent     compat_endmntent
#define hasmntopt     compat_hasmntopt

// Missing syscall wrappers
#define setns         compat_setns
#define unshare       compat_unshare
#define accept4       compat_accept4
#define dup3          compat_dup3
#define readlinkat    compat_readlinkat
#define symlinkat     compat_symlinkat
#define linkat        compat_linkat
#define inotify_init1 compat_inotify_init1
#define faccessat     compat_faccessat

ssize_t compat_getline(char **lineptr, size_t *n, FILE *stream);
ssize_t compat_getdelim(char **lineptr, size_t *n, int delim, FILE *stream);
struct mntent *compat_getmntent_r(FILE* fp, struct mntent* e, char* buf, int buf_len);
FILE *compat_setmntent(const char* path, const char* mode);
int compat_endmntent(FILE* fp);
char *compat_hasmntopt(const struct mntent* mnt, const char* opt);

static inline int compat_setns(int fd, int nstype) {
    return syscall(__NR_setns, fd, nstype);
}

static inline int compat_unshare(int flags) {
    return syscall(__NR_unshare, flags);
}

static inline int compat_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
    return syscall(__NR_accept4, sockfd, addr, addrlen, flags);
}

static inline int compat_dup3(int oldfd, int newfd, int flags) {
    return syscall(__NR_dup3, oldfd, newfd, flags);
}

static inline ssize_t compat_readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) {
    return syscall(__NR_readlinkat, dirfd, pathname, buf, bufsiz);
}

static inline int compat_symlinkat(const char *target, int newdirfd, const char *linkpath) {
    return syscall(__NR_symlinkat, target, newdirfd, linkpath);
}

static inline int compat_linkat(int olddirfd, const char *oldpath,
        int newdirfd, const char *newpath, int flags) {
    return syscall(__NR_linkat, olddirfd, oldpath, newdirfd, newpath, flags);
}

static inline int compat_inotify_init1(int flags) {
    return syscall(__NR_inotify_init1, flags);
}

static inline int compat_faccessat(int dirfd, const char *pathname, int mode, int flags) {
    return syscall(__NR_faccessat, dirfd, pathname, mode, flags);
}
