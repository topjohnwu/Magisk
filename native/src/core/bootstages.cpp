#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>
#include <sys/statvfs.h>
#include <linux/input.h>
#include <libgen.h>
#include <vector>
#include <string>

#include <magisk.hpp>
#include <db.hpp>
#include <base.hpp>
#include <daemon.hpp>
#include <resetprop.hpp>
#include <selinux.hpp>

#include "core.hpp"

using namespace std;

// Boot stage state
enum : int {
    FLAG_NONE = 0,
    FLAG_POST_FS_DATA_DONE = (1 << 0),
    FLAG_LATE_START_DONE = (1 << 1),
    FLAG_BOOT_COMPLETE = (1 << 2),
    FLAG_SAFE_MODE = (1 << 3),
};

static int boot_state = FLAG_NONE;

bool zygisk_enabled = false;

/*********
 * Setup *
 *********/

static bool mount_mirror(const std::string_view from, const std::string_view to) {
    return !xmkdirs(to.data(), 0755) &&
           // recursively bind mount to mirror dir, rootfs will fail before 3.12 kernel
           // because of MS_NOUSER
           !mount(from.data(), to.data(), nullptr, MS_BIND | MS_REC, nullptr) &&
           // make mirror dir as a private mount so that it won't be affected by magic mount
           !xmount("", to.data(), nullptr, MS_PRIVATE | MS_REC, nullptr) &&
           // make this shared again so that we can track mounts
           !xmount("", to.data(), nullptr, MS_SHARED | MS_REC, nullptr);
}

static void mount_mirrors() {
    // first of all, try to setup module mount in the global namespace
    if (access(SECURE_DIR, F_OK) == 0 || (SDK_INT < 24 && xmkdir(SECURE_DIR, 0700))) {
        auto dest = MAGISKTMP + "/" MODULEMNT;
        if (mount_mirror(MODULEROOT, dest)) {
            // remount to clear nosuid flag
            struct statvfs st{};
            statvfs(dest.data(), &st);
            for (const auto &info: ParseMountInfo("self")) {
                if (info.target != dest) {
                    continue;
                }
                // strip rw, from fs options
                if (auto pos = info.fs_option.find_first_of(','); pos != string::npos) {
                    xmount("", dest.data(), nullptr,  MS_REMOUNT | (st.f_flag & ~MS_NOSUID), info.fs_option.data() + pos + 1);
                }
                break;
            }
            restorecon();
            chmod(SECURE_DIR, 0700);
        } else {
            PLOGE("mount modules\n");
        }
    }

    // fixup sepolicy.rules
    char buf1[PATH_MAX] = BLOCKBYNAMEDIR;
    char buf2[PATH_MAX] = {};
    auto rules = MAGISKTMP + "/" RULESDIR;
    if (auto len = readlink(rules.data(), buf1 + sizeof(BLOCKBYNAMEDIR) - 1,
                 sizeof(buf1) - sizeof(BLOCKBYNAMEDIR) + 1); len > 0 && str_ends(buf1, RULESENTRY)) {
        buf1[sizeof(BLOCKBYNAMEDIR) + len - sizeof(RULESENTRY)] = '\0';
        if (readlink(buf1, buf2, sizeof(buf2)) > 0) {
            LOGD("sepolicy device: %s\n", buf2);
            for (const auto &info: ParseMountInfo("self")) {
                if (auto len = readlink(info.source.data(), buf1, sizeof(buf1));
                    len > 0) {
                    buf1[len] = '\0';
                }
                if (info.source != buf2 && strcmp(buf1, buf2) != 0) {
                    continue;
                }
                auto target = info.target + RULESENTRY;
                LOGD("sepolicy.rules -> %s\n", target.data());
                unlink(rules.data());
                xsymlink(target.data(), rules.data());
                break;
            }
        }
    }

    // then unshare for private mount points
    xunshare(CLONE_NEWNS);

    // make magisktmp a slave so that the mirror won't be visible
    xmount("", MAGISKTMP.data(), nullptr, MS_SLAVE, nullptr);
    auto mirror_dir = MAGISKTMP + "/" MIRRDIR;
    // recursively bind mount / to mirror dir
    if (!mount_mirror("/", mirror_dir)) {
        LOGI("fallback to mount subtree\n");
        // rootfs may fail, fallback to bind mount each mount point
        std::vector<string> mounted_dirs {{ MAGISKTMP }};
        parse_mnt("/proc/1/mounts", [&](mntent *me) {
            if (me->mnt_fsname == "rootfs"sv)
                return true;

            for (const auto &dir : mounted_dirs) {
                if (str_starts(me->mnt_dir, dir)) {
                    // Already mounted
                    return true;
                }
            }

            if (mount_mirror(me->mnt_dir, mirror_dir + me->mnt_dir)) {
                mounted_dirs.emplace_back(me->mnt_dir);
            }
            return true;
        });
    }

    auto worker_dir = MAGISKTMP + "/" WORKERDIR;
    // mount the worker dir
    xmount("tmpfs", worker_dir.data(), "tmpfs", 0, "mode=755");
    // make the worker dir shared so that we can track mounts
    xmount("", worker_dir.data(), nullptr, MS_SHARED, nullptr);
}

