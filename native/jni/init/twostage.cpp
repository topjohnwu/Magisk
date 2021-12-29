// Force using legacy_signal_inlines.h
#define __ANDROID_API_BACKUP__  __ANDROID_API__
#undef  __ANDROID_API__
#define __ANDROID_API__ 20
#include <android/legacy_signal_inlines.h>
#undef  __ANDROID_API__
#define __ANDROID_API__ __ANDROID_API_BACKUP__

#include <sys/mount.h>
#include <map>
#include <set>

#include <magisk.hpp>
#include <utils.hpp>
#include <socket.hpp>

#include "init.hpp"

using namespace std;

void fstab_entry::to_file(FILE *fp) {
    fprintf(fp, "%s %s %s %s %s\n", dev.data(), mnt_point.data(),
            type.data(), mnt_flags.data(), fsmgr_flags.data());
}

#define set_info(val) \
line[val##1] = '\0'; \
entry.val = &line[val##0];

static bool read_fstab_file(const char *fstab_file, vector<fstab_entry> &fstab) {
    if (!fstab_file || fstab_file[0] == '\0') {
        LOGE("fstab file is empty");
        return false;
    }
    file_readline(fstab_file, [&](string_view l) -> bool {
        if (l[0] == '#' || l.length() == 1)
            return true;
        char *line = (char *) l.data();

        int dev0, dev1, mnt_point0, mnt_point1, type0, type1,
            mnt_flags0, mnt_flags1, fsmgr_flags0, fsmgr_flags1;

        sscanf(line, "%n%*s%n %n%*s%n %n%*s%n %n%*s%n %n%*s%n",
               &dev0, &dev1, &mnt_point0, &mnt_point1, &type0, &type1,
               &mnt_flags0, &mnt_flags1, &fsmgr_flags0, &fsmgr_flags1);

        fstab_entry entry;

        set_info(dev)
        set_info(mnt_point)
        set_info(type)
        set_info(mnt_flags)
        set_info(fsmgr_flags)

        fstab.emplace_back(move(entry));
        return true;
    });
    return true;
}

#define FSR "/first_stage_ramdisk"

extern uint32_t patch_verity(void *buf, uint32_t size);

void FirstStageInit::prepare() {
    if (is_dsu()) {
        rename(backup_init(), "/init");
        LOGI("Skip loading Magisk because of DSU\n");
        return;
    }

    run_finally finally([]{ chdir("/"); });
    if (config->force_normal_boot) {
        xmkdirs(FSR "/system/bin", 0755);
        rename("/init" /* magiskinit */, FSR "/system/bin/init");
        symlink("/system/bin/init", FSR "/init");
        rename(backup_init(), "/init");

        rename("/.backup", FSR "/.backup");
        rename("/overlay.d", FSR "/overlay.d");

        chdir(FSR);
    } else {
        xmkdir("/system", 0755);
        xmkdir("/system/bin", 0755);
        rename("/init" /* magiskinit */ , "/system/bin/init");
        rename(backup_init(), "/init");
    }

    char fstab_file[128];
    fstab_file[0] = '\0';

    // Find existing fstab file
    for (const char *suffix : {config->fstab_suffix, config->hardware, config->hardware_plat }) {
        if (suffix[0] == '\0')
            continue;
        for (const char *prefix: { "odm/etc/fstab", "vendor/etc/fstab", "system/etc/fstab", "fstab" }) {
            sprintf(fstab_file, "%s.%s", prefix, suffix);
            if (access(fstab_file, F_OK) != 0) {
                fstab_file[0] = '\0';
            } else {
                LOGD("Found fstab file: %s\n", fstab_file);
                goto exit_loop;
            }
        }
    }
exit_loop:

    // Try to load dt fstab
    vector<fstab_entry> fstab;
    read_dt_fstab(fstab);

    if (!fstab.empty()) {
        // Dump dt fstab to fstab file in rootfs and force init to use it instead
        bool should_skip = false; // If there's any error in dt fstab and if so, skip loading it

        // All dt fstab entries should be first_stage_mount
        for (auto &entry : fstab) {
            if (!str_contains(entry.fsmgr_flags, "first_stage_mount")) {
                if (!entry.fsmgr_flags.empty())
                    entry.fsmgr_flags += ',';
                entry.fsmgr_flags += "first_stage_mount";
            }
            // If the entry contains slotselect but the current slot is empty, error occurs
            if (config->slot[0] == '\0' && str_contains(entry.fsmgr_flags, "slotselect")) {
                should_skip = true;
            } // TODO: else if expected_field checks and fs_mgr_flags checks
        }

        if (fstab_file[0] == '\0') {
            const char *suffix =
                    config->fstab_suffix[0] ? config->fstab_suffix :
                    (config->hardware[0] ? config->hardware :
                     (config->hardware_plat[0] ? config->hardware_plat : nullptr));
            if (suffix == nullptr) {
                LOGE("Cannot determine fstab suffix!\n");
                return;
            }
            sprintf(fstab_file, "fstab.%s", suffix);
        }
        if (should_skip) {
            // When dt fstab fails, fall back to default fstab
            LOGI("dt fstab contains error, falling back to default fstab");
            fstab.clear();
            if (!read_fstab_file(fstab_file, fstab)) return;

        } else {
            // Patch init to force IsDtFstabCompatible() return false
            auto init = mmap_data("/init", true);
            init.patch({make_pair("android,fstab", "xxx")});
        }
    } else if (!read_fstab_file(fstab_file, fstab)) return;

    // Append oppo's custom fstab
    if (access("oplus.fstab", F_OK) == 0) {
        LOGD("Found fstab file: %s\n", "oplus.fstab");
        set<string> main_mount_point;
        map<string, string> bind_map;
        map<string, fstab_entry> entry_map;

        // used to avoid duplication
        for (auto &entry: fstab) {
            main_mount_point.emplace(entry.mnt_point);
        }

        {
            vector<fstab_entry> oplus_fstab;
            read_fstab_file("oplus.fstab", oplus_fstab);

            for (auto &entry : oplus_fstab) {
                // skip duplicated entry between the main file and the oplus one
                if (main_mount_point.count(entry.mnt_point)) continue;
                if (entry.mnt_flags.find("bind") != string::npos) {
                    bind_map.emplace(std::move(entry.dev), std::move(entry.mnt_point));
                } else {
                    // skip duplicated entry in the same file
                    entry_map.insert_or_assign(decltype(entry.mnt_point){entry.mnt_point}, std::move(entry));
                }
            }
        }

        for (auto &[_, entry] : entry_map) {
            // Mount before switch root, fix img path
            if (str_starts(entry.dev, "loop@/system/"))
                entry.dev.insert(5, "/system_root");

            // change bind mount entries to dev mount since some users reported bind is not working
            // in this case, we drop the original mount point and leave only the one from bind entry
            // because some users reported keeping the original mount point causes bootloop
            if (auto got = bind_map.find(entry.mnt_point); got != bind_map.end()) {
                entry.mnt_point = got->second;
            }

            fstab.push_back(move(entry));
        }

        unlink("oplus.fstab");
    }

    {
        LOGD("Write fstab file: %s\n", fstab_file);
        auto fp = xopen_file(fstab_file, "we");
        for (auto &entry : fstab) {
            // Redirect system mnt_point so init won't switch root in first stage init
            if (entry.mnt_point == "/system")
                entry.mnt_point = "/system_root";

            // Force remove AVB for 2SI since it may bootloop some devices
            auto len = patch_verity(entry.fsmgr_flags.data(), entry.fsmgr_flags.length());
            entry.fsmgr_flags.resize(len);

            entry.to_file(fp.get());
        }
    }
    chmod(fstab_file, 0644);
}

#define INIT_PATH  "/system/bin/init"
#define REDIR_PATH "/system/bin/am"

void SARInit::first_stage_prep() {
    xmount("tmpfs", "/dev", "tmpfs", 0, "mode=755");

    // Patch init binary
    int src = xopen("/init", O_RDONLY);
    int dest = xopen("/dev/init", O_CREAT | O_WRONLY, 0);
    {
        auto init = mmap_data("/init");
        init.patch({ make_pair(INIT_PATH, REDIR_PATH) });
        write(dest, init.buf, init.sz);
        fclone_attr(src, dest);
        close(dest);
    }

    // Replace redirect init with magiskinit
    dest = xopen("/dev/magiskinit", O_CREAT | O_WRONLY, 0);
    write(dest, self.buf, self.sz);
    fclone_attr(src, dest);
    close(src);
    close(dest);

    xmount("/dev/init", "/init", nullptr, MS_BIND, nullptr);
    xmount("/dev/magiskinit", REDIR_PATH, nullptr, MS_BIND, nullptr);
    xumount2("/dev", MNT_DETACH);

    // Block SIGUSR1
    sigset_t block, old;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    sigprocmask(SIG_BLOCK, &block, &old);

    if (int child = xfork()) {
        LOGD("init daemon [%d]\n", child);
        // Wait for children signal
        int sig;
        sigwait(&block, &sig);

        // Restore sigmask
        sigprocmask(SIG_SETMASK, &old, nullptr);
    } else {
        // Establish socket for 2nd stage ack
        struct sockaddr_un sun{};
        int sockfd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
        xbind(sockfd, (struct sockaddr*) &sun, setup_sockaddr(&sun, INIT_SOCKET));
        xlisten(sockfd, 1);

        // Resume parent
        kill(getppid(), SIGUSR1);

        // Wait for second stage ack
        int client = xaccept4(sockfd, nullptr, nullptr, SOCK_CLOEXEC);

        // Write backup files
        string tmp_dir = read_string(client);
        chdir(tmp_dir.data());
        int cfg = xopen(INTLROOT "/config", O_WRONLY | O_CREAT, 0);
        xwrite(cfg, magisk_config.buf, magisk_config.sz);
        close(cfg);
        restore_folder(ROOTOVL, overlays);

        // Ack and bail out!
        write_int(client, 0);
        close(client);
        close(sockfd);

        exit(0);
    }
}
