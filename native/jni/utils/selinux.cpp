#include <sys/xattr.h>

#include <utils.hpp>
#include <selinux.hpp>

using namespace std;

// Stub implementation

static int stub(const char *) { return 0; }

static int stub(const char *, const char *) { return 0; }

static int stub(const char *, char **ctx) {
    *ctx = strdup("");
    return 0;
}

static int stub(int, const char *) { return 0; }

static int stub(int, char **ctx) {
    *ctx = strdup("");
    return 0;
}

// Builtin implementation

static void __freecon(char *s) {
    free(s);
}

static int __setcon(const char *ctx) {
    int fd = open("/proc/self/attr/current", O_WRONLY | O_CLOEXEC);
    if (fd < 0)
        return fd;
    size_t len = strlen(ctx) + 1;
    int rc = write(fd, ctx, len);
    close(fd);
    return rc != len;
}

static int __getfilecon(const char *path, char **ctx) {
    char buf[1024];
    int rc = syscall(__NR_getxattr, path, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
    if (rc >= 0)
        *ctx = strdup(buf);
    return rc;
}

static int __lgetfilecon(const char *path, char **ctx) {
    char buf[1024];
    int rc = syscall(__NR_lgetxattr, path, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
    if (rc >= 0)
        *ctx = strdup(buf);
    return rc;
}

static int __fgetfilecon(int fd, char **ctx) {
    char buf[1024];
    int rc = syscall(__NR_fgetxattr, fd, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
    if (rc >= 0)
        *ctx = strdup(buf);
    return rc;
}

static int __setfilecon(const char *path, const char *ctx) {
    return syscall(__NR_setxattr, path, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
}

static int __lsetfilecon(const char *path, const char *ctx) {
    return syscall(__NR_lsetxattr, path, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
}

static int __fsetfilecon(int fd, const char *ctx) {
    return syscall(__NR_fsetxattr, fd, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
}

// Function pointers

void (*freecon)(char *) = __freecon;
int (*setcon)(const char *) = stub;
int (*getfilecon)(const char *, char **) = stub;
int (*lgetfilecon)(const char *, char **) = stub;
int (*fgetfilecon)(int, char **) = stub;
int (*setfilecon)(const char *, const char *) = stub;
int (*lsetfilecon)(const char *, const char *) = stub;
int (*fsetfilecon)(int, const char *) = stub;

void getfilecon_at(int dirfd, const char *name, char **con) {
    char path[4096];
    fd_pathat(dirfd, name, path, sizeof(path));
    if (lgetfilecon(path, con))
        *con = strdup("");
}

void setfilecon_at(int dirfd, const char *name, const char *con) {
    char path[4096];
    fd_pathat(dirfd, name, path, sizeof(path));
    lsetfilecon(path, con);
}

void enable_selinux() {
    setcon = __setcon;
    getfilecon = __getfilecon;
    lgetfilecon = __lgetfilecon;
    fgetfilecon = __fgetfilecon;
    setfilecon = __setfilecon;
    lsetfilecon = __lsetfilecon;
    fsetfilecon = __fsetfilecon;
}
