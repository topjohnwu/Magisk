#include <base.hpp>
#include <init-rs.hpp>

using kv_pairs = std::vector<std::pair<std::string, std::string>>;

// For API 28 AVD, it uses legacy SAR setup that requires
// special hacks in magiskinit to work properly. We do not
// necessarily want this enabled in production builds.
#define ENABLE_AVD_HACK 0

struct BootConfig {
    bool skip_initramfs;
    bool force_normal_boot;
    bool rootwait;
    bool emulator;
    char slot[3];
    char dt_dir[64];
    char fstab_suffix[32];
    char hardware[32];
    char hardware_plat[32];

    void set(const kv_pairs &);
    void print();
};

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"
#define INIT_PATH  "/system/bin/init"

extern std::vector<std::string> mount_list;

int magisk_proxy_main(int argc, char *argv[]);
bool unxz(int fd, const uint8_t *buf, size_t size);
void load_kernel_info(BootConfig *config);
bool check_two_stage();
void setup_klog();
const char *backup_init();
void restore_ramdisk_init();
int dump_manager(const char *path, mode_t mode);
int dump_preload(const char *path, mode_t mode);

/***************
 * Base classes
 ***************/

class BaseInit {
protected:
    BootConfig *config = nullptr;
    char **argv = nullptr;

    [[noreturn]] void exec_init();
public:
    BaseInit(char *argv[], BootConfig *config = nullptr) : config(config), argv(argv) {}
    virtual ~BaseInit() = default;
    virtual void start() = 0;
};

class MagiskInit : public BaseInit {
private:
    void mount_rules_dir();
protected:
    mmap_data self;
    mmap_data magisk_cfg;
    std::string custom_rules_dir;

#if ENABLE_AVD_HACK
    // When this boolean is set, this means we are currently
    // running magiskinit on legacy SAR AVD emulator
    bool avd_hack = false;
#endif

    void patch_sepolicy(const char *in, const char *out);
    bool hijack_sepolicy();
    void setup_tmp(const char *path);
    void patch_rw_root();
public:
    using BaseInit::BaseInit;
};

class SARBase : public MagiskInit {
protected:
    std::vector<raw_file> overlays;

    void backup_files();
    void patch_ro_root();
public:
    using MagiskInit::MagiskInit;
};

/***************
 * 2 Stage Init
 ***************/

class FirstStageInit : public BaseInit {
private:
    void prepare();
public:
    FirstStageInit(char *argv[], BootConfig *config) : BaseInit(argv, config) {
        LOGD("%s\n", __FUNCTION__);
    };
    void start() override {
        prepare();
        exec_init();
    }
};

class SecondStageInit : public SARBase {
private:
    bool prepare();
public:
    SecondStageInit(char *argv[]) : SARBase(argv) {
        setup_klog();
        LOGD("%s\n", __FUNCTION__);
    };

    void start() override {
        if (prepare())
            patch_rw_root();
        else
            patch_ro_root();
        exec_init();
    }
};

/*************
 * Legacy SAR
 *************/

class LegacySARInit : public SARBase {
private:
    bool mount_system_root();
    void first_stage_prep();
public:
    LegacySARInit(char *argv[], BootConfig *config) : SARBase(argv, config) {
        LOGD("%s\n", __FUNCTION__);
    };
    void start() override {
        if (mount_system_root())
            first_stage_prep();
        else
            patch_ro_root();
        exec_init();
    }
};

/************
 * Initramfs
 ************/

class RootFSInit : public MagiskInit {
private:
    void prepare();
public:
    RootFSInit(char *argv[], BootConfig *config) : MagiskInit(argv, config) {
        LOGD("%s\n", __FUNCTION__);
    }
    void start() override {
        prepare();
        patch_rw_root();
        exec_init();
    }
};
