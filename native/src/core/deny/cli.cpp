#include <sys/wait.h>
#include <sys/mount.h>

#include <consts.hpp>
#include <base.hpp>

#include "deny.hpp"

using namespace std;

[[noreturn]] static void usage() {
    fprintf(stderr,
R"EOF(MagiskHide Config CLI
Usage: magiskhide [action [arguments...] ]
Actions:
   status          Return the MagiskHide status
   enable          Enable MagiskHide
   disable         Disable MagiskHide
   add PKG [PROC]  Add a new target to the hidelist (sulist)
   rm PKG [PROC]   Remove target(s) from the hidelist (sulist)
   ls              Print the current hidelist (sulist)
   exec CMDs...    Execute commands in isolated mount
                   namespace and do all unmounts
Magisk Delta specific Actions:
   sulist          Return the SuList status
   sulist [enable|disable]
                   Enable or disable SuList (need reboot)

)EOF");
    exit(1);
}

void denylist_handler(int client, const sock_cred *cred) {
    if (client < 0) {
        revert_unmount();
        return;
    }

    int req = read_int(client);
    int res = DenyResponse::ERROR;

    switch (req) {
    case DenyRequest::ENFORCE:
        res = enable_deny();
        break;
    case DenyRequest::DISABLE:
        res = disable_deny();
        break;
    case DenyRequest::ADD:
        res = add_list(client);
        break;
    case DenyRequest::REMOVE:
        res = rm_list(client);
        break;
    case DenyRequest::LIST:
        ls_list(client);
        return;
    case DenyRequest::ENFORCE_SULIST:
        update_sulist_config(true);
        res = DenyResponse::OK;
        break;
    case DenyRequest::DISABLE_SULIST:
        update_sulist_config(false);
        res = DenyResponse::OK;
        break;
    case DenyRequest::STATUS:
        res = (denylist_enforced)? DenyResponse::ENFORCED : DenyResponse::NOT_ENFORCED;
        break;
    case DenyRequest::SULIST_STATUS:
        res = (sulist_enabled)? DenyResponse::SULIST_ENFORCED : DenyResponse::SULIST_NOT_ENFORCED;
        break;
    default:
        // Unknown request code
        break;
    }
    write_int(client, res);
    close(client);
}

int denylist_cli(int argc, char **argv) {
    if (argc < 2)
        usage();

    int req = -1;
    if (argv[1] == "enable"sv)
        req = DenyRequest::ENFORCE;
    else if (argv[1] == "disable"sv)
        req = DenyRequest::DISABLE;
    else if (argv[1] == "add"sv){
    	req = DenyRequest::ADD;
    } else if (argv[1] == "rm"sv){
    	req = DenyRequest::REMOVE;
    } else if (argv[1] == "ls"sv)
        req = DenyRequest::LIST;
    else if (argv[1] == "status"sv)
        req = DenyRequest::STATUS;
    else if (argv[1] == "sulist"sv && argc >= 2) {
    	if (argc >= 3) {
            if (argv[2] == "enable"sv)
                req = DenyRequest::ENFORCE_SULIST;
            else if (argv[2] == "disable"sv)
                req = DenyRequest::DISABLE_SULIST;
        } else req = DenyRequest::SULIST_STATUS;
    } else if (argv[1] == "exec"sv && argc > 2) {
        xunshare(CLONE_NEWNS);
        xmount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr);
        revert_unmount();
        execvp(argv[2], argv + 2);
        exit(1);
    } else {
        usage();
    }

    // Send request
    int fd = connect_daemon(+RequestCode::DENYLIST);
    write_int(fd, req);
    if (req == DenyRequest::ADD || req == DenyRequest::REMOVE) {
        write_string(fd, argv[2]);
        write_string(fd, argv[3] ? argv[3] : "");
    }

    // Get response
    int res = read_int(fd);
    if (res < 0 || res >= DenyResponse::END)
        res = DenyResponse::ERROR;
    switch (res) {
    case DenyResponse::NOT_ENFORCED:
        fprintf(stderr, "MagiskHide is disabled\n");
        goto return_code;
    case DenyResponse::ENFORCED:
    	fprintf(stderr, "MagiskHide is enabled\n");
        goto return_code;
    case DenyResponse::SULIST_NOT_ENFORCED:
        fprintf(stderr, "SuList is not enforced\n");
        return 1;
    case DenyResponse::SULIST_ENFORCED:
    	fprintf(stderr, "SuList is enforced\n");
        return 0;
    case DenyResponse::ITEM_EXIST:
        fprintf(stderr, "Target already exists in hidelist\n");
        goto return_code;
    case DenyResponse::ITEM_NOT_EXIST:
        fprintf(stderr, "Target does not exist in hidelist\n");
        goto return_code;
    case DenyResponse::NO_NS:
        fprintf(stderr, "The kernel does not support mount namespace\n");
        goto return_code;
    case DenyResponse::INVALID_PKG:
        fprintf(stderr, "Invalid package / process name\n");
        goto return_code;
    case DenyResponse::ERROR:
        fprintf(stderr, "hide: Daemon error\n");
        return -1;
    case DenyResponse::SULIST_NO_DISABLE:
        fprintf(stderr, "MagiskHide cannot be disabled because SuList is enforced\n");
        return -1;
    case DenyResponse::OK:
        break;
    default:
        __builtin_unreachable();
    }

    if (req == DenyRequest::LIST) {
        string out;
        for (;;) {
            read_string(fd, out);
            if (out.empty())
                break;
            printf("%s\n", out.data());
        }
    }

return_code:
    return req == DenyRequest::STATUS ? res != DenyResponse::ENFORCED : res != DenyResponse::OK;
}
