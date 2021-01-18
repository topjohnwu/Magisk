#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <libgen.h>
#include <vector>

#include <xz.h>
#include <magisk.hpp>
#include <utils.hpp>

#include "binaries.h"
#include "init.hpp"

using namespace std;

// Debug toggle
#define ENABLE_TEST 0

constexpr int (*init_applet_main[])(int, char *[]) =
        { magiskpolicy_main, magiskpolicy_main, nullptr };

bool unxz(int fd, const uint8_t *buf, size_t size) {
    uint8_t out[8192];
    xz_crc32_init();
    struct xz_dec *dec = xz_dec_init(XZ_DYNALLOC, 1 << 26);
    struct xz_buf b = {
        .in = buf,
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
        write(fd, out, b.out_pos);
        b.out_pos = 0;
    } while (b.in_pos != size);
    return true;
}

static int dump_manager(const char *path, mode_t mode) {
    int fd = xopen(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
    if (fd < 0)
        return 1;
    if (!unxz(fd, manager_xz, sizeof(manager_xz)))
        return 1;
    close(fd);
    return 0;
}

class RecoveryInit : public BaseInit {
public:
    RecoveryInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {}
    void start() override {
        LOGD("Ramdisk is recovery, abort\n");
        rename("/.backup/init", "/init");
        rm_rf("/.backup");
        exec_init();
    }
};

#if ENABLE_TEST
class TestInit : public BaseInit {
public:
    TestInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
    void start() override {
        // Place init tests here
    }
};

static int test_main(int argc, char *argv[]) {
    // Log to console
    cmdline_logging();
    log_cb.ex = nop_ex;

    // Switch to isolate namespace
    xunshare(CLONE_NEWNS);
    xmount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr);

    // Unmount everything in reverse
    vector<string> mounts;
    parse_mnt("/proc/mounts", [&](mntent *me) {
        if (me->mnt_dir != "/"sv)
            mounts.emplace_back(me->mnt_dir);
        return true;
    });
    for (auto &m : reversed(mounts))
        xumount(m.data());

    // chroot jail
    chdir(dirname(argv[0]));
    chroot(".");
    chdir("/");

    cmdline cmd{};
    load_kernel_info(&cmd);

    auto init = make_unique<TestInit>(argv, &cmd);
    init->start();

    return 1;
}
#endif // ENABLE_TEST

static int magisk_proxy_main(int argc, char *argv[]) {
    setup_klog();
    auto init = make_unique<MagiskProxy>(argv);
    init->start();
    return 1;
}

int main(int argc, char *argv[]) {
    umask(0);

    auto name = basename(argv[0]);
    if (name == "magisk"sv)
        return magisk_proxy_main(argc, argv);
    for (int i = 0; init_applet[i]; ++i) {
        if (strcmp(name, init_applet[i]) == 0)
            return (*init_applet_main[i])(argc, argv);
    }

#if ENABLE_TEST
    if (getenv("INIT_TEST") != nullptr)
        return test_main(argc, argv);
#endif

    if (argc > 1 && argv[1] == "-x"sv) {
        if (argc > 2 && argv[2] == "manager"sv)
            return dump_manager(argv[3], 0644);
        return 1;
    }

    if (getpid() != 1)
        return 1;

    BaseInit *init;
    cmdline cmd{};

    if (argc > 1 && argv[1] == "selinux_setup"sv) {
        setup_klog();
        init = new SecondStageInit(argv);
    } else {
        // This will also mount /sys and /proc
        load_kernel_info(&cmd);

        if (cmd.skip_initramfs)
            init = new SARInit(argv, &cmd);
        else if (cmd.force_normal_boot)
            init = new FirstStageInit(argv, &cmd);
        else if (access("/sbin/recovery", F_OK) == 0 || access("/system/bin/recovery", F_OK) == 0)
            init = new RecoveryInit(argv, &cmd);
        else if (check_two_stage())
            init = new FirstStageInit(argv, &cmd);
        else
            init = new RootFSInit(argv, &cmd);
    }

    // Run the main routine
    init->start();
    exit(1);
}
