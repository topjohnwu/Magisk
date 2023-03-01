#include <sys/mount.h>
#include <libgen.h>
#include <sys/sysmacros.h>

#include <magisk.hpp>
#include <base.hpp>

#include "init.hpp"
#include "magiskrc.inc"

using namespace std;

static vector<string> rc_list;

static void patch_init_rc(const char *path, const char *tmp_dir) {
    auto src_dir = xopen_dir(path);
    if (!src_dir) return;
    int src_fd = dirfd(src_dir.get());
    // first of all, patch init.rc
    auto writable = faccessat(src_fd, "init.rc", W_OK, 0) == 0;
    auto dest_dir = writable ? [&] {
        return xopen_dir(path);
    }() : [&] {
        char buf[PATH_MAX] = {};
        ssprintf(buf, sizeof(buf), "%s%s", ROOTOVL, path);
        xmkdirs(buf, 0755);
        return xopen_dir(buf);
    }();
    int dest_fd = dirfd(dest_dir.get());
    {
        auto src = xopen_file(xopenat(src_fd, "init.rc", O_RDONLY | O_CLOEXEC, 0), "re");
        if (!src) return;
        file_attr attr{};
        getattrat(src_fd, "init.rc", &attr);
        if (writable) unlinkat(src_fd, "init.rc", 0);
        auto dest = xopen_file(xopenat(dest_fd, "init.rc", O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0), "we");
        LOGD("Patching init.rc in %s\n", path);
        file_readline(false, src.get(), [&dest](string_view line) -> bool {
            // Do not start vaultkeeper
            if (str_contains(line, "start vaultkeeper")) {
                LOGD("Remove vaultkeeper\n");
                return true;
            }
            // Do not run flash_recovery
            if (line.starts_with("service flash_recovery")) {
                LOGD("Remove flash_recovery\n");
                fprintf(dest.get(), "service flash_recovery /system/bin/xxxxx\n");
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
            replace_all(script, "${MAGISKTMP}", tmp_dir);
            fprintf(dest.get(), "\n%s\n", script.data());
        }
        rc_list.clear();

        // Inject Magisk rc scripts
        char pfd_svc[16], ls_svc[16];
        gen_rand_str(pfd_svc, sizeof(pfd_svc));
        gen_rand_str(ls_svc, sizeof(ls_svc));
        LOGD("Inject magisk services: [%s] [%s]\n", pfd_svc, ls_svc);
        fprintf(dest.get(), MAGISK_RC, tmp_dir, pfd_svc, ls_svc);
        setattrat(dest_fd, "init.rc", &attr);
    }

    // now, we can start patching init.zygisk*.rc
    for (dirent *entry; (entry = readdir(src_dir.get()));) {
        auto name = std::string_view(entry->d_name);
        if (!name.starts_with("init.zygote") || !name.ends_with(".rc")) continue;
        auto src = xopen_file(xopenat(src_fd, name.data(), O_RDONLY | O_CLOEXEC, 0), "re");
        if (!src) continue;
        file_attr attr{};
        getattrat(src_fd, name.data(), &attr);
        if (writable) unlinkat(src_fd, name.data(), 0);
        auto dest = xopen_file(xopenat(dest_fd, name.data(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0), "we");
        if (!dest) continue;
        LOGD("Patching %s in %s\n", name.data(), path);
        file_readline(false, src.get(), [&dest, &tmp_dir](string_view line) -> bool {
            if (line.starts_with("service zygote ")) {
                LOGD("Inject zygote restart\n");
                fprintf(dest.get(), "%s", line.data());
                fprintf(dest.get(), "    onrestart exec %s/magisk --zygote-restart\n", tmp_dir);
                return true;
            }
            fprintf(dest.get(), "%s", line.data());
            return true;
        });
        setattrat(dest_fd, name.data(), &attr);
    }
}

static void load_overlay_rc(const char *overlay) {
    auto dir = open_dir(overlay);
    if (!dir) return;

    int dfd = dirfd(dir.get());
    // Do not allow overwrite init.rc
    unlinkat(dfd, "init.rc", 0);

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

static string magic_mount_list;

static void magic_mount(const string &sdir, const string &ddir = "") {
    auto dir = xopen_dir(sdir.data());
    if (!dir) return;
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        string src = sdir + "/" + entry->d_name;
        string dest = ddir + "/" + entry->d_name;
        if (access(dest.data(), F_OK) == 0) {
            if (entry->d_type == DT_DIR) {
                // Recursive
                magic_mount(src, dest);
            } else {
                LOGD("Mount [%s] -> [%s]\n", src.data(), dest.data());
                xmount(src.data(), dest.data(), nullptr, MS_BIND, nullptr);
                magic_mount_list += dest;
                magic_mount_list += '\n';
            }
        }
    }
}

static void patch_socket_name(const char *path) {
    static char rstr[16] = { 0 };
    if (rstr[0] == '\0')
        gen_rand_str(rstr, sizeof(rstr));
    auto bin = mmap_data(path, true);
    bin.patch({ make_pair(MAIN_SOCKET, rstr) });
}

static void extract_files(bool sbin) {
    const char *m32 = sbin ? "/sbin/magisk32.xz" : "magisk32.xz";
    const char *m64 = sbin ? "/sbin/magisk64.xz" : "magisk64.xz";
    const char *stub_xz = sbin ? "/sbin/stub.xz" : "stub.xz";

    if (access(m32, F_OK) == 0) {
        auto magisk = mmap_data(m32);
        unlink(m32);
        int fd = xopen("magisk32", O_WRONLY | O_CREAT, 0755);
        unxz(fd, magisk.buf, magisk.sz);
        close(fd);
        patch_socket_name("magisk32");
    }
    if (access(m64, F_OK) == 0) {
        auto magisk = mmap_data(m64);
        unlink(m64);
        int fd = xopen("magisk64", O_WRONLY | O_CREAT, 0755);
        unxz(fd, magisk.buf, magisk.sz);
        close(fd);
        patch_socket_name("magisk64");
        xsymlink("./magisk64", "magisk");
    } else {
        xsymlink("./magisk32", "magisk");
    }
    if (access(stub_xz, F_OK) == 0) {
        auto stub = mmap_data(stub_xz);
        unlink(stub_xz);
        int fd = xopen("stub.apk", O_WRONLY | O_CREAT, 0);
        unxz(fd, stub.buf, stub.sz);
        close(fd);
    }
}

void MagiskInit::parse_config_file() {
    parse_prop_file("/data/.backup/.magisk", [this](auto key, auto value) -> bool {
        if (key == "PREINITDEVICE") {
            preinit_dev = value;
            return false;
        }
        return true;
    });
}

#define ROOTMIR         MIRRDIR "/system_root"
#define NEW_INITRC_DIR  "/system/etc/init/hw"

void MagiskInit::patch_ro_root() {
    mount_list.emplace_back("/data");
    parse_config_file();

    string tmp_dir;

    if (access("/sbin", F_OK) == 0) {
        tmp_dir = "/sbin";
    } else {
        char buf[8];
        gen_rand_str(buf, sizeof(buf));
        tmp_dir = "/dev/"s + buf;
        xmkdir(tmp_dir.data(), 0);
    }

    setup_tmp(tmp_dir.data());
    chdir(tmp_dir.data());

    // Mount system_root mirror
    xmkdir(ROOTMIR, 0755);
    xmount("/", ROOTMIR, nullptr, MS_BIND, nullptr);
    mount_list.emplace_back(tmp_dir + "/" ROOTMIR);

    // Recreate original sbin structure if necessary
    if (tmp_dir == "/sbin")
        recreate_sbin(ROOTMIR "/sbin", true);

    xrename("overlay.d", ROOTOVL);

#if ENABLE_AVD_HACK
    // Handle avd hack
    if (avd_hack) {
        int src = xopen("/init", O_RDONLY | O_CLOEXEC);
        auto init = mmap_data("/init");
        // Force disable early mount on original init
        init.patch({ make_pair("android,fstab", "xxx") });
        int dest = xopen(ROOTOVL "/init", O_CREAT | O_WRONLY | O_CLOEXEC, 0);
        xwrite(dest, init.buf, init.sz);
        fclone_attr(src, dest);
        close(src);
        close(dest);
    }
#endif

    load_overlay_rc(ROOTOVL);
    if (access(ROOTOVL "/sbin", F_OK) == 0) {
        // Move files in overlay.d/sbin into tmp_dir
        mv_path(ROOTOVL "/sbin", ".");
    }

    // Patch init.rc
    if (access(NEW_INITRC_DIR, F_OK) == 0) {
        // Android 11's new init.rc
        patch_init_rc(NEW_INITRC_DIR, tmp_dir.data());
    } else {
        patch_init_rc("/", tmp_dir.data());
    }

    // Extract magisk
    extract_files(false);

    // Oculus Go will use a special sepolicy if unlocked
    if (access("/sepolicy.unlocked", F_OK) == 0) {
        patch_sepolicy("/sepolicy.unlocked", ROOTOVL "/sepolicy.unlocked");
    } else if ((access(SPLIT_PLAT_CIL, F_OK) != 0 && access("/sepolicy", F_OK) == 0) || !hijack_sepolicy()) {
        patch_sepolicy("/sepolicy", ROOTOVL "/sepolicy");
    }

    // Mount rootdir
    magic_mount(ROOTOVL);
    int dest = xopen(ROOTMNT, O_WRONLY | O_CREAT, 0);
    write(dest, magic_mount_list.data(), magic_mount_list.length());
    close(dest);

    chdir("/");
}

void RootFSInit::prepare() {
    prepare_data();
    LOGD("Restoring /init\n");
    rename(backup_init(), "/init");
}

#define PRE_TMPSRC "/magisk"
#define PRE_TMPDIR PRE_TMPSRC "/tmp"

void MagiskInit::patch_rw_root() {
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
    patch_init_rc("/", "/sbin");

    bool treble;
    {
        auto init = mmap_data("/init");
        treble = init.contains(SPLIT_PLAT_CIL);
    }

    xmkdir(PRE_TMPSRC, 0);
    xmount("tmpfs", PRE_TMPSRC, "tmpfs", 0, "mode=755");
    xmkdir(PRE_TMPDIR, 0);
    setup_tmp(PRE_TMPDIR);
    chdir(PRE_TMPDIR);

    // Extract magisk
    extract_files(true);

    if ((!treble && access("/sepolicy", F_OK) == 0) || !hijack_sepolicy()) {
        patch_sepolicy("/sepolicy", "/sepolicy");
    }

    chdir("/");

    // Dump magiskinit as magisk
    cp_afc(REDIR_PATH, "/sbin/magisk");
}

int magisk_proxy_main(int argc, char *argv[]) {
    setup_klog();
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
    execv("/sbin/magisk", argv);
    return 1;
}
