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
static void setup_mounts() {
    LOGI("* Magic mount setup\n");
    auto self_mount_info = parse_mount_info("self");
    char path[PATH_MAX];

    // Bind remount module root to clear nosuid
    ssprintf(path, sizeof(path), "%s/" MODULEMNT, get_magisk_tmp());
    xmkdir(path, 0755);
    xmount(MODULEROOT, path, nullptr, MS_BIND, nullptr);
    xmount(nullptr, path, nullptr, MS_REMOUNT | MS_BIND | MS_RDONLY, nullptr);
    xmount(nullptr, path, nullptr, MS_PRIVATE, nullptr);

    // Check and mount preinit mirror
    char dev_path[64];
    ssprintf(dev_path, sizeof(dev_path), "%s/" PREINITDEV, get_magisk_tmp());
    if (struct stat st{}; stat(dev_path, &st) == 0 && S_ISBLK(st.st_mode)) {
        // DO NOT mount the block device directly, as we do not know the flags and configs
        // to properly mount the partition; mounting block devices directly as rw could cause
        // crashes if the filesystem driver is crap (e.g. some broken F2FS drivers).
        // What we do instead is to scan through the current mountinfo and find a pre-existing
        // mount point mounting our desired partition, and then bind mount the target folder.
        dev_t preinit_dev = st.st_rdev;
        bool mounted = false;
        ssprintf(path, sizeof(path), "%s/" PREINITMIRR, get_magisk_tmp());
        for (const auto &info: self_mount_info) {
            if (info.root == "/" && info.device == preinit_dev) {
                auto flags = split_view(info.fs_option, ",");
                auto rw = std::any_of(flags.begin(), flags.end(), [](const auto &flag) {
                    return flag == "rw"sv;
                });
                if (!rw) continue;
                string preinit_dir = resolve_preinit_dir(info.target.data());
                xmkdir(preinit_dir.data(), 0700);
                xmkdirs(path, 0755);
                mounted = xmount(preinit_dir.data(), path, nullptr, MS_BIND, nullptr) == 0;
                if (mounted) {
                    break;
                }
            }
        }
        if (!mounted) {
            LOGW("preinit mirror not mounted %u:%u\n", major(preinit_dev), minor(preinit_dev));
            unlink(dev_path);
        }
    }

    // Prepare worker
    ssprintf(path, sizeof(path), "%s/" WORKERDIR, get_magisk_tmp());
    xmkdir(path, 0);
    xmount(path, path, nullptr, MS_BIND, nullptr);
    xmount(nullptr, path, nullptr, MS_PRIVATE, nullptr);
}

string find_preinit_device() {
    enum part_t {
        UNKNOWN,
        PERSIST,
        METADATA,
        CACHE,
        DATA,
    };

    part_t ext4_type = UNKNOWN;
    part_t f2fs_type = UNKNOWN;

    bool encrypted = get_prop("ro.crypto.state") == "encrypted";
    bool mount = getuid() == 0 && getenv("MAGISKTMP");
    bool make_dev = mount && getenv("MAKEDEV");

    string preinit_source;
    string preinit_dir;
    dev_t preinit_dev;

    for (const auto &info: parse_mount_info("self")) {
        if (info.target.ends_with(PREINITMIRR))
            return basename(info.source.data());
        if (info.root != "/" || info.source[0] != '/' || info.source.find("/dm-") != string::npos)
            continue;
        // Skip all non ext4 partitions once we found a matching ext4 partition
        if (ext4_type != UNKNOWN && info.type != "ext4")
            continue;
        if (info.type != "ext4" && info.type != "f2fs")
            continue;
        auto flags = split_view(info.fs_option, ",");
        auto rw = std::any_of(flags.begin(), flags.end(), [](const auto &flag) {
            return flag == "rw"sv;
        });
        if (!rw) continue;
        if (auto base = std::string_view(info.source).substr(0, info.source.find_last_of('/'));
            !base.ends_with("/by-name") && !base.ends_with("/block")) {
            continue;
        }

        part_t &matched = (info.type == "f2fs") ? f2fs_type : ext4_type;
        switch (matched) {
            case UNKNOWN:
                if (info.target == "/persist" || info.target == "/mnt/vendor/persist") {
                    matched = PERSIST;
                    break;
                }
                [[fallthrough]];
            case PERSIST:
                if (info.target == "/metadata") {
                    matched = METADATA;
                    break;
                }
                [[fallthrough]];
            case METADATA:
                if (info.target == "/cache") {
                    matched = CACHE;
                    break;
                }
                [[fallthrough]];
            case CACHE:
                if (info.target == "/data") {
                    if (!encrypted || access("/data/unencrypted", F_OK) == 0) {
                        matched = DATA;
                        break;
                    }
                }
                [[fallthrough]];
            default:
                continue;
        }

        if (mount) {
            preinit_dir = resolve_preinit_dir(info.target.data());
            preinit_dev = info.device;
        }
        preinit_source = info.source;

        // Cannot find any better partition, stop finding
        if (ext4_type == DATA)
            break;
    }

    if (preinit_source.empty())
        return "";

    if (!preinit_dir.empty()) {
        auto mirror_dir = string(getenv("MAGISKTMP")) + "/" PREINITMIRR;
        mkdirs(preinit_dir.data(), 0700);
        mkdirs(mirror_dir.data(), 0700);
        xmount(preinit_dir.data(), mirror_dir.data(), nullptr, MS_BIND, nullptr);
        if (make_dev) {
            auto dev_path = string(getenv("MAGISKTMP")) + "/" PREINITDEV;
            xmknod(dev_path.data(), S_IFBLK | 0600, preinit_dev);
        }
    }
    return basename(preinit_source.data());
}

static bool magisk_env() {
    char buf[4096];

    LOGI("* Initializing Magisk environment\n");

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

/***********************
 * Boot Stage Handlers *
 ***********************/

static void disable_zygisk() {
    char sql[64];
    sprintf(sql, "REPLACE INTO settings (key,value) VALUES('%s',%d)",
            DB_SETTING_KEYS[ZYGISK_CONFIG], false);
    char *err = db_exec(sql);
    db_err(err);
}

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
            return safe_mode;
        }
    }

    if (!magisk_env()) {
        LOGE("* Magisk environment incomplete, abort\n");
        return safe_mode;
    }

    if (get_prop("persist.sys.safemode", true) == "1" ||
        get_prop("ro.sys.safemode") == "1" || check_key_combo()) {
        safe_mode = true;
        // Disable all modules and zygisk so next boot will be clean
        disable_modules();
        disable_zygisk();
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

    // At this point it's safe to create the folder
    if (access(SECURE_DIR, F_OK) != 0)
        xmkdir(SECURE_DIR, 0700);

    // Ensure manager exists
    check_pkg_refresh();
    get_manager(0, nullptr, true);

    reset_zygisk(true);
}
