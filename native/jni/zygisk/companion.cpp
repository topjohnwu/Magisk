#include <dlfcn.h>
#include <fcntl.h>

#include <utils.hpp>
#include <socket.hpp>
#include <daemon.hpp>

#include "zygisk.hpp"

using namespace std;

static int zygiskd_socket = -1;

[[noreturn]] static void zygiskd(int socket) {
    set_nice_name("zygiskd");
    LOGI("* Launching zygiskd\n");

    // Load modules
    using comp_entry = void(*)(int);
    vector<comp_entry> modules;
    {
#if defined(__LP64__)
        vector<int> module_fds = zygisk_module_fds(true);
#else
        vector<int> module_fds = zygisk_module_fds(false);
#endif
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

void start_companion(int client) {
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
            zygiskd(fds[1]);
        }
    }
    send_fd(zygiskd_socket, client);
}
