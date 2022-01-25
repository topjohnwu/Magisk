#include <sys/wait.h>
#include <sys/mount.h>

#include <utils.hpp>

#include "deny.hpp"

using namespace std;

[[noreturn]] static void usage() {
    fprintf(stderr,
R"EOF(DenyList Config CLI

Usage: magisk --denylist [action [arguments...] ]
Actions:
   status          Return the enforcement status
   enable          Enable denylist enforcement
   disable         Disable denylist enforcement
   add PKG [PROC]  Add a new target to the denylist
   rm PKG [PROC]   Remove target(s) from the denylist
   ls              Print the current denylist
   exec CMDs...    Execute commands in isolated mount
                   namespace and do all unmounts

)EOF");
    exit(1);
}

void denylist_handler(int client, const sock_cred *cred) {
    if (client < 0) {
        revert_unmount();
        return;
    }

    int req = read_int(client);
    int res = DAEMON_ERROR;

    switch (req) {
    case ENFORCE_DENY:
        res = enable_deny();
        break;
    case DISABLE_DENY:
        res = disable_deny();
        break;
    case ADD_LIST:
        res = add_list(client);
        break;
    case RM_LIST:
        res = rm_list(client);
        break;
    case LS_LIST:
        ls_list(client);
        return;
    case DENY_STATUS:
        res = (zygisk_enabled && denylist_enforced) ? DENY_IS_ENFORCED : DENY_NOT_ENFORCED;
        break;
    }

    write_int(client, res);
    close(client);
}

int denylist_cli(int argc, char **argv) {
    if (argc < 2)
        usage();

    int req;
    if (argv[1] == "enable"sv)
        req = ENFORCE_DENY;
    else if (argv[1] == "disable"sv)
        req = DISABLE_DENY;
    else if (argv[1] == "add"sv)
        req = ADD_LIST;
    else if (argv[1] == "rm"sv)
        req = RM_LIST;
    else if (argv[1] == "ls"sv)
        req = LS_LIST;
    else if (argv[1] == "status"sv)
        req = DENY_STATUS;
    else if (argv[1] == "exec"sv && argc > 2) {
        xunshare(CLONE_NEWNS);
        xmount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr);
        revert_unmount();
        execvp(argv[2], argv + 2);
        exit(1);
    } else {
        usage();
    }

    // Send request
    int fd = connect_daemon();
    write_int(fd, DENYLIST);
    write_int(fd, req);
    if (req == ADD_LIST || req == RM_LIST) {
        write_string(fd, argv[2]);
        write_string(fd, argv[3] ? argv[3] : "");
    }

    // Get response
    int code = read_int(fd);
    switch (code) {
    case DAEMON_SUCCESS:
        break;
    case DENY_NOT_ENFORCED:
        fprintf(stderr, "Denylist is not enforced\n");
        goto return_code;
    case DENY_IS_ENFORCED:
        fprintf(stderr, "Denylist is enforced\n");
        goto return_code;
    case DENYLIST_ITEM_EXIST:
        fprintf(stderr, "Target already exists in denylist\n");
        goto return_code;
    case DENYLIST_ITEM_NOT_EXIST:
        fprintf(stderr, "Target does not exist in denylist\n");
        goto return_code;
    case DENY_NO_NS:
        fprintf(stderr, "The kernel does not support mount namespace\n");
        goto return_code;
    case DENYLIST_INVALID_PKG:
        fprintf(stderr, "Invalid package / process name\n");
        goto return_code;
    case ROOT_REQUIRED:
        fprintf(stderr, "Root is required for this operation\n");
        goto return_code;
    case DAEMON_ERROR:
    default:
        fprintf(stderr, "Daemon error\n");
        return DAEMON_ERROR;
    }

    if (req == LS_LIST) {
        string res;
        for (;;) {
            read_string(fd, res);
            if (res.empty())
                break;
            printf("%s\n", res.data());
        }
    }

return_code:
    return req == DENY_STATUS ? (code == DENY_IS_ENFORCED ? 0 : 1) : code != DAEMON_SUCCESS;
}
