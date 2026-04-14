#include <sys/mount.h>
#include <libgen.h>

#include <sepolicy.hpp>
#include <consts.hpp>
#include <base.hpp>
#include <xz.h>

#include "init.hpp"

using namespace std;

#define NEW_INITRC_DIR  "/system/etc/init/hw"
#define INIT_RC         "init.rc"

static bool unxz(int fd, rust::Slice<const uint8_t> bytes) {
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
        write(fd, out, b.out_pos);
        b.out_pos = 0;
    } while (b.in_pos != size);
    return true;
}

static void recreate_sbin(const char *mirror, bool use_bind_mount) {
    auto dp = xopen_dir(mirror);
    int src = dirfd(dp.get());
    char buf[4096];
    for (dirent *entry; (entry = xreaddir(dp.get()));) {
        string sbin_path = "/sbin/"s + entry->d_name;
        struct stat st;
        fstatat(src, entry->d_name, &st, AT_SYMLINK_NOFOLLOW);
        if (S_ISLNK(st.st_mode)) {
            xreadlinkat(src, entry->d_name, buf, sizeof(buf));
            xsymlink(buf, sbin_path.data());
        } else {
            sprintf(buf, "%s/%s", mirror, entry->d_name);
            if (use_bind_mount) {
                auto mode = st.st_mode & 0777;
                // Create dummy
                if (S_ISDIR(st.st_mode))
                    xmkdir(sbin_path.data(), mode);
                else
                    close(xopen(sbin_path.data(), O_CREAT | O_WRONLY | O_CLOEXEC, mode));

                xmount(buf, sbin_path.data(), nullptr, MS_BIND, nullptr);
            } else {
                xsymlink(buf, sbin_path.data());
            }
        }
    }
}

static void extract_files(bool sbin) {
    const char *magisk_xz = sbin ? "/sbin/magisk.xz" : "magisk.xz";
    const char *stub_xz = sbin ? "/sbin/stub.xz" : "stub.xz";
    const char *init_ld_xz = sbin ? "/sbin/init-ld.xz" : "init-ld.xz";

    if (access(magisk_xz, F_OK) == 0) {
        mmap_data magisk(magisk_xz);
        unlink(magisk_xz);
        int fd = xopen("magisk", O_WRONLY | O_CREAT, 0755);
        unxz(fd, magisk);
        close(fd);
    }
    if (access(stub_xz, F_OK) == 0) {
        mmap_data stub(stub_xz);
        unlink(stub_xz);
        int fd = xopen("stub.apk", O_WRONLY | O_CREAT, 0);
        unxz(fd, stub);
        close(fd);
    }
    if (access(init_ld_xz, F_OK) == 0) {
        mmap_data init_ld(init_ld_xz);
        unlink(init_ld_xz);
        int fd = xopen("init-ld", O_WRONLY | O_CREAT, 0);
        unxz(fd, init_ld);
        close(fd);
    }
}

