#include <sys/mount.h>

#include <consts.hpp>
#include <sepolicy.hpp>

#include "init.hpp"

using namespace std;

void MagiskInit::patch_sepolicy(const char *in, const char *out) {
    LOGD("Patching monolithic policy\n");
    auto sepol = SePolicy::from_file(in);

    sepol.magisk_rules();

    // Custom rules
    auto rule = "/data/" PREINITMIRR "/sepolicy.rule";
    if (xaccess(rule, R_OK) == 0) {
        LOGD("Loading custom sepolicy patch: [%s]\n", rule);
        sepol.load_rule_file(rule);
    }

    LOGD("Dumping sepolicy to: [%s]\n", out);
    sepol.to_file(out);

    // Remove OnePlus stupid debug sepolicy and use our own
    if (access("/sepolicy_debug", F_OK) == 0) {
        unlink("/sepolicy_debug");
        link("/sepolicy", "/sepolicy_debug");
    }
}

#define MOCK_COMPAT    SELINUXMOCK "/compatible"
#define MOCK_LOAD      SELINUXMOCK "/load"
#define MOCK_ENFORCE   SELINUXMOCK "/enforce"

bool MagiskInit::hijack_sepolicy() {
    xmkdir(SELINUXMOCK, 0);

    if (access("/system/bin/init", F_OK) == 0) {
        // On 2SI devices, the 2nd stage init file is always a dynamic executable.
        // This meant that instead of going through convoluted methods trying to alter
        // and block init's control flow, we can just LD_PRELOAD and replace the
        // security_load_policy function with our own implementation.
        cp_afc("init-ld", "/dev/preload.so");
        setenv("LD_PRELOAD", "/dev/preload.so", 1);
    }

    // Hijack the "load" and "enforce" node in selinuxfs to manipulate
    // the actual sepolicy being loaded into the kernel
    auto hijack = [&] {
        LOGD("Hijack [" SELINUX_LOAD "]\n");
        close(xopen(MOCK_LOAD, O_CREAT | O_RDONLY, 0600));
        xmount(MOCK_LOAD, SELINUX_LOAD, nullptr, MS_BIND, nullptr);
        LOGD("Hijack [" SELINUX_ENFORCE "]\n");
        mkfifo(MOCK_ENFORCE, 0644);
        xmount(MOCK_ENFORCE, SELINUX_ENFORCE, nullptr, MS_BIND, nullptr);
    };

    string dt_compat;
    if (access(SELINUX_ENFORCE, F_OK) != 0) {
        // selinuxfs not mounted yet. Hijack the dt fstab nodes first
        // and let the original init mount selinuxfs for us.
        // This only happens on Android 8.0 - 9.0

        char buf[4096];
        ssprintf(buf, sizeof(buf), "%s/fstab/compatible", config.dt_dir);
        dt_compat = full_read(buf);
        if (dt_compat.empty()) {
            // Device does not do early mount and uses monolithic policy
            return false;
        }

        // Remount procfs with proper options
        xmount(nullptr, "/proc", nullptr, MS_REMOUNT, "hidepid=2,gid=3009");

        LOGD("Hijack [%s]\n", buf);

        // Preserve sysfs and procfs for hijacking
        mount_list.erase(std::remove_if(
                mount_list.begin(), mount_list.end(),
                [](const string &s) { return s == "/proc" || s == "/sys"; }), mount_list.end());

        mkfifo(MOCK_COMPAT, 0444);
        xmount(MOCK_COMPAT, buf, nullptr, MS_BIND, nullptr);
    } else {
        hijack();
    }

    // Read all custom rules into memory
    string rules;
    auto rule = "/data/" PREINITMIRR "/sepolicy.rule";
    if (xaccess(rule, R_OK) == 0) {
        LOGD("Loading custom sepolicy patch: [%s]\n", rule);
        rules = full_read(rule);
    }
    // Create a new process waiting for init operations
    if (xfork()) {
        // In parent, return and continue boot process
        return true;
    }

    if (!dt_compat.empty()) {
        // This open will block until init calls DoFirstStageMount
        // The only purpose here is actually to wait for init to mount selinuxfs for us
        int fd = xopen(MOCK_COMPAT, O_WRONLY);

        char buf[4096];
        ssprintf(buf, sizeof(buf), "%s/fstab/compatible", config.dt_dir);
        xumount2(buf, MNT_DETACH);

        hijack();

        xwrite(fd, dt_compat.data(), dt_compat.size());
        close(fd);
    }

    // This open will block until init calls security_getenforce
    int fd = xopen(MOCK_ENFORCE, O_WRONLY);

    // Cleanup the hijacks
    umount2("/init", MNT_DETACH);
    xumount2(SELINUX_LOAD, MNT_DETACH);
    xumount2(SELINUX_ENFORCE, MNT_DETACH);

    // Load and patch policy
    auto sepol = SePolicy::from_file(MOCK_LOAD);
    sepol.magisk_rules();
    sepol.load_rules(rules);

    // Load patched policy into kernel
    sepol.to_file(SELINUX_LOAD);

    // restore mounted files' context after sepolicy loaded
    rust::reset_overlay_contexts();

    // Write to the enforce node ONLY after sepolicy is loaded. We need to make sure
    // the actual init process is blocked until sepolicy is loaded, or else
    // restorecon will fail and re-exec won't change context, causing boot failure.
    // We (ab)use the fact that init reads the enforce node, and because
    // it has been replaced with our FIFO file, init will block until we
    // write something into the pipe, effectively hijacking its control flow.

    string enforce = full_read(SELINUX_ENFORCE);
    xwrite(fd, enforce.data(), enforce.length());
    close(fd);

    // At this point, the init process will be unblocked
    // and continue on with restorecon + re-exec.

    // Terminate process
    exit(0);
}
