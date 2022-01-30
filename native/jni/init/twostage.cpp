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

static void read_fstab_file(const char *fstab_file, vector<fstab_entry> &fstab) {
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

        fstab.emplace_back(std::move(entry));
        return true;
    });
}

#define FSR "/first_stage_ramdisk"

extern uint32_t patch_verity(void *buf, uint32_t size);

static void append_oplus(vector<fstab_entry> &fstab) {
    LOGD("Found fstab file: oplus.fstab\n");
    map<string, string> bind_map;
    vector<fstab_entry> entry_list;

    {
        // Make sure no duplicate mnt_point exists
        set<string_view> mount_points;

        // Add main fstab entry mnt_points
        for (auto &entry: fstab) {
            mount_points.emplace(entry.mnt_point);
        }

        vector<fstab_entry> oplus_fstab;
        read_fstab_file("oplus.fstab", oplus_fstab);

        for (auto &entry : oplus_fstab) {
            if (mount_points.count(entry.mnt_point))
                continue;
            if (str_contains(entry.mnt_flags, "bind")) {
                bind_map.emplace(std::move(entry.dev), std::move(entry.mnt_point));
            } else {
                mount_points.emplace(entry.mnt_point);
                entry_list.emplace_back(std::move(entry));
            }
        }
    }

    for (auto &entry : entry_list) {
        // Mount before switch root, fix img path
        if (str_starts(entry.dev, "loop@/system/"))
            entry.dev.insert(5, "/system_root");

        // change bind mount entries to dev mount since some users reported bind is not working
        // in this case, we drop the original mount point and leave only the one from bind entry
        // because some users reported keeping the original mount point causes bootloop
        if (auto it = bind_map.find(entry.mnt_point); it != bind_map.end()) {
            entry.mnt_point = it->second;
        }

        fstab.push_back(std::move(entry));
    }

    unlink("oplus.fstab");
}

void FirstStageInit::get_default_fstab(char *buf, size_t len) {
    buf[0] = '\0';

    // Find existing fstab file
    for (const char *suffix : { config->fstab_suffix, config->hardware, config->hardware_plat }) {
        if (suffix[0] == '\0')
            continue;
        for (const char *prefix: { "odm/etc/fstab", "vendor/etc/fstab", "system/etc/fstab", "fstab" }) {
            snprintf(buf, len, "%s.%s", prefix, suffix);
            if (access(buf, F_OK) == 0) {
                LOGD("Found fstab file: %s\n", buf);
                return;
            }
        }
    }

    buf[0] = '\0';

    // No existing fstab file is found, create a valid path
    const char *suffix = [&]() -> const char * {
        if (config->fstab_suffix[0])
            return config->fstab_suffix;
        if (config->hardware[0])
            return config->hardware;
        if (config->hardware_plat[0])
            return config->hardware_plat;
        return nullptr;
    }();
    if (suffix == nullptr) {
        LOGE("Cannot determine fstab suffix!\n");
        return;
    }
    snprintf(buf, len, "fstab.%s", suffix);
}

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
    get_default_fstab(fstab_file, sizeof(fstab_file));

    // Empty fstab file path is an error
    if (fstab_file[0] == '\0')
        return;

    // Try to load dt fstab
    vector<fstab_entry> fstab;
    read_dt_fstab(fstab);

    if (!fstab.empty()) {
        // Dump dt fstab to fstab file in rootfs and force init to use it instead

        // If there's any error in dt fstab, skip loading it
        bool skip = false;

        // All dt fstab entries should be first_stage_mount
        for (auto &entry : fstab) {
            if (!str_contains(entry.fsmgr_flags, "first_stage_mount")) {
                if (!entry.fsmgr_flags.empty())
                    entry.fsmgr_flags += ',';
                entry.fsmgr_flags += "first_stage_mount";
            }
            // If the entry contains slotselect but the current slot is empty, error occurs
            if (config->slot[0] == '\0' && str_contains(entry.fsmgr_flags, "slotselect")) {
                skip = true;
                break;
            } // TODO: else if expected_field checks and fs_mgr_flags checks
        }

        if (skip) {
            // When dt fstab fails, fall back to default fstab
            LOGI("dt fstab contains error, fall back to default fstab\n");
            fstab.clear();
            read_fstab_file(fstab_file, fstab);
        } else {
            // Patch init to force IsDtFstabCompatible() return false
            auto init = mmap_data("/init", true);
            init.patch({make_pair("android,fstab", "xxx")});
        }
    } else {
        read_fstab_file(fstab_file, fstab);
    }

    // Append oppo's custom fstab
    if (access("oplus.fstab", F_OK) == 0)
        append_oplus(fstab);

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
