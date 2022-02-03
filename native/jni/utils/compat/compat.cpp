// This file implements all missing symbols that should exist in normal API 21
// libc.a but missing in our extremely lean libc.a replacements.

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <mntent.h>
#include <unistd.h>
#include <sys/syscall.h>

extern "C" {

// Original source: https://github.com/freebsd/freebsd/blob/master/contrib/file/src/getline.c
// License: BSD, full copyright notice please check original source

ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp) {
    char *ptr, *eptr;

    if (*buf == nullptr || *bufsiz == 0) {
        *bufsiz = BUFSIZ;
        if ((*buf = (char *) malloc(*bufsiz)) == nullptr)
            return -1;
    }

    for (ptr = *buf, eptr = *buf + *bufsiz;;) {
        int c = fgetc(fp);
        if (c == -1) {
            if (feof(fp))
                return ptr == *buf ? -1 : ptr - *buf;
            else
                return -1;
        }
        *ptr++ = c;
        if (c == delimiter) {
            *ptr = '\0';
            return ptr - *buf;
        }
        if (ptr + 2 >= eptr) {
            char *nbuf;
            size_t nbufsiz = *bufsiz * 2;
            ssize_t d = ptr - *buf;
            if ((nbuf = (char *) realloc(*buf, nbufsiz)) == nullptr)
                return -1;
            *buf = nbuf;
            *bufsiz = nbufsiz;
            eptr = nbuf + nbufsiz;
            ptr = nbuf + d;
        }
    }
}

ssize_t getline(char **buf, size_t *bufsiz, FILE *fp) {
    return getdelim(buf, bufsiz, '\n', fp);
}

FILE *setmntent(const char *path, const char *mode) {
    return fopen(path, mode);
}

int endmntent(FILE *fp) {
    if (fp != nullptr) {
        fclose(fp);
    }
    return 1;
}

// Missing system call wrappers

int setns(int fd, int nstype) {
    return syscall(__NR_setns, fd, nstype);
}

int unshare(int flags) {
    return syscall(__NR_unshare, flags);
}

int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
    return syscall(__NR_accept4, sockfd, addr, addrlen, flags);
}

int dup3(int oldfd, int newfd, int flags) {
    return syscall(__NR_dup3, oldfd, newfd, flags);
}

ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) {
    return syscall(__NR_readlinkat, dirfd, pathname, buf, bufsiz);
}

int symlinkat(const char *target, int newdirfd, const char *linkpath) {
    return syscall(__NR_symlinkat, target, newdirfd, linkpath);
}

int linkat(int olddirfd, const char *oldpath,
           int newdirfd, const char *newpath, int flags) {
    return syscall(__NR_linkat, olddirfd, oldpath, newdirfd, newpath, flags);
}

int inotify_init1(int flags) {
    return syscall(__NR_inotify_init1, flags);
}

int faccessat(int dirfd, const char *pathname, int mode, int flags) {
    return syscall(__NR_faccessat, dirfd, pathname, mode, flags);
}

#define SPLIT_64(v) (unsigned)((v) & 0xFFFFFFFF), (unsigned)((v) >> 32)

#if defined(__arm__)
// Why the additional 0 is required: https://man7.org/linux/man-pages/man2/syscall.2.html
int ftruncate64(int fd, off64_t length) {
    return syscall(__NR_ftruncate64, fd, 0, SPLIT_64(length));
}
#elif defined(__i386__)
int ftruncate64(int fd, off64_t length) {
    return syscall(__NR_ftruncate64, fd, SPLIT_64(length));
}
#endif

#if !defined(__LP64__)
void android_set_abort_message(const char *) {}

// Original source: <android/legacy_signal_inlines.h>
int sigaddset(sigset_t *set, int signum) {
    /* Signal numbers start at 1, but bit positions start at 0. */
    int bit = signum - 1;
    auto *local_set = (unsigned long *)set;
    if (set == nullptr || bit < 0 || bit >= (int)(8 * sizeof(sigset_t))) {
        errno = EINVAL;
        return -1;
    }
    local_set[bit / LONG_BIT] |= 1UL << (bit % LONG_BIT);
    return 0;
}
int sigemptyset(sigset_t *set) {
    if (set == nullptr) {
        errno = EINVAL;
        return -1;
    }
    memset(set, 0, sizeof(sigset_t));
    return 0;
}

#include "fortify.hpp"

#endif

} // extern "C"
