#include <sys/mount.h>
#include <libgen.h>

#include <magisk.hpp>
#include <base.hpp>

#include "init.hpp"
#include "magiskrc.inc"

using namespace std;

static vector<string> rc_list;

static void patch_init_rc(const char *src, const char *dest, const char *tmp_dir) {
    FILE *rc = xfopen(dest, "we");
    if (!rc) {
        PLOGE("%s: open %s failed", __PRETTY_FUNCTION__, src);
        return;
    }
    file_readline(src, [=](string_view line) -> bool {
        // Do not start vaultkeeper
        if (str_contains(line, "start vaultkeeper")) {
            LOGD("Remove vaultkeeper\n");
            return true;
        }
        // Do not run flash_recovery
        if (str_starts(line, "service flash_recovery")) {
            LOGD("Remove flash_recovery\n");
            fprintf(rc, "service flash_recovery /system/bin/xxxxx\n");
            return true;
        }
        // Samsung's persist.sys.zygote.early will cause Zygote to start before post-fs-data
        if (str_starts(line, "on property:persist.sys.zygote.early=")) {
            LOGD("Invalidate persist.sys.zygote.early\n");
            fprintf(rc, "on property:persist.sys.zygote.early.xxxxx=true\n");
            return true;
        }
        // Else just write the line
        fprintf(rc, "%s", line.data());
        return true;
    });

    fprintf(rc, "\n");

    // Inject custom rc scripts
    for (auto &script : rc_list) {
        // Replace template arguments of rc scripts with dynamic paths
        replace_all(script, "${MAGISKTMP}", tmp_dir);
        fprintf(rc, "\n%s\n", script.data());
    }
    rc_list.clear();

    // Inject Magisk rc scripts
    char pfd_svc[16], ls_svc[16];
    gen_rand_str(pfd_svc, sizeof(pfd_svc));
    gen_rand_str(ls_svc, sizeof(ls_svc));
    LOGD("Inject magisk services: [%s] [%s]\n", pfd_svc, ls_svc);
    fprintf(rc, MAGISK_RC, tmp_dir, pfd_svc, ls_svc);

    fclose(rc);
    clone_attr(src, dest);
}

static void load_overlay_rc(const char *overlay) {
    auto dir = open_dir(overlay);
    if (!dir) return;

    int dfd = dirfd(dir.get());
    // Do not allow overwrite init.rc
    unlinkat(dfd, "init.rc", 0);
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        if (str_ends(entry->d_name, ".rc")) {
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

void SARBase::backup_files() {
    if (access("/overlay.d", F_OK) == 0)
        backup_folder("/overlay.d", overlays);
    else if (access("/data/overlay.d", F_OK) == 0)
        backup_folder("/data/overlay.d", overlays);

    self = mmap_data("/proc/self/exe");
    if (access("/.backup/.magisk", R_OK) == 0)
        magisk_cfg = mmap_data("/.backup/.magisk");
    else if (access("/data/.backup/.magisk", R_OK) == 0)
        magisk_cfg = mmap_data("/data/.backup/.magisk");
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

    auto magisk = mmap_data(m32);
    unlink(m32);
    int fd = xopen("magisk32", O_WRONLY | O_CREAT, 0755);
    unxz(fd, magisk.buf, magisk.sz);
    close(fd);
    patch_socket_name("magisk32");
    if (access(m64, F_OK) == 0) {
        magisk = mmap_data(m64);
        unlink(m64);
        fd = xopen("magisk64", O_WRONLY | O_CREAT, 0755);
        unxz(fd, magisk.buf, magisk.sz);
        close(fd);
        patch_socket_name("magisk64");
        xsymlink("./magisk64", "magisk");
    } else {
        xsymlink("./magisk32", "magisk");
    }

    dump_manager("stub.apk", 0);
}

#define ROOTMIR     MIRRDIR "/system_root"
#define NEW_INITRC  "/system/etc/init/hw/init.rc"

void SARBase::patch_ro_root() {
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

    xmkdir(ROOTOVL, 0);

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

    // Handle overlay.d
    restore_folder(ROOTOVL, overlays);
    overlays.clear();
    load_overlay_rc(ROOTOVL);
    if (access(ROOTOVL "/sbin", F_OK) == 0) {
        // Move files in overlay.d/sbin into tmp_dir
        mv_path(ROOTOVL "/sbin", ".");
    }

    // Patch init.rc
    if (access(NEW_INITRC, F_OK) == 0) {
        // Android 11's new init.rc
        xmkdirs(dirname(ROOTOVL NEW_INITRC), 0755);
        patch_init_rc(NEW_INITRC, ROOTOVL NEW_INITRC, tmp_dir.data());
    } else {
        patch_init_rc("/init.rc", ROOTOVL "/init.rc", tmp_dir.data());
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
    self = mmap_data("/init");
    magisk_cfg = mmap_data("/.backup/.magisk");

    LOGD("Restoring /init\n");
    rename(backup_init(), "/init");
}

#define PRE_TMPDIR "/magisk-tmp"

void MagiskInit::patch_rw_root() {
    // Create hardlink mirror of /sbin to /root
    mkdir("/root", 0777);
    clone_attr("/sbin", "/root");
    link_path("/sbin", "/root");

    // Handle overlays
    if (access("/overlay.d", F_OK) == 0) {
        LOGD("Merge overlay.d\n");
        load_overlay_rc("/overlay.d");
        mv_path("/overlay.d", "/");
    }
    rm_rf("/.backup");

    // Patch init.rc
    patch_init_rc("/init.rc", "/init.p.rc", "/sbin");
    rename("/init.p.rc", "/init.rc");

    bool treble;
    {
        auto init = mmap_data("/init");
        treble = init.contains(SPLIT_PLAT_CIL);
    }

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
    int fd = xopen("/sbin/magisk", O_WRONLY | O_CREAT, 0755);
    write(fd, self.buf, self.sz);
    close(fd);
}

int magisk_proxy_main(int argc, char *argv[]) {
    setup_klog();
    LOGD("%s\n", __FUNCTION__);

    // Mount rootfs as rw to do post-init rootfs patches
    xmount(nullptr, "/", nullptr, MS_REMOUNT, nullptr);

    unlink("/sbin/magisk");

    // Move tmpfs to /sbin
    // For some reason MS_MOVE won't work, as a workaround bind mount then unmount
    xmount(PRE_TMPDIR, "/sbin", nullptr, MS_BIND | MS_REC, nullptr);
    xumount2(PRE_TMPDIR, MNT_DETACH);
    rmdir(PRE_TMPDIR);

    // Create symlinks pointing back to /root
    recreate_sbin("/root", false);

    // Tell magiskd to remount rootfs
    setenv("REMOUNT_ROOT", "1", 1);
    execv("/sbin/magisk", argv);
    return 1;
}
