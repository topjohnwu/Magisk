#define SYS_INLINE

#include "linux_syscall_support.h"

// Some missing declarations
static inline _syscall3(int, faccessat, int, f, const char *, p, int, m)
_syscall2(int, umount2, const char *, t, int, f)
_syscall4(int, renameat, int, o, const char *, op, int, n, const char *, np)
_syscall1(mode_t, umask, mode_t, mask)
_syscall1(int, chroot, const char *, path)
_syscall2(int, nanosleep, const struct kernel_timespec *, req, struct kernel_timespec *, rem)
_syscall5(int, mount, const char *, s, const char *, t,
          const char *, fs, unsigned long, f, const void *, d)
_syscall3(int, symlinkat, const char *, t, int, fd, const char *, l)
_syscall3(int, mkdirat, int, dirfd, const char *, pathname, mode_t, mode)
_syscall4(ssize_t, sendfile, int, out_fd, int, in_fd, off_t *, offset, size_t, count)
_syscall5(int, linkat, int, o, const char *, op, int, n, const char *, np, int, f)
_syscall4(int, mknodat, int, dirfd, const char *, pathname, mode_t, mode, dev_t, dev)
_syscall2(int, fchmod, int, fd, mode_t, mode)
_syscall4(int, fchmodat, int, dirfd, const char *, pathname, mode_t, mode, int, flags)
_syscall5(int, fchownat, int, dirfd, const char *, p, uid_t, owner, gid_t, group, int, flags)
_syscall3(ssize_t, readv, int, fd, const struct kernel_iovec*, v, size_t, c)

