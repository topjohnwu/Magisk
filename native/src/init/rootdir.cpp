#include <sys/mount.h>
#include <libgen.h>

#include <sepolicy.hpp>
#include <consts.hpp>
#include <base.hpp>
#include <xz.h>

#include "init.hpp"

using namespace std;

static vector<string> rc_list;

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

// When return true, run patch_fissiond
static bool patch_rc_scripts(const char *src_path, const char *tmp_path, bool writable) {
    auto src_dir = xopen_dir(src_path);
    if (!src_dir) return false;
    int src_fd = dirfd(src_dir.get());

    // If writable, directly modify the file in src_path, or else add to rootfs overlay
    auto dest_dir = writable ? [&] {
        return xopen_dir(src_path);
    }() : [&] {
        char buf[PATH_MAX] = {};
        ssprintf(buf, sizeof(buf), ROOTOVL "%s", src_path);
        xmkdirs(buf, 0755);
        return xopen_dir(buf);
    }();
    if (!dest_dir) return false;
    int dest_fd = dirfd(dest_dir.get());

    // First patch init.rc
    {
        auto src = xopen_file(xopenat(src_fd, INIT_RC, O_RDONLY | O_CLOEXEC, 0), "re");
        if (!src) return false;
        if (writable) unlinkat(src_fd, INIT_RC, 0);
        auto dest = xopen_file(
                xopenat(dest_fd, INIT_RC, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0), "we");
        if (!dest) return false;
        LOGD("Patching " INIT_RC " in %s\n", src_path);
        file_readline(false, src.get(), [&dest](string_view line) -> bool {
            // Do not start vaultkeeper
            if (str_contains(line, "start vaultkeeper")) {
                LOGD("Remove vaultkeeper\n");
                return true;
            }
            // Do not run flash_recovery
            if (line.starts_with("service flash_recovery")) {
                LOGD("Remove flash_recovery\n");
                fprintf(dest.get(), "service flash_recovery /system/bin/true\n");
                return true;
            }
            // Samsung's persist.sys.zygote.early will cause Zygote to start before post-fs-data
            if (line.starts_with("on property:persist.sys.zygote.early=")) {
                LOGD("Invalidate persist.sys.zygote.early\n");
                fprintf(dest.get(), "on property:persist.sys.zygote.early.xxxxx=true\n");
                return true;
            }
            // Else just write the line
            fprintf(dest.get(), "%s", line.data());
            return true;
        });

        fprintf(dest.get(), "\n");

        // Inject custom rc scripts
        for (auto &script : rc_list) {
            // Replace template arguments of rc scripts with dynamic paths
            replace_all(script, "${MAGISKTMP}", tmp_path);
            fprintf(dest.get(), "\n%s\n", script.data());
        }
        rc_list.clear();

        // Inject Magisk rc scripts
        rust::inject_magisk_rc(fileno(dest.get()), tmp_path);

        fclone_attr(fileno(src.get()), fileno(dest.get()));
    }

    // Then patch init.zygote*.rc
    for (dirent *entry; (entry = readdir(src_dir.get()));) {
        auto name = std::string_view(entry->d_name);
        if (!name.starts_with("init.zygote") || !name.ends_with(".rc")) continue;
        auto src = xopen_file(xopenat(src_fd, name.data(), O_RDONLY | O_CLOEXEC, 0), "re");
        if (!src) continue;
        if (writable) unlinkat(src_fd, name.data(), 0);
        auto dest = xopen_file(
                xopenat(dest_fd, name.data(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0), "we");
        if (!dest) continue;
        LOGD("Patching %s in %s\n", name.data(), src_path);
        file_readline(false, src.get(), [&dest, &tmp_path](string_view line) -> bool {
            if (line.starts_with("service zygote ")) {
                LOGD("Inject zygote restart\n");
                fprintf(dest.get(), "%s", line.data());
                fprintf(dest.get(),
                        "    onrestart exec " MAGISK_PROC_CON " 0 0 -- %s/magisk --zygote-restart\n", tmp_path);
                return true;
            }
            fprintf(dest.get(), "%s", line.data());
            return true;
        });
        fclone_attr(fileno(src.get()), fileno(dest.get()));
    }

    return faccessat(src_fd, "init.fission_host.rc", F_OK, 0) == 0;
}

void MagiskInit::patch_fissiond(const char *tmp_path) noexcept {
    {
        LOGD("Patching fissiond\n");
        mmap_data fissiond("/system/bin/fissiond", false);
        for (size_t off : fissiond.patch(
                "ro.build.system.fission_single_os",
                "ro.build.system.xxxxxxxxxxxxxxxxx"))
        {
            LOGD("Patch @ %08zX [ro.build.system.fission_single_os] -> "
                 "[ro.build.system.xxxxxxxxxxxxxxxxx]\n", off);
        }
        mkdirs(ROOTOVL "/system/bin", 0755);
        if (auto target_fissiond = xopen_file(ROOTOVL "/system/bin/fissiond", "we")) {
            fwrite(fissiond.buf(), 1, fissiond.sz(), target_fissiond.get());
            clone_attr("/system/bin/fissiond", ROOTOVL "/system/bin/fissiond");
        }
    }
    LOGD("hijack isolated\n");
    auto hijack = xopen_file("/sys/devices/system/cpu/isolated", "re");
    mkfifo(INTLROOT "/isolated", 0777);
    xmount(INTLROOT "/isolated", "/sys/devices/system/cpu/isolated", nullptr, MS_BIND, nullptr);
    if (!xfork()) {
        auto dest = xopen_file(INTLROOT "/isolated", "we");
        LOGD("hijacked isolated\n");
        xumount2("/sys/devices/system/cpu/isolated", MNT_DETACH);
        unlink(INTLROOT "/isolated");
        string content;
        full_read(fileno(hijack.get()), content);
        {
            string target = "/dev/cells/cell2"s + tmp_path;
            xmkdirs(target.data(), 0);
            xmount(tmp_path, target.data(), nullptr, MS_BIND | MS_REC, nullptr);
            mount_overlay("/dev/cells/cell2");
        }
        fprintf(dest.get(), "%s", content.data());
        exit(0);
    }
}

static void load_overlay_rc(const char *overlay) {
    auto dir = open_dir(overlay);
    if (!dir) return;

    int dfd = dirfd(dir.get());
    // Do not allow overwrite init.rc
    unlinkat(dfd, INIT_RC, 0);

    // '/' + name + '\0'
    char buf[NAME_MAX + 2];
    buf[0] = '/';
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        if (!str_ends(entry->d_name, ".rc")) {
            continue;
        }
        strscpy(buf + 1, entry->d_name, sizeof(buf) - 1);
        if (access(buf, F_OK) == 0) {
            LOGD("Replace rc script [%s]\n", entry->d_name);
        } else {
            LOGD("Found rc script [%s]\n", entry->d_name);
            int rc = xopenat(dfd, entry->d_name, O_RDONLY | O_CLOEXEC);
            rc_list.push_back(full_read(rc));
            close(rc);
            unlinkat(dfd, entry->d_name, 0);
        }
    }
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
        xwrite(dest, init.buf(), init.sz());
        fclone_attr(src, dest);
        close(src);
        close(dest);
    }

    load_overlay_rc(ROOTOVL);
    if (access(ROOTOVL "/sbin", F_OK) == 0) {
        // Move files in overlay.d/sbin into tmp_dir
        mv_path(ROOTOVL "/sbin", ".");
    }

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
    load_overlay_rc("/overlay.d");
    mv_path("/overlay.d", "/");
    rm_rf("/data/overlay.d");
    rm_rf("/.backup");

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

rust::Utf8CStr backup_init() {
    if (access("/.backup/init.xz", F_OK) == 0)
        unxz_init("/.backup/init.xz", "/.backup/init");
    return "/.backup/init";
}
