#include <sys/mount.h>
#include <sys/xattr.h>

#include <consts.hpp>
#include <sepolicy.hpp>

#include "init.hpp"

using namespace std;

#define POLICY_VERSION "/selinux_version"

#define MOCK_VERSION   SELINUXMOCK "/version"
#define MOCK_LOAD      SELINUXMOCK "/load"
#define MOCK_ENFORCE   SELINUXMOCK "/enforce"
#define MOCK_REQPROT   SELINUXMOCK "/checkreqprot"

static void mock_fifo(const char *target, const char *mock) {
    LOGD("Hijack [%s]\n", target);
    mkfifo(mock, 0666);
    xmount(mock, target, nullptr, MS_BIND, nullptr);
}

static void mock_file(const char *target, const char *mock) {
    LOGD("Hijack [%s]\n", target);
    close(xopen(mock, O_CREAT | O_RDONLY, 0666));
    xmount(mock, target, nullptr, MS_BIND, nullptr);
}

enum SePatchStrategy {
    // 2SI, Android 10+
    // On 2SI devices, the 2nd stage init is always a dynamic executable.
    // This meant that instead of going through convoluted hacks, we can just
    // LD_PRELOAD and replace security_load_policy with our own implementation.
    LD_PRELOAD,
    // Treble enabled, Android 8.0+
    // selinuxfs is mounted in init.cpp. Errors when mounting selinuxfs is ignored,
    // which means that we can directly mount selinuxfs ourselves and hijack nodes in it.
    SELINUXFS,
    // Dynamic patching, Android 6.0 - 7.1
    // selinuxfs is mounted in libselinux's selinux_android_load_policy(). Errors when
    // mounting selinuxfs is fatal, which means we need to block init's control flow after
    // it mounted selinuxfs for us, then we can hijack nodes in it.
    LEGACY,
};

