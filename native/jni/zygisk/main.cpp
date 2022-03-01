#include <sys/mount.h>
#include <android/dlext.h>
#include <dlfcn.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <socket.hpp>
#include <daemon.hpp>

#include "zygisk.hpp"

using namespace std;

// Entrypoint for app_process overlay
int app_process_main(int argc, char *argv[]) {
    android_logging();
    char buf[PATH_MAX];

    bool zygote = false;
    if (auto fp = open_file("/proc/self/attr/current", "r")) {
        fscanf(fp.get(), "%s", buf);
        zygote = (buf == "u:r:zygote:s0"sv);
    }

    if (!zygote) {
        // For the non zygote case, we need to get real app_process via passthrough
        // We have to connect magiskd via exec-ing magisk due to SELinux restrictions

        // This is actually only relevant for calling app_process via ADB shell
        // because zygisk shall already have the app_process overlays unmounted
        // during app process specialization within its private mount namespace.

        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds);
        if (fork_dont_care() == 0) {
            // This fd has to survive exec
            fcntl(fds[1], F_SETFD, 0);
            snprintf(buf, sizeof(buf), "%d", fds[1]);
#if defined(__LP64__)
            execlp("magisk", "zygisk", "passthrough", buf, "1", (char *) nullptr);
#else
            execlp("magisk", "zygisk", "passthrough", buf, "0", (char *) nullptr);
#endif
            exit(-1);
        }

        close(fds[1]);
        if (read_int(fds[0]) != 0)
            return 1;
        int app_proc_fd = recv_fd(fds[0]);
        if (app_proc_fd < 0)
            return 1;
        close(fds[0]);

        fcntl(app_proc_fd, F_SETFD, FD_CLOEXEC);
        fexecve(app_proc_fd, argv, environ);
        return 1;
    }

    if (int socket = zygisk_request(ZygiskRequest::SETUP); socket >= 0) {
        do {
            if (read_int(socket) != 0)
                break;

            int app_proc_fd = recv_fd(socket);
            if (app_proc_fd < 0)
                break;

            string tmp = read_string(socket);
            xreadlink("/proc/self/exe", buf, sizeof(buf));
            if (char *ld = getenv("LD_PRELOAD")) {
                string env = ld;
                env += ':';
                env += buf;
                setenv("LD_PRELOAD", env.data(), 1);
            } else {
                setenv("LD_PRELOAD", buf, 1);
            }
            setenv(INJECT_ENV_1, "1", 1);
            setenv(MAGISKTMP_ENV, tmp.data(), 1);

            close(socket);

            fcntl(app_proc_fd, F_SETFD, FD_CLOEXEC);
            fexecve(app_proc_fd, argv, environ);
        } while (false);

        close(socket);
    }

    // If encountering any errors, unmount and execute the original app_process
    xreadlink("/proc/self/exe", buf, sizeof(buf));
    xumount2("/proc/self/exe", MNT_DETACH);
    execve(buf, argv, environ);
    return 1;
}

static void zygiskd(int socket) {
    if (getuid() != 0 || fcntl(socket, F_GETFD) < 0)
        exit(-1);

#if defined(__LP64__)
    set_nice_name("zygiskd64");
    LOGI("* Launching zygiskd64\n");
#else
    set_nice_name("zygiskd32");
    LOGI("* Launching zygiskd32\n");
#endif

    // Load modules
    using comp_entry = void(*)(int);
    vector<comp_entry> modules;
    {
        vector<int> module_fds = recv_fds(socket);
        char buf[256];
        for (int fd : module_fds) {
            snprintf(buf, sizeof(buf), "/proc/self/fd/%d", fd);
            comp_entry entry = nullptr;
            if (void *h = dlopen(buf, RTLD_LAZY)) {
                *(void **) &entry = dlsym(h, "zygisk_companion_entry");
            }
            modules.push_back(entry);
        }
    }

    // ack
    write_int(socket, 0);

    // Start accepting requests
    pollfd pfd = { socket, POLLIN, 0 };
    for (;;) {
        poll(&pfd, 1, -1);
        if (pfd.revents && !(pfd.revents & POLLIN)) {
            // Something bad happened in magiskd, terminate zygiskd
            exit(0);
        }
        int client = recv_fd(socket);
        if (client < 0) {
            // Something bad happened in magiskd, terminate zygiskd
            exit(0);
        }
        int module_id = read_int(client);
        if (module_id >= 0 && module_id < modules.size() && modules[module_id]) {
            exec_task([=, entry = modules[module_id]] {
                struct stat s1;
                fstat(client, &s1);
                entry(client);
                // Only close client if it is the same file so we don't
                // accidentally close a re-used file descriptor.
                // This check is required because the module companion
                // handler could've closed the file descriptor already.
                if (struct stat s2; fstat(client, &s2) == 0) {
                    if (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino) {
                        close(client);
                    }
                }
            });
        } else {
            close(client);
        }
    }
}

// Entrypoint where we need to re-exec ourselves
// This should only ever be called internally
int zygisk_main(int argc, char *argv[]) {
    android_logging();

    if (argc == 3 && argv[1] == "companion"sv) {
        zygiskd(parse_int(argv[2]));
    } else if (argc == 4 && argv[1] == "passthrough"sv) {
        int client = parse_int(argv[2]);
        int is_64_bit = parse_int(argv[3]);
        if (fcntl(client, F_GETFD) < 0)
            return 1;
        if (int magiskd = connect_daemon(MainRequest::ZYGISK_PASSTHROUGH); magiskd >= 0) {
            write_int(magiskd, ZygiskRequest::PASSTHROUGH);
            write_int(magiskd, is_64_bit);

            if (read_int(magiskd) != 0) {
                write_int(client, 1);
                return 0;
            }

            write_int(client, 0);
            int real_app_fd = recv_fd(magiskd);
            send_fd(client, real_app_fd);
        } else {
            write_int(client, 1);
            return 0;
        }
    }
    return 0;
}
