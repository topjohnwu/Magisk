// This file implements all missing symbols that should exist in normal API 23
// libc.a but missing in our extremely lean libc.a replacements.

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/xattr.h>

extern "C" {

#if !defined(__LP64__)

// Add "hacky" libc.a missing symbols back
// All symbols in this file are weak, so a vanilla NDK should still link properly

#include "fortify.hpp"

// Original source: https://github.com/freebsd/freebsd/blob/master/contrib/file/src/getline.c
// License: BSD, full copyright notice please check original source

[[gnu::weak]]
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

[[gnu::weak]]
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp) {
    return getdelim(buf, bufsiz, '\n', fp);
}

// Missing system call wrappers

[[gnu::weak]]
ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) {
    return syscall(__NR_readlinkat, dirfd, pathname, buf, bufsiz);
}

[[gnu::weak]]
int symlinkat(const char *target, int newdirfd, const char *linkpath) {
    return syscall(__NR_symlinkat, target, newdirfd, linkpath);
}

[[gnu::weak]]
int linkat(int olddirfd, const char *oldpath,
           int newdirfd, const char *newpath, int flags) {
    return syscall(__NR_linkat, olddirfd, oldpath, newdirfd, newpath, flags);
}

[[gnu::weak]]
int faccessat(int dirfd, const char *pathname, int mode, int flags) {
    return syscall(__NR_faccessat, dirfd, pathname, mode, flags);
}

[[gnu::weak]]
int mkfifo(const char *path, mode_t mode) {
    return mknod(path, (mode & ~S_IFMT) | S_IFIFO, 0);
}

[[gnu::weak]]
int fsetxattr(int fd, const char *name, const void *value, size_t size, int flags) {
    return syscall(__NR_fsetxattr, fd, name, value, size, flags);
}

[[gnu::weak]]
int lsetxattr(const char *path, const char *name, const void *value, size_t size, int flags) {
    return syscall(__NR_lsetxattr, path, name, value, size, flags);
}

#define SPLIT_64(v) (unsigned)((v) & 0xFFFFFFFF), (unsigned)((v) >> 32)

#if defined(__arm__)
// Why the additional 0 is required: https://man7.org/linux/man-pages/man2/syscall.2.html
[[gnu::weak]]
int ftruncate64(int fd, off64_t length) {
    return syscall(__NR_ftruncate64, fd, 0, SPLIT_64(length));
}
#elif defined(__i386__)
[[gnu::weak]]
int ftruncate64(int fd, off64_t length) {
    return syscall(__NR_ftruncate64, fd, SPLIT_64(length));
}
#endif

[[gnu::weak]]
void android_set_abort_message(const char *) {}

extern FILE __sF[];

[[gnu::weak]] FILE* stdin = &__sF[0];
[[gnu::weak]] FILE* stdout = &__sF[1];
[[gnu::weak]] FILE* stderr = &__sF[2];

#endif // !defined(__LP64__)

[[maybe_unused]]
int __wrap_renameat(int old_dir_fd, const char *old_path, int new_dir_fd, const char *new_path) {
    long out = syscall(__NR_renameat, old_dir_fd, old_path, new_dir_fd, new_path);
    if (out == -1 && errno == ENOSYS) {
        out = syscall(__NR_renameat2, old_dir_fd, old_path, new_dir_fd, new_path, 0);
    }
    return static_cast<int>(out);
}

[[maybe_unused]]
int __wrap_rename(const char *old_path, const char *new_path) {
    return __wrap_renameat(AT_FDCWD, old_path, AT_FDCWD, new_path);
}

} // extern "C"
