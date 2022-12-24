#include <sys/mount.h>

#include <magisk.hpp>
#include <base.hpp>
#include <socket.hpp>

#include "init.hpp"

using namespace std;

void FirstStageInit::prepare() {
    prepare_data();
    restore_ramdisk_init();
    auto init = mmap_data("/init", true);
    // Redirect original init to magiskinit
    init.patch({ make_pair(INIT_PATH, REDIR_PATH) });
}

void LegacySARInit::first_stage_prep() {
    // Patch init binary
    int src = xopen("/init", O_RDONLY);
    int dest = xopen("/data/init", O_CREAT | O_WRONLY, 0);
    {
        auto init = mmap_data("/init");
        init.patch({ make_pair(INIT_PATH, REDIR_PATH) });
        write(dest, init.buf, init.sz);
        fclone_attr(src, dest);
        close(dest);
        close(src);
    }
    xmount("/data/init", "/init", nullptr, MS_BIND, nullptr);
}

bool SecondStageInit::prepare() {
    umount2("/init", MNT_DETACH);
    umount2("/proc/self/exe", MNT_DETACH);

    // Make sure init dmesg logs won't get messed up
    argv[0] = (char *) INIT_PATH;

    // Some weird devices like meizu, uses 2SI but still have legacy rootfs
    // Check if root and system are on different filesystems
    struct stat root{}, system{};
    xstat("/", &root);
    xstat("/system", &system);
    if (root.st_dev != system.st_dev) {
        // We are still on rootfs, so make sure we will execute the init of the 2nd stage
        unlink("/init");
        xsymlink(INIT_PATH, "/init");
        return true;
    }
    return false;
}