static bool magisk_env() {
    char buf[4096];

    LOGI("* Initializing Magisk environment\n");

    preserve_stub_apk();
    string pkg;
    get_manager(0, &pkg);

    ssprintf(buf, sizeof(buf), "%s/0/%s/install", APP_DATA_DIR,
            pkg.empty() ? "xxx" /* Ensure non-exist path */ : pkg.data());

    // Alternative binaries paths
    const char *alt_bin[] = { "/cache/data_adb/magisk", "/data/magisk", buf };
    for (auto alt : alt_bin) {
        struct stat st{};
        if (lstat(alt, &st) == 0) {
            if (S_ISLNK(st.st_mode)) {
                unlink(alt);
                continue;
            }
            rm_rf(DATABIN);
            cp_afc(alt, DATABIN);
            rm_rf(alt);
            break;
        }
    }
    rm_rf("/cache/data_adb");

    // Directories in /data/adb
    xmkdir(DATABIN, 0755);
    xmkdir(MODULEROOT, 0755);
    xmkdir(SECURE_DIR "/post-fs-data.d", 0755);
    xmkdir(SECURE_DIR "/service.d", 0755);

    restore_databincon();

    if (access(DATABIN "/busybox", X_OK))
        return false;

    sprintf(buf, "%s/" BBPATH "/busybox", MAGISKTMP.data());
    mkdir(dirname(buf), 0755);
    cp_afc(DATABIN "/busybox", buf);
    exec_command_async(buf, "--install", "-s", dirname(buf));

    if (access(DATABIN "/magiskpolicy", X_OK) == 0) {
        sprintf(buf, "%s/magiskpolicy", MAGISKTMP.data());
        cp_afc(DATABIN "/magiskpolicy", buf);
    }

    return true;
}

void reboot() {
    if (RECOVERY_MODE)
        exec_command_sync("/system/bin/reboot", "recovery");
    else
        exec_command_sync("/system/bin/reboot");
}

static bool check_data() {
    bool mnt = false;
    file_readline("/proc/mounts", [&](string_view s) {
        if (str_contains(s, " /data ") && !str_contains(s, "tmpfs")) {
            mnt = true;
            return false;
        }
        return true;
    });
    if (!mnt)
        return false;
    auto crypto = getprop("ro.crypto.state");
    if (!crypto.empty()) {
        if (crypto != "encrypted") {
            // Unencrypted, we can directly access data
            return true;
        } else {
            // Encrypted, check whether vold is started
            return !getprop("init.svc.vold").empty();
        }
    }
    // ro.crypto.state is not set, assume it's unencrypted
    return true;
}

void unlock_blocks() {
    int fd, dev, OFF = 0;

    auto dir = xopen_dir("/dev/block");
    if (!dir)
        return;
    dev = dirfd(dir.get());

    for (dirent *entry; (entry = readdir(dir.get()));) {
        if (entry->d_type == DT_BLK) {
            if ((fd = openat(dev, entry->d_name, O_RDONLY | O_CLOEXEC)) < 0)
                continue;
            if (ioctl(fd, BLKROSET, &OFF) < 0)
                PLOGE("unlock %s", entry->d_name);
            close(fd);
        }
    }
}

#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))

