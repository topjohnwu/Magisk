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

void FirstStageInit::prepare() {
    xmkdirs("/data", 0755);
    xmount("tmpfs", "/data", "tmpfs", 0, "mode=755");
    cp_afc("/init", "/data/magiskinit");
    {
        auto init = mmap_data(backup_init(), true);
        init.patch({make_pair("/system/bin/init", "/data/magiskinit")});
    }
    unlink("/init");
    xrename(backup_init(), "/init");
    cp_afc(".backup", "/data/.backup");
    cp_afc("overlay.d", "/data/overlay.d");
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
        init.patch({make_pair(INIT_PATH, REDIR_PATH)});
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
