#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>
#include <linux/input.h>
#include <libgen.h>
#include <vector>
#include <string>

#include <magisk.hpp>
#include <db.hpp>
#include <utils.hpp>
#include <daemon.hpp>
#include <resetprop.hpp>
#include <selinux.hpp>

#include "core.hpp"

using namespace std;

static bool safe_mode = false;
bool zygisk_enabled = false;

/*********
 * Setup *
 *********/

#define MNT_DIR_IS(dir) (me->mnt_dir == string_view(dir))
#define MNT_TYPE_IS(type) (me->mnt_type == string_view(type))
#define SETMIR(b, part) snprintf(b, sizeof(b), "%s/" MIRRDIR "/" #part, MAGISKTMP.data())
#define SETBLK(b, part) snprintf(b, sizeof(b), "%s/" BLOCKDIR "/" #part, MAGISKTMP.data())

#define do_mount_mirror(part) {     \
    SETMIR(buf1, part);             \
    SETBLK(buf2, part);             \
    unlink(buf2);                   \
    mknod(buf2, S_IFBLK | 0600, st.st_dev); \
    xmkdir(buf1, 0755);             \
    int flags = 0;                  \
    auto opts = split_ro(me->mnt_opts, ",");\
    for (string_view s : opts) {    \
        if (s == "ro") {            \
            flags |= MS_RDONLY;     \
            break;                  \
        }                           \
    }                               \
    xmount(buf2, buf1, me->mnt_type, flags, nullptr); \
    LOGI("mount: %s\n", buf1);      \
}

