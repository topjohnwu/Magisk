#include <dlfcn.h>
#include <fcntl.h>

#include <utils.hpp>
#include <socket.hpp>
#include <daemon.hpp>
#include <magisk.hpp>

#include "zygisk.hpp"

using namespace std;

void zygiskd(int socket) {
    if (getuid() != 0 || fcntl(socket, F_GETFD) < 0)
        exit(-1);
    android_logging();

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
        if (!(pfd.revents & POLLIN)) {
            // Something bad happened in magiskd, terminate zygiskd
            exit(0);
        }
        int client = recv_fd(socket);
        int module_id = read_int(client);
        if (module_id < modules.size() && modules[module_id]) {
            exec_task([=, entry = modules[module_id]] {
                int dup = fcntl(client, F_DUPFD_CLOEXEC);
                entry(client);
                // Only close client if it is the same as dup so we don't
                // accidentally close a re-used file descriptor.
                // This check is required because the module companion
                // handler could've closed the file descriptor already.
                if (struct stat s1; fstat(client, &s1) == 0) {
                    struct stat s2{};
                    fstat(dup, &s2);
                    if (s1.st_dev == s2.st_dev && s1.st_ino == s2.st_ino) {
                        close(client);
                    }
                }
                close(dup);
            });
        } else {
            close(client);
        }
    }
}

static int zygiskd_sockets[] = { -1, -1 };
#define zygiskd_socket zygiskd_sockets[is_64_bit]

void connect_companion(int client, bool is_64_bit) {
    if (zygiskd_socket >= 0) {
        // Make sure the socket is still valid
        pollfd pfd = { zygiskd_socket, 0, 0 };
        poll(&pfd, 1, 0);
        if (pfd.revents) {
            // Any revent means error
            close(zygiskd_socket);
            zygiskd_socket = -1;
        }
    }
    if (zygiskd_socket < 0) {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds);
        zygiskd_socket = fds[0];
        if (fork_dont_care() == 0) {
            string exe = MAGISKTMP + "/magisk" + (is_64_bit ? "64" : "32");
            // This fd has to survive exec
            fcntl(fds[1], F_SETFD, 0);
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", fds[1]);
            execlp(exe.data(), "magisk", "--companion", buf, (char *) nullptr);
            exit(-1);
        }
        close(fds[1]);
        vector<int> module_fds = zygisk_module_fds(is_64_bit);
        send_fds(zygiskd_socket, module_fds.data(), module_fds.size());
        // Wait for ack
        if (read_int(zygiskd_socket) != 0) {
            return;
        }
    }
    send_fd(zygiskd_socket, client);
}
