#include <sys/mount.h>

#include <consts.hpp>
#include <base.hpp>
#include <sys/vfs.h>

#include "init.hpp"

using namespace std;

void FirstStageInit::prepare() {
    prepare_data();
    restore_ramdisk_init();
    auto init = mmap_data("/init", true);
    // Redirect original init to magiskinit
    for (size_t off : init.patch(INIT_PATH, REDIR_PATH)) {
        LOGD("Patch @ %08zX [" INIT_PATH "] -> [" REDIR_PATH "]\n", off);
    }
}

void LegacySARInit::first_stage_prep() {
    // Patch init binary
    int src = xopen("/init", O_RDONLY);
    int dest = xopen("/data/init", O_CREAT | O_WRONLY, 0);
    {
        mmap_data init("/init");
        for (size_t off : init.patch(INIT_PATH, REDIR_PATH)) {
            LOGD("Patch @ %08zX [" INIT_PATH "] -> [" REDIR_PATH "]\n", off);
        }
        write(dest, init.buf(), init.sz());
        fclone_attr(src, dest);
        close(dest);
        close(src);
    }
    xmount("/data/init", "/init", nullptr, MS_BIND, nullptr);
}

bool SecondStageInit::prepare() {
    umount2("/init", MNT_DETACH);
    unlink("/data/init");

    // Make sure init dmesg logs won't get messed up
    argv[0] = (char *) INIT_PATH;

    // Some weird devices like meizu, uses 2SI but still have legacy rootfs
    struct statfs sfs{};
    statfs("/", &sfs);
    if (sfs.f_type == RAMFS_MAGIC || sfs.f_type == TMPFS_MAGIC) {
        // We are still on rootfs, so make sure we will execute the init of the 2nd stage
        unlink("/init");
        xsymlink(INIT_PATH, "/init");
        return true;
    }
    return false;
}
