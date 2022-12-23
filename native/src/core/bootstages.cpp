#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>
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
#include <mntinfo.hpp>

#include "core.hpp"

using namespace std;

static bool safe_mode = false;
bool zygisk_enabled = false;

/*********
 * Setup *
 *********/

static bool mount_correct(const std::string_view dev, const std::string_view dir, const std::string_view root, const std::string_view type, int flags = 0){
    int ret = xmount(dev.data(), dir.data(), type.data(), flags, nullptr);
    if (ret) return false;
    if (root != "/"sv){
        string mnt_dir = string(dir) + string(root);
        if (xmount(mnt_dir.data(), dir.data(), nullptr, MS_BIND, nullptr)){
       	    umount2(dir.data(), MNT_DETACH);
       	    return false;
        }
    }
    LOGD("mount: %s\n", dir.data());
    return true;
}

static bool mount_recreate(const std::string_view from, const std::string_view to){
    return !xmount(from.data(), to.data(), nullptr, MS_BIND, nullptr) &&
           !xmount("", to.data(), nullptr, MS_PRIVATE, nullptr);
}

static void mount_mirrors() {
	LOGI("* Mount mirrors\n");

    string mirror_dir = MAGISKTMP + "/" MIRRDIR;
    string block_dir = MAGISKTMP + "/" BLOCKDIR;
    string system_mirror = mirror_dir + "/system";
    
    std::vector<string> mounted_dirs;

    auto mount_info = ParseMountInfo("1");

    for (auto &info : mount_info) {
        const char *mnt_dir = info.target.data();
        const char *mnt_type = info.type.data();
        const char *mnt_opts = info.fs_option.data();
        const char *mnt_root = info.root.data();
        int matched = 0;
        struct stat st{};
            if (lstat(mnt_dir, &st) != 0)
                continue;
            // skip mount subtree of /data to avoid problem
            if (string(mnt_dir).starts_with("/data/"))
                continue;
            for (const char *part : { MIRRORS }) {
                if (mnt_dir == string_view(part))
				    matched = 1;
            }
            for (const auto &dir : mounted_dirs) {
                if (info.target.starts_with(dir + "/")){
                    string dest = mirror_dir + mnt_dir;
                    if (mount_recreate(mnt_dir, dest))
                        LOGD("mount: %s\n", dest.data());
                    continue;
                }
                if (string_view(mnt_dir) == dir) {
                    // Already mounted
                    continue;
                }
            }
            if (mnt_type == "tmpfs"sv || mnt_type == "overlay"sv || mnt_type == "rootfs"sv){
           	    if (matched == 1){
  	    	        string dest = mirror_dir + mnt_dir;
                    xmkdir(dest.data(), 0755);
                    // if root partition like /system is overlay
                    if (mount_recreate(mnt_dir, dest))
                        LOGD("mount: %s\n", dest.data());
           	    }
                continue;
            }
            int flags = 0;
            auto opts = split_ro(mnt_opts, ",");
            for (string_view s : opts) {
                if (s == "ro") {
                    flags |= MS_RDONLY;
                    break;
                }
            }
            if (mnt_dir == "/"sv) {
                string src = block_dir + "/system_root";
                string dest = mirror_dir + "/system_root";
                mknod(src.data(), S_IFBLK | 0600, st.st_dev);
                xmkdir(dest.data(), 0755);
                symlink("./system_root/system", system_mirror.data());
                if (mount_correct(src.data(), dest.data(), mnt_root, mnt_type, flags)){
                    mounted_dirs.emplace_back("/system");
                    mounted_dirs.emplace_back("/");
                }
                continue;
            }
            if (matched == 1){
                    string src = block_dir + mnt_dir;
                    string dest = mirror_dir + mnt_dir;
                    mknod(src.data(), S_IFBLK | 0600, st.st_dev);
                    xmkdir(dest.data(), 0755);
                    if (mount_correct(src.data(), dest.data(), mnt_root, mnt_type, flags)){
                        goto add_mounted_dir;
                    }
                    continue;
            }
            continue;

        add_mounted_dir:
            mounted_dirs.emplace_back(mnt_dir);
    }

    for (const char *part : { SPEC_PARTS }){
        string dest = mirror_dir + part;
        string src = string("./system") + part;
        if (access(dest.data(), F_OK) != 0 && access(part, F_OK) == 0) {
            symlink(src.data(), dest.data());
        }
    }
    for (const char *part : { SE_MIRRORS }){
        string dest = mirror_dir + part;
        string src = part;
        if (access(part, F_OK) == 0) {
            symlink(src.data(), dest.data());
        }
    }
    if (access("/persist", F_OK) != 0 && access("/mnt/vendor/persist", F_OK) == 0){
        string dest = mirror_dir + "/persist";
   	    symlink("/mnt/vendor/persist", dest.data());
    }
    if (access(system_mirror.data(), F_OK) != 0) {
        symlink("./system_root/system", system_mirror.data());
    }
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

static pthread_mutex_t stage_lock = PTHREAD_MUTEX_INITIALIZER;
extern int disable_deny();

void post_fs_data(int client) {
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
    prune_su_access();

    if (access(SECURE_DIR, F_OK) != 0) {
        LOGE(SECURE_DIR " is not present, abort\n");
        goto early_abort;
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
    close(client);

    mutex_guard lock(stage_lock);
    DAEMON_STATE = STATE_BOOT_COMPLETE;
    setup_logfile(false);

    LOGI("** boot-complete triggered\n");

    if (safe_mode)
        return;

    // At this point it's safe to create the folder
    if (access(SECURE_DIR, F_OK) != 0)
        xmkdir(SECURE_DIR, 0700);

    // Ensure manager exists
    check_pkg_refresh();
    get_manager(0, nullptr, true);
}

void zygote_restart(int client) {
    close(client);

    LOGI("** zygote restarted\n");
    pkg_xml_ino = 0;
    prune_su_access();
}
