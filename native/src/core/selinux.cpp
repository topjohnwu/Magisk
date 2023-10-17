#include <unistd.h>
#include <sys/syscall.h>
#include <sys/xattr.h>

#include <magisk.hpp>
#include <base.hpp>
#include <selinux.hpp>
#include <flags.h>

using namespace std;

void freecon(char *s) {
    free(s);
}

int setcon(const char *ctx) {
    int fd = open("/proc/self/attr/current", O_WRONLY | O_CLOEXEC);
    if (fd < 0)
        return fd;
    size_t len = strlen(ctx) + 1;
    int rc = write(fd, ctx, len);
    close(fd);
    return rc != len;
}

int getfilecon(const char *path, char **ctx) {
    char buf[1024];
    int rc = syscall(__NR_getxattr, path, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
    if (rc >= 0)
        *ctx = strdup(buf);
    return rc;
}

int lgetfilecon(const char *path, char **ctx) {
    char buf[1024];
    int rc = syscall(__NR_lgetxattr, path, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
    if (rc >= 0)
        *ctx = strdup(buf);
    return rc;
}

int fgetfilecon(int fd, char **ctx) {
    char buf[1024];
    int rc = syscall(__NR_fgetxattr, fd, XATTR_NAME_SELINUX, buf, sizeof(buf) - 1);
    if (rc >= 0)
        *ctx = strdup(buf);
    return rc;
}

int setfilecon(const char *path, const char *ctx) {
    return syscall(__NR_setxattr, path, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
}

int lsetfilecon(const char *path, const char *ctx) {
    return syscall(__NR_lsetxattr, path, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
}

int fsetfilecon(int fd, const char *ctx) {
    return syscall(__NR_fsetxattr, fd, XATTR_NAME_SELINUX, ctx, strlen(ctx) + 1, 0);
}

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

#define UNLABEL_CON "u:object_r:unlabeled:s0"
#define SYSTEM_CON  "u:object_r:system_file:s0"
#define ADB_CON     "u:object_r:adb_data_file:s0"
#define ROOT_CON    "u:object_r:rootfs:s0"

static void restore_syscon_from_null(int dirfd) {
    struct dirent *entry;
    char *con;

    if (fgetfilecon(dirfd, &con) >= 0) {
        if (strlen(con) == 0 || strcmp(con, UNLABEL_CON) == 0)
            fsetfilecon(dirfd, SYSTEM_CON);
        freecon(con);
    }

    auto dir = xopen_dir(dirfd);
    while ((entry = xreaddir(dir.get()))) {
        int fd = openat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
        if (entry->d_type == DT_DIR) {
            restore_syscon_from_null(fd);
            continue;
        } else if (entry->d_type == DT_REG) {
            if (fgetfilecon(fd, &con) >= 0) {
                if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
                    fsetfilecon(fd, SYSTEM_CON);
                freecon(con);
            }
        } else if (entry->d_type == DT_LNK) {
            getfilecon_at(dirfd, entry->d_name, &con);
            if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
                setfilecon_at(dirfd, entry->d_name, con);
            freecon(con);
        }
        close(fd);
    }
}

static void restore_syscon(int dirfd) {
    struct dirent *entry;

    fsetfilecon(dirfd, SYSTEM_CON);
    fchown(dirfd, 0, 0);

    auto dir = xopen_dir(dirfd);
    while ((entry = xreaddir(dir.get()))) {
        int fd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
        if (entry->d_type == DT_DIR) {
            restore_syscon(fd);
            continue;
        } else if (entry->d_type) {
            fsetfilecon(fd, SYSTEM_CON);
            fchown(fd, 0, 0);
        }
        close(fd);
    }
}

void restorecon() {
    int fd = xopen("/sys/fs/selinux/context", O_WRONLY | O_CLOEXEC);
    if (write(fd, ADB_CON, sizeof(ADB_CON)) >= 0)
        lsetfilecon(SECURE_DIR, ADB_CON);
    close(fd);
    lsetfilecon(MODULEROOT, SYSTEM_CON);
    restore_syscon_from_null(xopen(MODULEROOT, O_RDONLY | O_CLOEXEC));
    restore_syscon(xopen(DATABIN, O_RDONLY | O_CLOEXEC));
}

void restore_tmpcon() {
    if (MAGISKTMP == "/sbin")
        setfilecon(MAGISKTMP.data(), ROOT_CON);
    else
        chmod(MAGISKTMP.data(), 0711);

    auto dir = xopen_dir(MAGISKTMP.data());
    int dfd = dirfd(dir.get());

    for (dirent *entry; (entry = xreaddir(dir.get()));)
        setfilecon_at(dfd, entry->d_name, SYSTEM_CON);
}
