#include <string_view>

#include <magisk.hpp>
#include <selinux.hpp>
#include <base.hpp>

using namespace std;

#define UNLABEL_CON "u:object_r:unlabeled:s0"
#define SYSTEM_CON  "u:object_r:system_file:s0"
#define ADB_CON     "u:object_r:adb_data_file:s0"
#define ROOT_CON    "u:object_r:rootfs:s0"
#define EXEC_CON    "u:object_r:" SEPOL_EXEC_TYPE ":s0"

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
    if (!selinux_enabled())
        return;
    int fd = xopen(SELINUX_CONTEXT, O_WRONLY | O_CLOEXEC);
    if (write(fd, ADB_CON, sizeof(ADB_CON)) >= 0)
        lsetfilecon(SECURE_DIR, ADB_CON);
    close(fd);
    lsetfilecon(MODULEROOT, SYSTEM_CON);
    restore_syscon_from_null(xopen(MODULEROOT, O_RDONLY | O_CLOEXEC));
    restore_syscon(xopen(DATABIN, O_RDONLY | O_CLOEXEC));
}

void restore_tmpcon() {
    if (!selinux_enabled())
        return;
    if (MAGISKTMP == "/sbin")
        setfilecon(MAGISKTMP.data(), ROOT_CON);
    else
        chmod(MAGISKTMP.data(), 0700);

    auto dir = xopen_dir(MAGISKTMP.data());
    int dfd = dirfd(dir.get());

    for (dirent *entry; (entry = xreaddir(dir.get()));)
        setfilecon_at(dfd, entry->d_name, SYSTEM_CON);

    if (SDK_INT >= 26) {
        string magisk = MAGISKTMP + "/magisk";
        setfilecon(magisk.data(), EXEC_CON);
    }
}
