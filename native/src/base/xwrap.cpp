#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/ptrace.h>
#include <sys/inotify.h>

#include <base.hpp>

using namespace std;

// Write exact same size as count
ssize_t xwrite(int fd, const void *buf, size_t count) {
    size_t write_sz = 0;
    ssize_t ret;
    do {
        ret = write(fd, (byte *) buf + write_sz, count - write_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            PLOGE("write");
            return ret;
        }
        write_sz += ret;
    } while (write_sz != count && ret != 0);
    if (write_sz != count) {
        PLOGE("write (%zu != %zu)", count, write_sz);
    }
    return write_sz;
}

// Read error other than EOF
ssize_t xread(int fd, void *buf, size_t count) {
    int ret = read(fd, buf, count);
    if (ret < 0) {
        PLOGE("read");
    }
    return ret;
}

// Read exact same size as count
ssize_t xxread(int fd, void *buf, size_t count) {
    size_t read_sz = 0;
    ssize_t ret;
    do {
        ret = read(fd, (byte *) buf + read_sz, count - read_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            PLOGE("read");
            return ret;
        }
        read_sz += ret;
    } while (read_sz != count && ret != 0);
    if (read_sz != count) {
        PLOGE("read (%zu != %zu)", count, read_sz);
    }
    return read_sz;
}

off_t xlseek(int fd, off_t offset, int whence) {
    off_t ret = lseek(fd, offset, whence);
    if (ret < 0) {
        PLOGE("lseek");
    }
    return ret;
}

int xpipe2(int pipefd[2], int flags) {
    int ret = pipe2(pipefd, flags);
    if (ret < 0) {
        PLOGE("pipe2");
    }
    return ret;
}

struct dirent *xreaddir(DIR *dirp) {
    errno = 0;
    for (dirent *e;;) {
        e = readdir(dirp);
        if (e == nullptr) {
            if (errno)
                PLOGE("readdir");
            return nullptr;
        } else if (e->d_name == "."sv || e->d_name == ".."sv) {
            // Filter . and .. for users
            continue;
        }
        return e;
    }
}

int xpthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*start_routine) (void *), void *arg) {
    errno = pthread_create(thread, attr, start_routine, arg);
    if (errno) {
        PLOGE("pthread_create");
    }
    return errno;
}

ssize_t xreadlink(const char *pathname, char *buf, size_t bufsiz) {
    ssize_t ret = readlink(pathname, buf, bufsiz);
    if (ret < 0) {
        PLOGE("readlink %s", pathname);
    } else {
        buf[ret] = '\0';
    }
    return ret;
}

ssize_t xreadlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz) {
    // readlinkat() may fail on x86 platform, returning random value
    // instead of number of bytes placed in buf (length of link)
#if defined(__i386__) || defined(__x86_64__)
    memset(buf, 0, bufsiz);
    ssize_t ret = readlinkat(dirfd, pathname, buf, bufsiz);
    if (ret < 0) {
        PLOGE("readlinkat %s", pathname);
    }
    return ret;
#else
    ssize_t ret = readlinkat(dirfd, pathname, buf, bufsiz);
    if (ret < 0) {
        PLOGE("readlinkat %s", pathname);
    } else {
        buf[ret] = '\0';
    }
    return ret;
#endif
}

int xfaccessat(int dirfd, const char *pathname) {
    int ret = faccessat(dirfd, pathname, F_OK, 0);
    if (ret < 0) {
        PLOGE("faccessat %s", pathname);
    }
#if defined(__i386__) || defined(__x86_64__)
    if (ret > 0 && errno == 0) {
        LOGD("faccessat success but ret is %d\n", ret);
        ret = 0;
    }
#endif
    return ret;
}

int xmkdirs(const char *pathname, mode_t mode) {
    int ret = mkdirs(pathname, mode);
    if (ret < 0) {
        PLOGE("mkdirs %s", pathname);
    }
    return ret;
}

void *xmmap(void *addr, size_t length, int prot, int flags,
    int fd, off_t offset) {
    void *ret = mmap(addr, length, prot, flags, fd, offset);
    if (ret == MAP_FAILED) {
        PLOGE("mmap");
        return nullptr;
    }
    return ret;
}

ssize_t xsendfile(int out_fd, int in_fd, off_t *offset, size_t count) {
    ssize_t ret = sendfile(out_fd, in_fd, offset, count);
    if (ret < 0) {
        PLOGE("sendfile");
    }
    return ret;
}

int xpoll(struct pollfd *fds, nfds_t nfds, int timeout) {
    int ret = poll(fds, nfds, timeout);
    if (ret < 0) {
        PLOGE("poll");
    }
    return ret;
}

char *xrealpath(const char *path, char *resolved_path) {
    char buf[PATH_MAX];
    char *ret = realpath(path, buf);
    if (ret == nullptr) {
        PLOGE("xrealpath");
    } else {
        strcpy(resolved_path, buf);
    }
    return ret;
}
