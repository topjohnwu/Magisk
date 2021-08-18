#include <sys/wait.h>
#include <sys/mount.h>

#include <utils.hpp>

#include "magiskhide.hpp"

using namespace std;

[[noreturn]] static void usage(char *arg0) {
    fprintf(stderr,
        "MagiskHide - Hide Config CLI\n\n"
        "Usage: %s [action [arguments...] ]\n\n"
        "Actions:\n"
        "   status          Return the status of magiskhide\n"
        "   enable          Start magiskhide\n"
        "   disable         Stop magiskhide\n"
        "   add PKG [PROC]  Add a new target to the hide list\n"
        "   rm PKG [PROC]   Remove target(s) from the hide list\n"
        "   ls              Print the current hide list\n"
        "   exec CMDs...    Execute commands in isolated mount\n"
        "                   namespace and do all hide unmounts\n"
        , arg0);
    exit(1);
}

void magiskhide_handler(int client, ucred *cred) {
    int req = read_int(client);
    int res = DAEMON_ERROR;

    switch (req) {
    case DISABLE_HIDE:
    case ADD_LIST:
    case RM_LIST:
    case LS_LIST:
        if (!hide_enabled()) {
            write_int(client, HIDE_NOT_ENABLED);
            close(client);
            return;
        }
    }

    switch (req) {
    case ENABLE_HIDE:
        res = enable_hide();
        break;
    case DISABLE_HIDE:
        res = disable_hide();
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
    case HIDE_STATUS:
        res = hide_enabled() ? HIDE_IS_ENABLED : HIDE_NOT_ENABLED;
        break;
    }

    write_int(client, res);
    close(client);
}

int magiskhide_main(int argc, char *argv[]) {
    if (argc < 2)
        usage(argv[0]);

    // CLI backwards compatibility
    const char *opt = argv[1];
    if (opt[0] == '-' && opt[1] == '-')
        opt += 2;

    int req;
    if (opt == "enable"sv)
        req = ENABLE_HIDE;
    else if (opt == "disable"sv)
        req = DISABLE_HIDE;
    else if (opt == "add"sv)
        req = ADD_LIST;
    else if (opt == "rm"sv)
        req = RM_LIST;
    else if (opt == "ls"sv)
        req = LS_LIST;
    else if (opt == "status"sv)
        req = HIDE_STATUS;
    else if (opt == "exec"sv && argc > 2) {
        xunshare(CLONE_NEWNS);
        xmount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr);
        hide_unmount();
        execvp(argv[2], argv + 2);
        exit(1);
    } else {
        usage(argv[0]);
    }

    // Send request
    int fd = connect_daemon();
    write_int(fd, MAGISKHIDE);
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
    case HIDE_NOT_ENABLED:
        fprintf(stderr, "MagiskHide is not enabled\n");
        goto return_code;
    case HIDE_IS_ENABLED:
        fprintf(stderr, "MagiskHide is enabled\n");
        goto return_code;
    case HIDE_ITEM_EXIST:
        fprintf(stderr, "Target already exists in hide list\n");
        goto return_code;
    case HIDE_ITEM_NOT_EXIST:
        fprintf(stderr, "Target does not exist in hide list\n");
        goto return_code;
    case HIDE_NO_NS:
        fprintf(stderr, "Your kernel doesn't support mount namespace\n");
        goto return_code;
    case HIDE_INVALID_PKG:
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
    return req == HIDE_STATUS ? (code == HIDE_IS_ENABLED ? 0 : 1) : code != DAEMON_SUCCESS;
}
