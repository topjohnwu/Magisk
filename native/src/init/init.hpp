#include <base.hpp>
#include <stream.hpp>

#include "init-rs.hpp"

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"
#define INIT_PATH  "/system/bin/init"
#define REDIR_PATH "/data/magiskinit"

int magisk_proxy_main(int argc, char *argv[]);
bool unxz(out_stream &strm, rust::Slice<const uint8_t> bytes);
bool check_two_stage();
const char *backup_init();
void restore_ramdisk_init();

class MagiskInit {
private:
    std::string preinit_dev;
    std::vector<std::string> mount_list;
    char **argv;
    BootConfig config;

    // Setup mounts and environment
    void setup_tmp(const char *path);
    void collect_devices();
    void mount_preinit_dir();
    void prepare_data();
    dev_t find_block(const char *partname);
    bool mount_system_root();

    // Setup and patch root directory
    void parse_config_file();
    void patch_rw_root();
    void patch_ro_root();

    // Two stage init
    void redirect_second_stage();
    void first_stage();
    void second_stage();

    // SELinux
    void patch_sepolicy(const char *in, const char *out);
    bool hijack_sepolicy();

    [[noreturn]] void exec_init();
    void legacy_system_as_root();
    void rootfs();
public:
    explicit MagiskInit(char *argv[]);
    void start();
};