static bool check_key_combo() {
    uint8_t bitmask[(KEY_MAX + 1) / 8];
    vector<int> events;
    constexpr char name[] = "/dev/.ev";

    // First collect candidate events that accepts volume down
    for (int minor = 64; minor < 96; ++minor) {
        if (xmknod(name, S_IFCHR | 0444, makedev(13, minor)))
            continue;
        int fd = open(name, O_RDONLY | O_CLOEXEC);
        unlink(name);
        if (fd < 0)
            continue;
        memset(bitmask, 0, sizeof(bitmask));
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bitmask)), bitmask);
        if (test_bit(KEY_VOLUMEDOWN, bitmask))
            events.push_back(fd);
        else
            close(fd);
    }
    if (events.empty())
        return false;

    run_finally fin([&]{ std::for_each(events.begin(), events.end(), close); });

    // Check if volume down key is held continuously for more than 3 seconds
    for (int i = 0; i < 300; ++i) {
        bool pressed = false;
        for (const int &fd : events) {
            memset(bitmask, 0, sizeof(bitmask));
            ioctl(fd, EVIOCGKEY(sizeof(bitmask)), bitmask);
            if (test_bit(KEY_VOLUMEDOWN, bitmask)) {
                pressed = true;
                break;
            }
        }
        if (!pressed)
            return false;
        // Check every 10ms
        usleep(10000);
    }
    LOGD("KEY_VOLUMEDOWN detected: enter safe mode\n");
    return true;
}

/***********************
 * Boot Stage Handlers *
 ***********************/

extern int disable_deny();

static void post_fs_data() {
    if (getenv("REMOUNT_ROOT"))
        xmount(nullptr, "/", nullptr, MS_REMOUNT | MS_RDONLY, nullptr);

    if (!check_data())
        return;

    setup_logfile(true);

    LOGI("** post-fs-data mode running\n");

    unlock_blocks();
    mount_mirrors();
    prune_su_access();

    if (access(SECURE_DIR, F_OK) != 0) {
        LOGE(SECURE_DIR " is not present, abort\n");
        goto early_abort;
    }

    if (!magisk_env()) {
        LOGE("* Magisk environment incomplete, abort\n");
        goto early_abort;
    }

    if (getprop("persist.sys.safemode", true) == "1" ||
        getprop("ro.sys.safemode") == "1" || check_key_combo()) {
        boot_state |= FLAG_SAFE_MODE;
        // Disable all modules and denylist so next boot will be clean
        disable_modules();
        disable_deny();
    } else {
        exec_common_scripts("post-fs-data");
        db_settings dbs;
        get_db_settings(dbs, ZYGISK_CONFIG);
        zygisk_enabled = dbs[ZYGISK_CONFIG];
        initialize_denylist();
        handle_modules();
    }

early_abort:
    // We still do magic mount because root itself might need it
    magic_mount();
    boot_state |= FLAG_POST_FS_DATA_DONE;
}

static void late_start() {
    setup_logfile(false);

    LOGI("** late_start service mode running\n");

    exec_common_scripts("service");
    exec_module_scripts("service");

    boot_state |= FLAG_LATE_START_DONE;
}

static void boot_complete() {
    boot_state |= FLAG_BOOT_COMPLETE;
    setup_logfile(false);

    LOGI("** boot-complete triggered\n");

    // At this point it's safe to create the folder
    if (access(SECURE_DIR, F_OK) != 0)
        xmkdir(SECURE_DIR, 0700);

    // Ensure manager exists
    check_pkg_refresh();
    get_manager(0, nullptr, true);
}

void boot_stage_handler(int code) {
    // Make sure boot stage execution is always serialized
    static pthread_mutex_t stage_lock = PTHREAD_MUTEX_INITIALIZER;
    mutex_guard lock(stage_lock);

    switch (code) {
    case MainRequest::POST_FS_DATA:
        if ((boot_state & FLAG_POST_FS_DATA_DONE) == 0)
            post_fs_data();
        close(xopen(UNBLOCKFILE, O_RDONLY | O_CREAT, 0));
        break;
    case MainRequest::LATE_START:
        if ((boot_state & FLAG_POST_FS_DATA_DONE) && (boot_state & FLAG_SAFE_MODE) == 0)
            late_start();
        break;
    case MainRequest::BOOT_COMPLETE:
        if ((boot_state & FLAG_SAFE_MODE) == 0)
            boot_complete();
        break;
    default:
        __builtin_unreachable();
    }
}
