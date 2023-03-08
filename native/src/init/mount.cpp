#include <set>
#include <sys/mount.h>
#include <sys/sysmacros.h>
#include <libgen.h>

#include <base.hpp>
#include <selinux.hpp>
#include <magisk.hpp>

#include "init.hpp"

using namespace std;

struct devinfo {
    int major;
    int minor;
    char devname[32];
    char partname[32];
    char dmname[32];
};

static vector<devinfo> dev_list;

static void parse_device(devinfo *dev, const char *uevent) {
    dev->partname[0] = '\0';
    parse_prop_file(uevent, [=](string_view key, string_view value) -> bool {
        if (key == "MAJOR")
            dev->major = parse_int(value.data());
        else if (key == "MINOR")
            dev->minor = parse_int(value.data());
        else if (key == "DEVNAME")
            strcpy(dev->devname, value.data());
        else if (key == "PARTNAME")
            strcpy(dev->partname, value.data());

        return true;
    });
}

static void collect_devices() {
    char path[128];
    devinfo dev{};
    if (auto dir = xopen_dir("/sys/dev/block"); dir) {
        for (dirent *entry; (entry = readdir(dir.get()));) {
            if (entry->d_name == "."sv || entry->d_name == ".."sv)
                continue;
            sprintf(path, "/sys/dev/block/%s/uevent", entry->d_name);
            parse_device(&dev, path);
            sprintf(path, "/sys/dev/block/%s/dm/name", entry->d_name);
            if (access(path, F_OK) == 0) {
                auto name = rtrim(full_read(path));
                strcpy(dev.dmname, name.data());
            }
            dev_list.push_back(dev);
        }
    }
}

static struct {
    char partname[32];
    char block_dev[64];
} blk_info;

static int64_t setup_block() {
    if (dev_list.empty())
        collect_devices();

    for (int tries = 0; tries < 3; ++tries) {
        for (auto &dev : dev_list) {
            if (strcasecmp(dev.partname, blk_info.partname) == 0)
                LOGD("Setup %s: [%s] (%d, %d)\n", dev.partname, dev.devname, dev.major, dev.minor);
            else if (strcasecmp(dev.dmname, blk_info.partname) == 0)
                LOGD("Setup %s: [%s] (%d, %d)\n", dev.dmname, dev.devname, dev.major, dev.minor);
            else
                continue;

            dev_t rdev = makedev(dev.major, dev.minor);
            xmknod(blk_info.block_dev, S_IFBLK | 0600, rdev);
            return rdev;
        }
        // Wait 10ms and try again
        usleep(10000);
        dev_list.clear();
        collect_devices();
    }

    // The requested partname does not exist
    return -1;
}

static void switch_root(const string &path) {
    LOGD("Switch root to %s\n", path.data());
    int root = xopen("/", O_RDONLY);
    for (set<string, greater<>> mounts; auto &info : parse_mount_info("self")) {
        if (info.target == "/" || info.target == path)
            continue;
        if (auto last_mount = mounts.upper_bound(info.target);
                last_mount != mounts.end() && info.target.starts_with(*last_mount + '/')) {
            continue;
        }
        mounts.emplace(info.target);
        auto new_path = path + info.target;
        xmkdir(new_path.data(), 0755);
        xmount(info.target.data(), new_path.data(), nullptr, MS_MOVE, nullptr);
    }
    chdir(path.data());
    xmount(path.data(), "/", nullptr, MS_MOVE, nullptr);
    chroot(".");

    LOGD("Cleaning rootfs\n");
    frm_rf(root);
}

static void mount_rules_dir(string path, dev_t rules_dev) {
    if (!rules_dev) return;
    xmknod(BLOCKDIR "/rules", S_IFBLK | 0600, rules_dev);
    xmkdir(MIRRDIR "/rules", 0);

    bool mounted = false;
    // first of all, find if rules dev is already mounted
    for (auto &info : parse_mount_info("self")) {
        if (info.root == "/" && info.device == rules_dev) {
            // Already mounted, just bind mount
            xmount(info.target.data(), MIRRDIR "/rules", nullptr, MS_BIND, nullptr);
            mounted = true;
            break;
        }
    }

    if (mounted || mount(BLOCKDIR "/rules", MIRRDIR "/rules", "ext4", MS_RDONLY, nullptr) == 0 ||
        mount(BLOCKDIR "/rules", MIRRDIR "/rules", "f2fs", MS_RDONLY, nullptr) == 0) {
        string custom_rules_dir = find_rules_dir(MIRRDIR "/rules");
        // Create bind mount
        xmkdirs(RULESDIR, 0);
        if (access(custom_rules_dir.data(), F_OK)) {
            LOGW("empty sepolicy.rules: %s\n", custom_rules_dir.data());
        } else {
            LOGD("sepolicy.rules: %s\n", custom_rules_dir.data());
            xmount(custom_rules_dir.data(), RULESDIR, nullptr, MS_BIND, nullptr);
            mount_list.emplace_back(path += "/" RULESDIR);
        }
        xumount2(MIRRDIR "/rules", MNT_DETACH);
    } else {
        PLOGE("Failed to mount sepolicy.rules %u:%u", major(rules_dev), minor(rules_dev));
        unlink(BLOCKDIR "/rules");
    }
}

