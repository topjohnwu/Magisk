#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <vector>

#include <xz.h>

#include <base.hpp>

#include "init.hpp"

using namespace std;

#ifdef USE_CRT0
__BEGIN_DECLS
int tiny_vfprintf(FILE *stream, const char *format, va_list arg);
int vfprintf(FILE *stream, const char *format, va_list arg) {
    return tiny_vfprintf(stream, format, arg);
}
__END_DECLS
#endif

bool unxz(out_stream &strm, rust::Slice<const uint8_t> bytes) {
    uint8_t out[8192];
    xz_crc32_init();
    size_t size = bytes.size();
    struct xz_dec *dec = xz_dec_init(XZ_DYNALLOC, 1 << 26);
    run_finally finally([&] { xz_dec_end(dec); });
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

void MagiskInit::init() noexcept {
    // Get kernel data using procfs and sysfs
    if (access("/proc/cmdline", F_OK) != 0) {
        xmkdir("/proc", 0755);
        xmount("proc", "/proc", "proc", 0, nullptr);
        mount_list.emplace_back("/proc");
    }
    if (access("/sys/block", F_OK) != 0) {
        xmkdir("/sys", 0755);
        xmount("sysfs", "/sys", "sysfs", 0, nullptr);
        mount_list.emplace_back("/sys");
    }

    // Log to kernel
    rust::setup_klog();

    // Load kernel configs
    config.init();
}

static void recovery() {
    LOGI("Ramdisk is recovery, abort\n");
    restore_ramdisk_init();
    rm_rf("/.backup");
}

void MagiskInit::legacy_system_as_root() noexcept {
    LOGI("Legacy SAR Init\n");
    prepare_data();
    bool is_two_stage = mount_system_root();
    if (is_two_stage)
        redirect_second_stage();
    else
        patch_ro_root();
}

void MagiskInit::rootfs() noexcept {
    LOGI("RootFS Init\n");
    prepare_data();
    LOGD("Restoring /init\n");
    rename(backup_init(), "/init");
    patch_rw_root();
}

void MagiskInit::start() noexcept {
    if (argv[1] != nullptr && argv[1] == "selinux_setup"sv)
        second_stage();
    else if (config.skip_initramfs)
        legacy_system_as_root();
    else if (config.force_normal_boot)
        first_stage();
    else if (access("/sbin/recovery", F_OK) == 0 || access("/system/bin/recovery", F_OK) == 0)
        recovery();
    else if (check_two_stage())
        first_stage();
    else
        rootfs();

    // Finally execute the original init
    exec_init();
}

int main(int argc, char *argv[]) {
    umask(0);

    auto name = basename(argv[0]);
    if (name == "magisk"sv)
        return magisk_proxy_main(argc, argv);

    if (getpid() != 1)
        return 1;

    rust::start_magisk_init(argv);
}
