#include <unistd.h>
#include <sys/syscall.h>
#include <sys/xattr.h>

#include <consts.hpp>
#include <base.hpp>
#include <selinux.hpp>
#include <core.hpp>
#include <flags.h>

using namespace std;

int setcon(const char *con) {
    int fd = open("/proc/self/attr/current", O_WRONLY | O_CLOEXEC);
    if (fd < 0)
        return fd;
    size_t len = strlen(con) + 1;
    int rc = write(fd, con, len);
    close(fd);
    return rc != len;
}

int getfilecon(const char *path, byte_data con) {
    return syscall(__NR_getxattr, path, XATTR_NAME_SELINUX, con.buf(), con.sz());
}

int lgetfilecon(const char *path, byte_data con) {
    return syscall(__NR_lgetxattr, path, XATTR_NAME_SELINUX, con.buf(), con.sz());
}

int fgetfilecon(int fd, byte_data con) {
    return syscall(__NR_fgetxattr, fd, XATTR_NAME_SELINUX, con.buf(), con.sz());
}

int setfilecon(const char *path, const char *con) {
    return syscall(__NR_setxattr, path, XATTR_NAME_SELINUX, con, strlen(con) + 1, 0);
}

int lsetfilecon(const char *path, const char *con) {
    return syscall(__NR_lsetxattr, path, XATTR_NAME_SELINUX, con, strlen(con) + 1, 0);
}

int fsetfilecon(int fd, const char *con) {
    return syscall(__NR_fsetxattr, fd, XATTR_NAME_SELINUX, con, strlen(con) + 1, 0);
}

int getfilecon_at(int dirfd, const char *name, byte_data con) {
    char path[4096];
    fd_pathat(dirfd, name, path, sizeof(path));
    return lgetfilecon(path, con);
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
    char con[1024];

    if (fgetfilecon(dirfd, { con, sizeof(con) }) >= 0) {
        if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
            fsetfilecon(dirfd, SYSTEM_CON);
    }

    auto dir = xopen_dir(dirfd);
    while ((entry = xreaddir(dir.get()))) {
        int fd = openat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
        if (entry->d_type == DT_DIR) {
            restore_syscon_from_null(fd);
            continue;
        } else if (entry->d_type == DT_REG) {
            if (fgetfilecon(fd, { con, sizeof(con) }) >= 0) {
                if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
                    fsetfilecon(fd, SYSTEM_CON);
            }
        } else if (entry->d_type == DT_LNK) {
            if (getfilecon_at(dirfd, entry->d_name, { con, sizeof(con) }) >= 0) {
                if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
                    setfilecon_at(dirfd, entry->d_name, SYSTEM_CON);
            }
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
    const char *tmp = get_magisk_tmp();
    if (tmp == "/sbin"sv)
        setfilecon(tmp, ROOT_CON);
    else
        chmod(tmp, 0711);

    auto dir = xopen_dir(tmp);
    int dfd = dirfd(dir.get());

    for (dirent *entry; (entry = xreaddir(dir.get()));)
        setfilecon_at(dfd, entry->d_name, SYSTEM_CON);

    string logd = tmp + "/"s LOG_PIPE;
    setfilecon(logd.data(), MAGISK_LOG_CON);
}
