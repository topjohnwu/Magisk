#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>
#include <linux/input.h>
#include <libgen.h>
#include <set>
#include <string>

#include <consts.hpp>
#include <db.hpp>
#include <base.hpp>
#include <core.hpp>
#include <selinux.hpp>

using namespace std;

bool zygisk_enabled = false;

/*********
 * Setup *
 *********/

static bool magisk_env() {
    char buf[4096];

    LOGI("* Initializing Magisk environment\n");

    ssprintf(buf, sizeof(buf), "%s/0/%s/install", APP_DATA_DIR, JAVA_PACKAGE_NAME);
    // Alternative binaries paths
    const char *alt_bin[] = { "/cache/data_adb/magisk", "/data/magisk", buf };
    for (auto alt : alt_bin) {
        if (access(alt, F_OK) == 0) {
            rm_rf(DATABIN);
            cp_afc(alt, DATABIN);
            rm_rf(alt);
        }
    }
    rm_rf("/cache/data_adb");

    // Directories in /data/adb
    chmod(SECURE_DIR, 0700);
    xmkdir(DATABIN, 0755);
    xmkdir(MODULEROOT, 0755);
    xmkdir(SECURE_DIR "/post-fs-data.d", 0755);
    xmkdir(SECURE_DIR "/service.d", 0755);
    restorecon();

    if (access(DATABIN "/busybox", X_OK))
        return false;

    ssprintf(buf, sizeof(buf), "%s/" BBPATH "/busybox", get_magisk_tmp());
    mkdir(dirname(buf), 0755);
    cp_afc(DATABIN "/busybox", buf);
    exec_command_async(buf, "--install", "-s", dirname(buf));

    // magisk32 and magiskpolicy are not installed into ramdisk and has to be copied
    // from data to magisk tmp
    if (access(DATABIN "/magisk32", X_OK) == 0) {
        ssprintf(buf, sizeof(buf), "%s/magisk32", get_magisk_tmp());
        cp_afc(DATABIN "/magisk32", buf);
    }
    if (access(DATABIN "/magiskpolicy", X_OK) == 0) {
        ssprintf(buf, sizeof(buf), "%s/magiskpolicy", get_magisk_tmp());
        cp_afc(DATABIN "/magiskpolicy", buf);
    }

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

static bool check_safe_mode() {
    int bootloop_cnt;
    db_settings dbs;
    get_db_settings(dbs, BOOTLOOP_COUNT);
    bootloop_cnt = dbs[BOOTLOOP_COUNT];
    // Increment the bootloop counter
    set_db_settings(BOOTLOOP_COUNT, bootloop_cnt + 1);
    return bootloop_cnt >= 2 || get_prop("persist.sys.safemode", true) == "1" ||
           get_prop("ro.sys.safemode") == "1" || check_key_combo();
}

/***********************
 * Boot Stage Handlers *
 ***********************/

bool MagiskD::post_fs_data() const {
    as_rust().setup_logfile();

    LOGI("** post-fs-data mode running\n");

    preserve_stub_apk();
    prune_su_access();

    bool safe_mode = false;

    if (access(SECURE_DIR, F_OK) != 0) {
        if (SDK_INT < 24) {
            xmkdir(SECURE_DIR, 0700);
        } else {
            LOGE(SECURE_DIR " is not present, abort\n");
            safe_mode = true;
            return safe_mode;
        }
    }

    if (!magisk_env()) {
        LOGE("* Magisk environment incomplete, abort\n");
        safe_mode = true;
        return safe_mode;
    }

    if (check_safe_mode()) {
        LOGI("* Safe mode triggered\n");
        safe_mode = true;
        // Disable all modules and zygisk so next boot will be clean
        disable_modules();
        set_db_settings(ZYGISK_CONFIG, false);
        return safe_mode;
    }

    exec_common_scripts("post-fs-data");
    db_settings dbs;
    get_db_settings(dbs, ZYGISK_CONFIG);
    zygisk_enabled = dbs[ZYGISK_CONFIG];
    initialize_denylist();
    setup_mounts();
    handle_modules();
    load_modules();
    return safe_mode;
}

void MagiskD::late_start() const {
    as_rust().setup_logfile();

    LOGI("** late_start service mode running\n");

    exec_common_scripts("service");
    exec_module_scripts("service");
}

void MagiskD::boot_complete() const {
    as_rust().setup_logfile();

    LOGI("** boot-complete triggered\n");

    // Reset the bootloop counter once we have boot-complete
    set_db_settings(BOOTLOOP_COUNT, 0);

    // At this point it's safe to create the folder
    if (access(SECURE_DIR, F_OK) != 0)
        xmkdir(SECURE_DIR, 0700);

    // Ensure manager exists
    get_manager(0, nullptr, true);

    reset_zygisk(true);
}
