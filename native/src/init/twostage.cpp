#include <sys/mount.h>

#include <consts.hpp>
#include <base.hpp>
#include <sys/vfs.h>

#include "init.hpp"

using namespace std;

void MagiskInit::first_stage() const noexcept {
    LOGI("First Stage Init\n");
    prepare_data();

    if (struct stat st{}; fstatat(-1, "/sdcard", &st, AT_SYMLINK_NOFOLLOW) != 0 &&
        fstatat(-1, "/first_stage_ramdisk/sdcard", &st, AT_SYMLINK_NOFOLLOW) != 0) {
        if (config.force_normal_boot) {
            xmkdirs("/first_stage_ramdisk/storage/self", 0755);
            xsymlink("/system/system/bin/init", "/first_stage_ramdisk/storage/self/primary");
            LOGD("Symlink /first_stage_ramdisk/storage/self/primary -> /system/system/bin/init\n");
            close(xopen("/first_stage_ramdisk/sdcard", O_RDONLY | O_CREAT | O_CLOEXEC, 0));
        } else {
            xmkdirs("/storage/self", 0755);
            xsymlink("/system/system/bin/init", "/storage/self/primary");
            LOGD("Symlink /storage/self/primary -> /system/system/bin/init\n");
        }
        xrename("/init", "/sdcard");
        // Try to keep magiskinit in rootfs for samsung RKP
        if (mount("/sdcard", "/sdcard", nullptr, MS_BIND, nullptr) == 0) {
            LOGD("Bind mount /sdcard -> /sdcard\n");
        } else {
            // rootfs before 3.12
            xmount(REDIR_PATH, "/sdcard", nullptr, MS_BIND, nullptr);
            LOGD("Bind mount " REDIR_PATH " -> /sdcard\n");
        }
        restore_ramdisk_init();
    } else {
        restore_ramdisk_init();
        // fallback to hexpatch if /sdcard exists
        auto init = mmap_data("/init", true);
        // Redirect original init to magiskinit
        for (size_t off : init.patch(INIT_PATH, REDIR_PATH)) {
            LOGD("Patch @ %08zX [" INIT_PATH "] -> [" REDIR_PATH "]\n", off);
        }
    }
}

void MagiskInit::redirect_second_stage() const noexcept {
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