#define mount_mirror(part) \
if (MNT_DIR_IS("/" #part)  \
    && !MNT_TYPE_IS("tmpfs") \
    && !MNT_TYPE_IS("overlay") \
    && lstat(me->mnt_dir, &st) == 0) { \
    do_mount_mirror(part); \
    break;                 \
}

#define link_mirror(part) \
SETMIR(buf1, part); \
if (access("/system/" #part, F_OK) == 0 && access(buf1, F_OK) != 0) { \
    xsymlink("./system/" #part, buf1); \
    LOGI("link: %s\n", buf1); \
}

#define link_orig_dir(dir, part) \
if (MNT_DIR_IS(dir) && !MNT_TYPE_IS("tmpfs") && !MNT_TYPE_IS("overlay")) { \
    SETMIR(buf1, part);          \
    rmdir(buf1);                 \
    xsymlink(dir, buf1);         \
    LOGI("link: %s\n", buf1);    \
    break;                       \
}

#define link_orig(part) link_orig_dir("/" #part, part)

static void mount_mirrors() {
    char buf1[4096];
    char buf2[4096];

    LOGI("* Mounting mirrors\n");

    parse_mnt("/proc/mounts", [&](mntent *me) {
        struct stat st{};
        do {
            mount_mirror(system)
            mount_mirror(vendor)
            mount_mirror(product)
            mount_mirror(system_ext)
            mount_mirror(data)
            link_orig(cache)
            link_orig(metadata)
            link_orig(persist)
            link_orig_dir("/mnt/vendor/persist", persist)
            if (SDK_INT >= 24 && MNT_DIR_IS("/proc") && !strstr(me->mnt_opts, "hidepid=2")) {
                xmount(nullptr, "/proc", nullptr, MS_REMOUNT, "hidepid=2,gid=3009");
                break;
            }
        } while (false);
        return true;
    });
    SETMIR(buf1, system);
    if (access(buf1, F_OK) != 0) {
        xsymlink("./system_root/system", buf1);
        LOGI("link: %s\n", buf1);
        parse_mnt("/proc/mounts", [&](mntent *me) {
            struct stat st;
            if (MNT_DIR_IS("/") && me->mnt_type != "rootfs"sv && stat("/", &st) == 0) {
                do_mount_mirror(system_root)
                return false;
            }
            return true;
        });
    }
    link_mirror(vendor)
    link_mirror(product)
    link_mirror(system_ext)
}

static bool magisk_env() {
    char buf[4096];

    LOGI("* Initializing Magisk environment\n");

    string pkg;
    get_manager(&pkg);

    sprintf(buf, "%s/0/%s/install", APP_DATA_DIR,
            pkg.empty() ? "xxx" /* Ensure non-exist path */ : pkg.data());

    // Alternative binaries paths
    const char *alt_bin[] = { "/cache/data_adb/magisk", "/data/magisk", buf };
    for (auto alt : alt_bin) {
        struct stat st;
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

    // Remove stuffs
    rm_rf("/cache/data_adb");
    rm_rf("/data/adb/modules/.core");
    unlink("/data/adb/magisk.img");
    unlink("/data/adb/magisk_merge.img");
    unlink("/data/magisk.img");
    unlink("/data/magisk_merge.img");
    unlink("/data/magisk_debug.log");

    // Directories in /data/adb
    xmkdir(DATABIN, 0755);
    xmkdir(MODULEROOT, 0755);
    xmkdir(SECURE_DIR "/post-fs-data.d", 0755);
    xmkdir(SECURE_DIR "/service.d", 0755);

    if (access(DATABIN "/busybox", X_OK))
        return false;

    sprintf(buf, "%s/" BBPATH "/busybox", MAGISKTMP.data());
    mkdir(dirname(buf), 0755);
    cp_afc(DATABIN "/busybox", buf);
    exec_command_async(buf, "--install", "-s", dirname(buf));

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
        if (crypto == "unencrypted") {
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

static pthread_mutex_t stage_lock = PTHREAD_MUTEX_INITIALIZER;
extern int disable_deny();

void post_fs_data(int client) {
    // ack
    write_int(client, 0);
    close(client);

    mutex_guard lock(stage_lock);

    if (getenv("REMOUNT_ROOT"))
        xmount(nullptr, "/", nullptr, MS_REMOUNT | MS_RDONLY, nullptr);

    if (!check_data())
        goto unblock_init;

    DAEMON_STATE = STATE_POST_FS_DATA;
    setup_logfile(true);

    LOGI("** post-fs-data mode running\n");

    unlock_blocks();
    mount_mirrors();

    if (access(SECURE_DIR, F_OK) != 0) {
        if (SDK_INT < 24) {
            // There is no FBE pre 7.0, we can directly create the folder without issues
            xmkdir(SECURE_DIR, 0700);
        } else {
            // If the folder is not automatically created by Android,
            // do NOT proceed further. Manual creation of the folder
            // will have no encryption flag, which will cause bootloops on FBE devices.
            LOGE(SECURE_DIR " is not present, abort\n");
            goto early_abort;
        }
    }

    if (!magisk_env()) {
        LOGE("* Magisk environment incomplete, abort\n");
        goto early_abort;
    }

    if (getprop("persist.sys.safemode", true) == "1" || check_key_combo()) {
        safe_mode = true;
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
    DAEMON_STATE = STATE_POST_FS_DATA_DONE;

unblock_init:
    close(xopen(UNBLOCKFILE, O_RDONLY | O_CREAT, 0));
}

void late_start(int client) {
    // ack
    write_int(client, 0);
    close(client);

    mutex_guard lock(stage_lock);
    run_finally fin([]{ DAEMON_STATE = STATE_LATE_START_DONE; });
    setup_logfile(false);

    LOGI("** late_start service mode running\n");

    if (DAEMON_STATE < STATE_POST_FS_DATA_DONE || safe_mode)
        return;

    exec_common_scripts("service");
    exec_module_scripts("service");
}

void boot_complete(int client) {
    // ack
    write_int(client, 0);
    close(client);

    mutex_guard lock(stage_lock);
    DAEMON_STATE = STATE_BOOT_COMPLETE;
    setup_logfile(false);

    LOGI("** boot_complete triggered\n");

    if (safe_mode)
        return;

    // At this point it's safe to create the folder
    if (access(SECURE_DIR, F_OK) != 0)
        xmkdir(SECURE_DIR, 0700);

    if (!get_manager()) {
        if (access(MANAGERAPK, F_OK) == 0) {
            // Only try to install APK when no manager is installed
            // Magisk Manager should be upgraded by itself, not through recovery installs
            rename(MANAGERAPK, "/data/magisk.apk");
            install_apk("/data/magisk.apk");
        } else {
            // Install stub
            auto init = MAGISKTMP + "/magiskinit";
            exec_command_sync(init.data(), "-x", "manager", "/data/magisk.apk");
            install_apk("/data/magisk.apk");
        }
    }
    unlink(MANAGERAPK);
}
