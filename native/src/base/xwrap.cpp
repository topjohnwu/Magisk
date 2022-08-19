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

ssize_t xwrite(int fd, const void *buf, size_t count) {
    return rust::xwrite(fd, rust::Slice(static_cast<const uint8_t *>(buf), count));
}

ssize_t xread(int fd, void *buf, size_t count) {
    return rust::xread(fd, rust::Slice(static_cast<uint8_t *>(buf), count));
}

ssize_t xxread(int fd, void *buf, size_t count) {
    return rust::xxread(fd, rust::Slice(static_cast<uint8_t *>(buf), count));
}

dirent *xreaddir(DIR *dirp) {
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
