#include <base.hpp>
#include <stream.hpp>

#include "init-rs.hpp"

using kv_pairs = std::vector<std::pair<std::string, std::string>>;

struct BootConfig {
    bool skip_initramfs = false;
    bool force_normal_boot = false;
    bool rootwait = false;
    bool emulator = false;
    char slot[3]{};
    char dt_dir[64]{};
    char fstab_suffix[32]{};
    char hardware[32]{};
    char hardware_plat[32]{};
    kv_pairs partition_map;

    BootConfig();
private:
    void set(const kv_pairs &);
    void print();
};

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"
#define INIT_PATH  "/system/bin/init"
#define REDIR_PATH "/data/magiskinit"

extern std::vector<std::string> mount_list;

int magisk_proxy_main(int argc, char *argv[]);
bool unxz(out_stream &strm, rust::Slice<const uint8_t> bytes);
bool check_two_stage();
const char *backup_init();
void restore_ramdisk_init();

/***************
 * Base classes
 ***************/

class BaseInit {
protected:
    BootConfig *config;
    char **argv;

    [[noreturn]] void exec_init();
    void prepare_data();
    dev_t find_block(const char *partname);
    void collect_devices();
public:
    BaseInit(char *argv[], BootConfig *config) : config(config), argv(argv) {}
    virtual ~BaseInit() = default;
    virtual void start() = 0;
};

class MagiskInit : public BaseInit {
private:
    std::string preinit_dev;

    void parse_config_file();
    void patch_sepolicy(const char *in, const char *out);
    bool hijack_sepolicy();
    void setup_tmp(const char *path);
    void mount_preinit_dir();
protected:
    void patch_rw_root();
    void patch_ro_root();
public:
    using BaseInit::BaseInit;
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

class SecondStageInit : public MagiskInit {
private:
    bool prepare();
public:
    SecondStageInit(char *argv[], BootConfig *config) : MagiskInit(argv, config) {
        LOGD("%s\n", __FUNCTION__);
    };

    void start() override {
        bool is_rootfs = prepare();
        if (is_rootfs)
            patch_rw_root();
        else
            patch_ro_root();
        exec_init();
    }
};

/*************
 * Legacy SAR
 *************/

class LegacySARInit : public MagiskInit {
private:
    bool mount_system_root();
    void first_stage_prep();
public:
    LegacySARInit(char *argv[], BootConfig *config) : MagiskInit(argv, config) {
        LOGD("%s\n", __FUNCTION__);
    };
    void start() override {
        prepare_data();
        bool is_two_stage = mount_system_root();
        if (is_two_stage)
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
