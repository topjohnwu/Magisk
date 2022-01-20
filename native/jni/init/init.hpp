#include <utils.hpp>

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

struct fstab_entry {
    std::string dev;
    std::string mnt_point;
    std::string type;
    std::string mnt_flags;
    std::string fsmgr_flags;

    fstab_entry() = default;
    fstab_entry(const fstab_entry &) = delete;
    fstab_entry(fstab_entry &&) = default;
    fstab_entry &operator=(const fstab_entry&) = delete;
    fstab_entry &operator=(fstab_entry&&) = default;
    void to_file(FILE *fp);
};

#define INIT_SOCKET "MAGISKINIT"
#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"

extern std::vector<std::string> mount_list;

bool unxz(int fd, const uint8_t *buf, size_t size);
void load_kernel_info(BootConfig *config);
bool is_dsu();
bool check_two_stage();
void setup_klog();
const char *backup_init();

/***************
 * Base classes
 ***************/

class BaseInit {
protected:
    BootConfig *config = nullptr;
    char **argv = nullptr;

    [[noreturn]] void exec_init();
    void read_dt_fstab(std::vector<fstab_entry> &fstab);
public:
    BaseInit(char *argv[], BootConfig *config) : config(config), argv(argv) {}
    virtual ~BaseInit() = default;
    virtual void start() = 0;
};

class MagiskInit : public BaseInit {
protected:
    mmap_data self;
    mmap_data magisk_config;
    std::string custom_rules_dir;

#if ENABLE_AVD_HACK
    // When this boolean is set, this means we are currently
    // running magiskinit on legacy SAR AVD emulator
    bool avd_hack = false;
#else
    // Make it const so compiler can optimize hacks out of the code
    static const bool avd_hack = false;
#endif

    void mount_with_dt();
    bool patch_sepolicy(const char *file);
    void setup_tmp(const char *path);
    void mount_rules_dir(const char *dev_base, const char *mnt_base);
public:
    MagiskInit(char *argv[], BootConfig *cmd) : BaseInit(argv, cmd) {}
};

class SARBase : virtual public MagiskInit {
protected:
    std::vector<raw_file> overlays;

    void backup_files();
    void patch_rootdir();
    void mount_system_root();
public:
    SARBase() = default;
};

/***************
 * 2 Stage Init
 ***************/

class FirstStageInit : public BaseInit {
private:
    void prepare();
    void get_default_fstab(char *buf, size_t len);
public:
    FirstStageInit(char *argv[], BootConfig *cmd) : BaseInit(argv, cmd) {
        LOGD("%s\n", __FUNCTION__);
    };
    void start() override {
        prepare();
        exec_init();
    }
};

/*************
 * Legacy SAR
 *************/

class SARInit : public SARBase {
private:
    bool is_two_stage;

    void early_mount();
    void first_stage_prep();
public:
    SARInit(char *argv[], BootConfig *cmd) : MagiskInit(argv, cmd), is_two_stage(false) {
        LOGD("%s\n", __FUNCTION__);
    };
    void start() override {
        early_mount();
        if (is_two_stage)
            first_stage_prep();
        else
            patch_rootdir();
        exec_init();
    }
};

/************
 * Initramfs
 ************/

class RootFSBase : virtual public MagiskInit {
protected:
    void patch_rootfs();
public:
    RootFSBase() = default;
    void start() = 0;
};

class RootFSInit : public RootFSBase {
private:
    void early_mount();

public:
    RootFSInit(char *argv[], BootConfig *cmd) : MagiskInit(argv, cmd) {
        LOGD("%s\n", __FUNCTION__);
    }
    void start() override {
        early_mount();
        patch_rootfs();
        exec_init();
    }
};

class SecondStageInit : public RootFSBase, public SARBase {
private:
    bool prepare();
public:
    SecondStageInit(char *argv[]) : MagiskInit(argv, nullptr) {
        LOGD("%s\n", __FUNCTION__);
    };

    void start() override {
        if (prepare()) patch_rootfs();
        else patch_rootdir();
        exec_init();
    }
};


class MagiskProxy : public MagiskInit {
public:
    explicit MagiskProxy(char *argv[]) : MagiskInit(argv, nullptr) {
        LOGD("%s\n", __FUNCTION__);
    }
    void start() override;
};
