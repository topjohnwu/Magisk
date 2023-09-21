#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <vector>

#include <xz.h>

#include <base.hpp>
#include <embed.hpp>

#include "init.hpp"

using namespace std;

bool unxz(out_stream &strm, rust::Slice<const uint8_t> bytes) {
    uint8_t out[8192];
    xz_crc32_init();
    size_t size = bytes.size();
    struct xz_dec *dec = xz_dec_init(XZ_DYNALLOC, 1 << 26);
    struct xz_buf b = {
        .in = bytes.data(),
        .in_pos = 0,
        .in_size = size,
        .out = out,
        .out_pos = 0,
        .out_size = sizeof(out)
    };
    enum xz_ret ret;
    do {
        ret = xz_dec_run(dec, &b);
        if (ret != XZ_OK && ret != XZ_STREAM_END)
            return false;
        strm.write(out, b.out_pos);
        b.out_pos = 0;
    } while (b.in_pos != size);
    return true;

}

void restore_ramdisk_init() {
    unlink("/init");

    const char *orig_init = backup_init();
    if (access(orig_init, F_OK) == 0) {
        xrename(orig_init, "/init");
    } else {
        // If the backup init is missing, this means that the boot ramdisk
        // was created from scratch, and the real init is in a separate CPIO,
        // which is guaranteed to be placed at /system/bin/init.
        xsymlink(INIT_PATH, "/init");
    }
}

class RecoveryInit : public BaseInit {
public:
    using BaseInit::BaseInit;
    void start() override {
        LOGD("Ramdisk is recovery, abort\n");
        restore_ramdisk_init();
        rm_rf("/.backup");
        exec_init();
    }
};

int main(int argc, char *argv[]) {
    umask(0);

    auto name = basename(argv[0]);
    if (name == "magisk"sv)
        return magisk_proxy_main(argc, argv);

    if (getpid() != 1)
        return 1;

    BaseInit *init;
    BootConfig config{};

    if (argc > 1 && argv[1] == "selinux_setup"sv) {
        rust::setup_klog();
        init = new SecondStageInit(argv);
    } else {
        // This will also mount /sys and /proc
        load_kernel_info(&config);

        if (config.skip_initramfs)
            init = new LegacySARInit(argv, &config);
        else if (config.force_normal_boot)
            init = new FirstStageInit(argv, &config);
        else if (access("/sbin/recovery", F_OK) == 0 || access("/system/bin/recovery", F_OK) == 0)
            init = new RecoveryInit(argv, &config);
        else if (check_two_stage())
            init = new FirstStageInit(argv, &config);
        else
            init = new RootFSInit(argv, &config);
    }

    // Run the main routine
    init->start();
    exit(1);
}