void MagiskInit::patch_ro_root() noexcept {
    mount_list.emplace_back("/data");
    parse_config_file();

    string tmp_dir;

    if (access("/sbin", F_OK) == 0) {
        tmp_dir = "/sbin";
    } else {
        tmp_dir = "/debug_ramdisk";
        xmkdir("/data/debug_ramdisk", 0);
        xmount("/debug_ramdisk", "/data/debug_ramdisk", nullptr, MS_MOVE, nullptr);
    }

    setup_tmp(tmp_dir.data());
    chdir(tmp_dir.data());

    if (tmp_dir == "/sbin") {
        // Recreate original sbin structure
        xmkdir(MIRRDIR, 0755);
        xmount("/", MIRRDIR, nullptr, MS_BIND, nullptr);
        recreate_sbin(MIRRDIR "/sbin", true);
        xumount2(MIRRDIR, MNT_DETACH);
    } else {
        // Restore debug_ramdisk
        xmount("/data/debug_ramdisk", "/debug_ramdisk", nullptr, MS_MOVE, nullptr);
        rmdir("/data/debug_ramdisk");
    }

    xrename("overlay.d", ROOTOVL);

    extern bool avd_hack;
    // Handle avd hack
    if (avd_hack) {
        int src = xopen("/init", O_RDONLY | O_CLOEXEC);
        mmap_data init("/init");
        // Force disable early mount on original init
        for (size_t off : init.patch("android,fstab", "xxx")) {
            LOGD("Patch @ %08zX [android,fstab] -> [xxx]\n", off);
        }
        int dest = xopen(ROOTOVL "/init", O_CREAT | O_WRONLY | O_CLOEXEC, 0);
        xwrite(dest, init.data(), init.size());
        fclone_attr(src, dest);
        close(src);
        close(dest);
    }

    load_overlay_rc(ROOTOVL, "");
    if (access(ROOTOVL "/sbin", F_OK) == 0) {
        // Move files in overlay.d/sbin into tmp_dir
        mv_path(ROOTOVL "/sbin", ".");
    }

    handle_modules_rc(ROOTOVL);

    // Patch init.rc
    bool p;
    if (access(NEW_INITRC_DIR "/" INIT_RC, F_OK) == 0) {
        // Android 11's new init.rc
        p = patch_rc_scripts(NEW_INITRC_DIR, tmp_dir.data(), false);
    } else {
        p = patch_rc_scripts("/", tmp_dir.data(), false);
    }
    if (p) patch_fissiond(tmp_dir.data());

    // Extract overlay archives
    extract_files(false);

    handle_sepolicy();
    unlink("init-ld");

    // Mount rootdir
    mount_overlay("/");

    chdir("/");
}

#define PRE_TMPSRC "/magisk"
#define PRE_TMPDIR PRE_TMPSRC "/tmp"

void MagiskInit::patch_rw_root() noexcept {
    mount_list.emplace_back("/data");
    parse_config_file();

    // Create hardlink mirror of /sbin to /root
    mkdir("/root", 0777);
    clone_attr("/sbin", "/root");
    link_path("/sbin", "/root");

    // Handle overlays
    load_overlay_rc("/overlay.d", "");
    mv_path("/overlay.d", "/");
    rm_rf("/data/overlay.d");
    rm_rf("/.backup");

    handle_modules_rc("/");

    // Patch init.rc
    if (patch_rc_scripts("/", "/sbin", true))
        patch_fissiond("/sbin");

    xmkdir(PRE_TMPSRC, 0);
    xmount("tmpfs", PRE_TMPSRC, "tmpfs", 0, "mode=755");
    xmkdir(PRE_TMPDIR, 0);
    setup_tmp(PRE_TMPDIR);
    chdir(PRE_TMPDIR);

    // Extract overlay archives
    extract_files(true);

    handle_sepolicy();
    unlink("init-ld");

    chdir("/");

    // Dump magiskinit as magisk
    cp_afc(REDIR_PATH, "/sbin/magisk");
}

int magisk_proxy_main(int, char *argv[]) {
    rust::setup_klog();
    LOGD("%s\n", __FUNCTION__);

    // Mount rootfs as rw to do post-init rootfs patches
    xmount(nullptr, "/", nullptr, MS_REMOUNT, nullptr);

    unlink("/sbin/magisk");

    // Move tmpfs to /sbin
    // make parent private before MS_MOVE
    xmount(nullptr, PRE_TMPSRC, nullptr, MS_PRIVATE, nullptr);
    xmount(PRE_TMPDIR, "/sbin", nullptr, MS_MOVE, nullptr);
    xumount2(PRE_TMPSRC, MNT_DETACH);
    rmdir(PRE_TMPDIR);
    rmdir(PRE_TMPSRC);

    // Create symlinks pointing back to /root
    recreate_sbin("/root", false);

    // Tell magiskd to remount rootfs
    setenv("REMOUNT_ROOT", "1", 1);
    execve("/sbin/magisk", argv, environ);
    return 1;
}

static void unxz_init(const char *init_xz, const char *init) {
    LOGD("unxz %s -> %s\n", init_xz, init);
    int fd = xopen(init, O_WRONLY | O_CREAT, 0777);
    unxz(fd, mmap_data{init_xz});
    close(fd);
    clone_attr(init_xz, init);
    unlink(init_xz);
}

Utf8CStr backup_init() {
    if (access("/.backup/init.xz", F_OK) == 0)
        unxz_init("/.backup/init.xz", "/.backup/init");
    return "/.backup/init";
}