bool LegacySARInit::mount_system_root() {
    LOGD("Mounting system_root\n");

    // there's no /dev in stub cpio
    xmkdir("/dev", 0777);

    strcpy(blk_info.block_dev, "/dev/root");

    do {
        // Try legacy SAR dm-verity
        strcpy(blk_info.partname, "vroot");
        auto dev = setup_block();
        if (dev >= 0)
            goto mount_root;

        // Try NVIDIA naming scheme
        strcpy(blk_info.partname, "APP");
        dev = setup_block();
        if (dev >= 0)
            goto mount_root;

        sprintf(blk_info.partname, "system%s", config->slot);
        dev = setup_block();
        if (dev >= 0)
            goto mount_root;

        // Poll forever if rootwait was given in cmdline
    } while (config->rootwait);

    // We don't really know what to do at this point...
    LOGE("Cannot find root partition, abort\n");
    exit(1);

mount_root:
    xmkdir("/system_root", 0755);

    if (xmount("/dev/root", "/system_root", "ext4", MS_RDONLY, nullptr)) {
        if (xmount("/dev/root", "/system_root", "erofs", MS_RDONLY, nullptr)) {
            // We don't really know what to do at this point...
            LOGE("Cannot mount root partition, abort\n");
            exit(1);
        }
    }

    switch_root("/system_root");

    // Make dev writable
    xmount("tmpfs", "/dev", "tmpfs", 0, "mode=755");
    mount_list.emplace_back("/dev");

    // Use the apex folder to determine whether 2SI (Android 10+)
    bool is_two_stage = access("/apex", F_OK) == 0;
    LOGD("is_two_stage: [%d]\n", is_two_stage);

#if ENABLE_AVD_HACK
    if (!is_two_stage) {
        if (config->emulator) {
            avd_hack = true;
            // These values are hardcoded for API 28 AVD
            xmkdir("/dev/block", 0755);
            strcpy(blk_info.block_dev, "/dev/block/vde1");
            strcpy(blk_info.partname, "vendor");
            setup_block();
            xmount(blk_info.block_dev, "/vendor", "ext4", MS_RDONLY, nullptr);
        }
    }
#endif

    return is_two_stage;
}

void BaseInit::exec_init() {
    // Unmount in reverse order
    for (auto &p : reversed(mount_list)) {
        if (xumount2(p.data(), MNT_DETACH) == 0)
            LOGD("Unmount [%s]\n", p.data());
    }
    execv("/init", argv);
    exit(1);
}

void BaseInit::prepare_data() {
    LOGD("Setup data tmp\n");
    xmkdir("/data", 0755);
    xmount("magisk", "/data", "tmpfs", 0, "mode=755");

    cp_afc("/init", "/data/magiskinit");
    cp_afc("/.backup", "/data/.backup");
    cp_afc("/overlay.d", "/data/overlay.d");
}

void MagiskInit::setup_tmp(const char *path) {
    LOGD("Setup Magisk tmp at %s\n", path);
    chdir("/data");

    xmkdir(INTLROOT, 0755);
    xmkdir(MIRRDIR, 0);
    xmkdir(BLOCKDIR, 0);
    xmkdir(WORKERDIR, 0);

    mount_rules_dir(path, rules_dev);

    cp_afc(".backup/.magisk", INTLROOT "/config");
    rm_rf(".backup");

    // Create applet symlinks
    for (int i = 0; applet_names[i]; ++i)
        xsymlink("./magisk", applet_names[i]);
    xsymlink("./magiskpolicy", "supolicy");

    xmount(".", path, nullptr, MS_BIND | MS_REC, nullptr);

    chdir("/");
}
