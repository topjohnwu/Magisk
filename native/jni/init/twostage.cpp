#include <sys/mount.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <socket.hpp>

#include "init.hpp"

using namespace std;

#define INIT_PATH  "/system/bin/init"
#define REDIR_PATH "/data/magiskinit"

void FirstStageInit::prepare() {
    xmkdirs("/data", 0755);
    xmount("tmpfs", "/data", "tmpfs", 0, "mode=755");
    cp_afc("/init" /* magiskinit */, REDIR_PATH);

    unlink("/init");
    xrename(backup_init(), "/init");

    {
        auto init = mmap_data("/init", true);
        // Redirect original init to magiskinit
        init.patch({ make_pair(INIT_PATH, REDIR_PATH) });
    }

    // Copy files to tmpfs
    cp_afc(".backup", "/data/.backup");
    cp_afc("overlay.d", "/data/overlay.d");
}

void LegacySARInit::first_stage_prep() {
    xmount("tmpfs", "/data", "tmpfs", 0, "mode=755");

    // Patch init binary
    int src = xopen("/init", O_RDONLY);
    int dest = xopen("/data/init", O_CREAT | O_WRONLY, 0);
    {
        auto init = mmap_data("/init");
        init.patch({ make_pair(INIT_PATH, REDIR_PATH) });
        write(dest, init.buf, init.sz);
        fclone_attr(src, dest);
        close(dest);
    }
    xmount("/data/init", "/init", nullptr, MS_BIND, nullptr);

    // Replace redirect init with magiskinit
    dest = xopen(REDIR_PATH, O_CREAT | O_WRONLY, 0);
    write(dest, self.buf, self.sz);
    fclone_attr(src, dest);
    close(src);
    close(dest);

    // Copy files to tmpfs
    xmkdir("/data/.backup", 0);
    xmkdir("/data/overlay.d", 0);
    restore_folder("/data/overlay.d", overlays);
    int cfg = xopen("/data/.backup/config", O_WRONLY | O_CREAT, 0);
    xwrite(cfg, magisk_cfg.buf, magisk_cfg.sz);
    close(cfg);
}

bool SecondStageInit::prepare() {
    backup_files();

    umount2("/init", MNT_DETACH);
    umount2("/proc/self/exe", MNT_DETACH);
    umount2("/data", MNT_DETACH);

    // Make sure init dmesg logs won't get messed up
    argv[0] = (char *) INIT_PATH;

    // Some weird devices like meizu, uses 2SI but still have legacy rootfs
    // Check if root and system are on the same filesystem
    struct stat root{}, system{};
    xstat("/", &root);
    xstat("/system", &system);
    return root.st_dev != system.st_dev;
}