void MagiskInit::handle_sepolicy() noexcept {
    xmkdir(SELINUXMOCK, 0711);

    // Read all custom rules into memory
    string rules;
    auto rule = "/data/" PREINITMIRR "/sepolicy.rule";
    if (xaccess(rule, R_OK) == 0) {
        LOGD("Loading custom sepolicy patch: [%s]\n", rule);
        full_read(rule, rules);
    }

    // Step 0: determine strategy

    SePatchStrategy strat;

    if (access("/system/bin/init", F_OK) == 0) {
        strat = LD_PRELOAD;
    } else {
        auto init = mmap_data("/init");
        if (init.contains(SPLIT_PLAT_CIL)) {
            // Supports split policy
            strat = SELINUXFS;
        } else if (init.contains(POLICY_VERSION)) {
            // Does not support split policy, hijack /selinux_version
            strat = LEGACY;
        } else {
            LOGE("Unknown sepolicy setup, abort...\n");
            return;
        }
    }

    // Step 1: setup for intercepting init boot control flow

    switch (strat) {
        case LD_PRELOAD: {
            LOGI("SePatchStrategy: LD_PRELOAD\n");

            cp_afc("init-ld", PRELOAD_LIB);
            setenv("LD_PRELOAD", PRELOAD_LIB, 1);
            mkfifo(PRELOAD_ACK, 0666);
            break;
        }
        case SELINUXFS: {
            LOGI("SePatchStrategy: SELINUXFS\n");

            if (access(SELINUX_ENFORCE, F_OK) != 0) {
                // selinuxfs was not already mounted, mount it ourselves

                // Remount procfs with proper options
                xmount(nullptr, "/proc", nullptr, MS_REMOUNT, "hidepid=2,gid=3009");

                // Preserve sysfs and procfs
                decltype(mount_list) new_mount_list;
                std::remove_copy_if(
                        mount_list.begin(), mount_list.end(),
                        std::back_inserter(new_mount_list),
                        [](const auto &s) { return s == "/proc" || s == "/sys"; });
                new_mount_list.swap(mount_list);

                // Mount selinuxfs
                xmount("selinuxfs", "/sys/fs/selinux", "selinuxfs", 0, nullptr);
            }

            mock_file(SELINUX_LOAD, MOCK_LOAD);
            mock_fifo(SELINUX_ENFORCE, MOCK_ENFORCE);
            break;
        }
        case LEGACY: {
            LOGI("SePatchStrategy: LEGACY\n");

            if (access(POLICY_VERSION, F_OK) != 0) {
                // The file does not exist, create one
                close(xopen(POLICY_VERSION, O_RDONLY | O_CREAT, 0644));
            }

            // The only purpose of this is to block init's control flow after it mounts selinuxfs
            // and before it calls security_load_policy().
            // Target: selinux_android_load_policy() -> set_policy_index() -> open(POLICY_VERSION)
            mock_fifo(POLICY_VERSION, MOCK_VERSION);
            break;
        }
    }

    // Create a new process waiting for init operations
    if (xfork()) {
        return;
    }

    // Step 2: wait for selinuxfs to be mounted (only for LEGACY)

    if (strat == LEGACY) {
        // Busy wait until selinuxfs is mounted
        while (access(SELINUX_ENFORCE, F_OK)) {
            // Retry every 100ms
            usleep(100000);
        }

        // On Android 6.0, init does not call security_getenforce() first; instead it directly
        // call security_setenforce() after security_load_policy(). What's even worse, it opens the
        // enforce node with O_RDWR, which will not block when opening FIFO files. As a workaround,
        // we do not mock the enforce node, and block init with mock checkreqprot instead.
        // Android 7.0 - 7.1 doesn't have this issue, but for simplicity, let's just use the
        // same blocking strategy for both since it also works just fine.

        mock_file(SELINUX_LOAD, MOCK_LOAD);
        mock_fifo(SELINUX_REQPROT, MOCK_REQPROT);

        // This will unblock init at selinux_android_load_policy() -> set_policy_index().
        close(xopen(MOCK_VERSION, O_WRONLY));

        xumount2(POLICY_VERSION, MNT_DETACH);

        // libselinux does not read /selinux_version after open; instead it mmap the file,
        // which can never succeed on FIFO files. This is fine as set_policy_index() will just
        // fallback to the default index 0.
    }

    // Step 3: obtain sepolicy, patch, and load the patched sepolicy

    if (strat == LD_PRELOAD) {
        // This open will block until preload.so finish writing the sepolicy
        owned_fd ack_fd = xopen(PRELOAD_ACK, O_WRONLY);

        auto sepol = SePolicy::from_file(PRELOAD_POLICY);

        // Remove the files before loading the policy
        unlink(PRELOAD_POLICY);
        unlink(PRELOAD_ACK);

        sepol.magisk_rules();
        sepol.load_rules(rules);
        sepol.to_file(SELINUX_LOAD);

        // restore mounted files' context after sepolicy loaded
        restore_overlay_contexts();

        // Write ack to restore preload.so's control flow
        xwrite(ack_fd, &ack_fd, 1);
    } else {
        int mock_enforce = -1;

        if (strat == LEGACY) {
            // Busy wait until sepolicy is fully written.
            struct stat st{};
            decltype(st.st_size) sz;
            do {
                sz = st.st_size;
                // Check every 100ms
                usleep(100000);
                xstat(MOCK_LOAD, &st);
            } while (sz == 0 || sz != st.st_size);
        } else {
            // This open will block until init calls security_getenforce().
            mock_enforce = xopen(MOCK_ENFORCE, O_WRONLY);
        }

        // Cleanup the hijacks
        umount2("/init", MNT_DETACH);
        xumount2(SELINUX_LOAD, MNT_DETACH);
        umount2(SELINUX_ENFORCE, MNT_DETACH);
        umount2(SELINUX_REQPROT, MNT_DETACH);

        auto sepol = SePolicy::from_file(MOCK_LOAD);
        sepol.magisk_rules();
        sepol.load_rules(rules);
        sepol.to_file(SELINUX_LOAD);

        // For some reason, restorecon on /init won't work in some cases
        setxattr("/init", XATTR_NAME_SELINUX, "u:object_r:init_exec:s0", 24, 0);

        // restore mounted files' context after sepolicy loaded
        restore_overlay_contexts();

        // We need to make sure the actual init process is blocked until sepolicy is loaded,
        // or else restorecon will fail and re-exec won't change context, causing boot failure.
        // We (ab)use the fact that init either reads the enforce node, or writes the checkreqprot
        // node, and because both has been replaced with FIFO files, init will block until we
        // handle it, effectively hijacking its control flow until the patched sepolicy is loaded.

        if (strat == LEGACY) {
            // init is blocked on checkreqprot, write to the real node first, then
            // unblock init by opening the mock FIFO.
            owned_fd real_req = xopen(SELINUX_REQPROT, O_WRONLY);
            xwrite(real_req, "0", 1);
            owned_fd mock_req = xopen(MOCK_REQPROT, O_RDONLY);
            full_read(mock_req);
        } else {
            // security_getenforce was called
            string data = full_read(SELINUX_ENFORCE);
            xwrite(mock_enforce, data.data(), data.length());
            close(mock_enforce);
        }
    }

    // At this point, the init process will be unblocked
    // and continue on with restorecon + re-exec.

    // Terminate process
    exit(0);
}