#define SYMBOL_ALIAS(from, to) \
__asm__(".global " #from " \n " #from " = " #to)

#define EXPORT_SYMBOL(name) \
SYMBOL_ALIAS(name, sys_##name)

EXPORT_SYMBOL(_exit);
EXPORT_SYMBOL(openat);
EXPORT_SYMBOL(close);
EXPORT_SYMBOL(read);
EXPORT_SYMBOL(symlink);
EXPORT_SYMBOL(write);
EXPORT_SYMBOL(writev);
EXPORT_SYMBOL(unlink);
EXPORT_SYMBOL(mmap);
EXPORT_SYMBOL(munmap);
EXPORT_SYMBOL(mremap);
EXPORT_SYMBOL(readlink);
EXPORT_SYMBOL(unlinkat);
EXPORT_SYMBOL(getpid);
EXPORT_SYMBOL(chdir);
EXPORT_SYMBOL(umount2);
EXPORT_SYMBOL(readlinkat);
EXPORT_SYMBOL(renameat);
EXPORT_SYMBOL(umask);
EXPORT_SYMBOL(chroot);
EXPORT_SYMBOL(mount);
EXPORT_SYMBOL(symlinkat);
EXPORT_SYMBOL(stat);
EXPORT_SYMBOL(lstat);
EXPORT_SYMBOL(statfs);
EXPORT_SYMBOL(mkdirat);
EXPORT_SYMBOL(ioctl);
EXPORT_SYMBOL(fork);
EXPORT_SYMBOL(sendfile);
EXPORT_SYMBOL(ftruncate);
EXPORT_SYMBOL(linkat);
EXPORT_SYMBOL(mknodat);
EXPORT_SYMBOL(fchmod);
EXPORT_SYMBOL(fchmodat);
EXPORT_SYMBOL(fchownat);
EXPORT_SYMBOL(readv);
EXPORT_SYMBOL(lseek);
EXPORT_SYMBOL(execve);
EXPORT_SYMBOL(getdents64);

SYMBOL_ALIAS(exit, _exit);

#if defined(__LP64__)

EXPORT_SYMBOL(fstat);
EXPORT_SYMBOL(newfstatat);
SYMBOL_ALIAS(fstatat, newfstatat);

#else

EXPORT_SYMBOL(fstat64);
EXPORT_SYMBOL(fstatat64);
SYMBOL_ALIAS(fstat, fstat64);
SYMBOL_ALIAS(fstatat, fstatat64);

#endif

int fchown(int fd, uid_t owner, gid_t group) {
    return fchownat(fd, "", owner, group, AT_EMPTY_PATH);
}

int lchown(const char* path, uid_t uid, gid_t gid) {
    return fchownat(AT_FDCWD, path, uid, gid, AT_SYMLINK_NOFOLLOW);
}

int chown(const char* path, uid_t uid, gid_t gid) {
    return fchownat(AT_FDCWD, path, uid, gid, 0);
}

int chmod(const char* path, mode_t mode) {
    return sys_fchmodat(AT_FDCWD, path, mode, 0);
}

int mkfifoat(int fd, const char* path, mode_t mode) {
    return sys_mknodat(fd, path, (mode & ~S_IFMT) | S_IFIFO, 0);
}

int mkfifo(const char* path, mode_t mode) {
    return mkfifoat(AT_FDCWD, path, mode);
}

int mknod(const char* path, mode_t mode, dev_t dev) {
    return sys_mknodat(AT_FDCWD, path, mode, dev);
}

int link(const char *oldpath, const char *newpath) {
    return sys_linkat(AT_FDCWD, oldpath, AT_FDCWD, newpath, 0);
}

int rmdir(const char *path) {
    return sys_unlinkat(AT_FDCWD, path, AT_REMOVEDIR);
}

int mkdir(const char *pathname, mode_t mode) {
    return sys_mkdirat(AT_FDCWD, pathname, mode);
}

int symlink(const char *target, const char *linkpath) {
    return sys_symlinkat(target, AT_FDCWD, linkpath);
}

int rename(const char *oldpath, const char *newpath) {
    return sys_renameat(AT_FDCWD, oldpath, AT_FDCWD, newpath);
}

int access(const char* path, int mode) {
    return faccessat(AT_FDCWD, path, mode, 0);
}

int remove(const char *path) {
    int r = sys_unlinkat(AT_FDCWD, path, 0);
    if (r < 0 && errno == EISDIR) {
        r = sys_unlinkat(AT_FDCWD, path, AT_REMOVEDIR);
    }
    return r;
}

// Source: bionic/libc/bionic/abort.cpp
void abort() {
    // Don't block SIGABRT to give any signal handler a chance; we ignore
    // any errors -- X311J doesn't allow abort to return anyway.
    struct kernel_sigset_t mask;
    sys_sigfillset(&mask);
    sys_sigdelset(&mask, SIGABRT);

    sys_sigprocmask(SIG_SETMASK, &mask, NULL);
    sys_raise(SIGABRT);

    // If SIGABRT is ignored or it's caught and the handler returns,
    // remove the SIGABRT signal handler and raise SIGABRT again.
    struct kernel_sigaction sa = { .sa_handler_ = SIG_DFL, .sa_flags = SA_RESTART };
    sys_sigaction(SIGABRT, &sa, NULL);

    sys_sigprocmask(SIG_SETMASK, &mask, NULL);
    sys_raise(SIGABRT);

    // If we get this far, just exit.
    _exit(127);
}

// Source: bionic/libc/bionic/usleep.cpp
int usleep(useconds_t us) {
    struct kernel_timespec ts;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000;
    return sys_nanosleep(&ts, NULL);
}

// Source: bionic/libc/bionic/faccessat.cpp
int faccessat(int dirfd, const char *pathname, int mode, int flags) {
    // "The mode specifies the accessibility check(s) to be performed,
    // and is either the value F_OK, or a mask consisting of the
    // bitwise OR of one or more of R_OK, W_OK, and X_OK."
    if ((mode != F_OK) && ((mode & ~(R_OK | W_OK | X_OK)) != 0) &&
        ((mode & (R_OK | W_OK | X_OK)) == 0)) {
        errno = EINVAL;
        return -1;
    }

    if (flags != 0) {
        // We deliberately don't support AT_SYMLINK_NOFOLLOW, a glibc
        // only feature which is error prone and dangerous.
        // More details at http://permalink.gmane.org/gmane.linux.lib.musl.general/6952
        //
        // AT_EACCESS isn't supported either. Android doesn't have setuid
        // programs, and never runs code with euid!=uid.
        //
        // We could use faccessat2(2) from Linux 5.8, but since we don't want the
        // first feature and don't need the second, we just reject such requests.
        errno = EINVAL;
        return -1;
    }

    return sys_faccessat(dirfd, pathname, mode);
}

int open(const char *pathname, int flags, ...) {
    int mode = 0;

    if (((flags & O_CREAT) == O_CREAT) || ((flags & O_TMPFILE) == O_TMPFILE)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);
    }

#if !defined(__LP64__)
    flags |= O_LARGEFILE;
#endif

    return sys_openat(AT_FDCWD, pathname, flags, mode);
}
