#include <utils.hpp>

#include "raw_data.hpp"

struct cmdline {
    bool skip_initramfs;
    bool force_normal_boot;
    bool rootwait;
    char slot[3];
    char dt_dir[64];
    char fstab_suffix[32];
    char hardware[32];
    char hardware_plat[32];
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
    void to_file(FILE *fp);
};

#define INIT_SOCKET "MAGISKINIT"
#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"

extern std::vector<std::string> mount_list;

bool unxz(int fd, const uint8_t *buf, size_t size);
void load_kernel_info(cmdline *cmd);
bool check_two_stage();
void setup_klog();

/***************
 * Base classes
 ***************/

class BaseInit {
protected:
    cmdline *cmd;
    char **argv;

    [[noreturn]] void exec_init();
    void read_dt_fstab(std::vector<fstab_entry> &fstab);
public:
    BaseInit(char *argv[], cmdline *cmd) : cmd(cmd), argv(argv) {}
    virtual ~BaseInit() = default;
    virtual void start() = 0;
};

class MagiskInit : public BaseInit {
protected:
    mmap_data self;
    mmap_data config;
    std::string custom_rules_dir;

    void mount_with_dt();
    bool patch_sepolicy(const char *file);
    void setup_tmp(const char *path);
    void mount_rules_dir(const char *dev_base, const char *mnt_base);
public:
    MagiskInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {}
};

class SARBase : public MagiskInit {
protected:
    std::vector<raw_file> overlays;

    void backup_files();
    void patch_rootdir();
    void mount_system_root();
public:
    SARBase(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {}
};

/***************
 * 2 Stage Init
 ***************/

class FirstStageInit : public BaseInit {
private:
    void prepare();
public:
    FirstStageInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {
        LOGD("%s\n", __FUNCTION__);
    };
    void start() override {
        prepare();
        exec_init();
    }
};

class SecondStageInit : public SARBase {
private:
    void prepare();
public:
    SecondStageInit(char *argv[]) : SARBase(argv, nullptr) {
        LOGD("%s\n", __FUNCTION__);
    };
    void start() override {
        prepare();
        patch_rootdir();
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
    SARInit(char *argv[], cmdline *cmd) : SARBase(argv, cmd), is_two_stage(false) {
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

class RootFSInit : public MagiskInit {
private:
    void early_mount();
    void patch_rootfs();
public:
    RootFSInit(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {
        LOGD("%s\n", __FUNCTION__);
    }
    void start() override {
        early_mount();
        patch_rootfs();
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
