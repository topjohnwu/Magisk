#include <utils.hpp>
#include <magiskpolicy.hpp>

#include "sepolicy.hpp"

using namespace std;

void sepolicy::magisk_rules() {
    // Temp suppress warnings
    auto bak = log_cb.w;
    log_cb.w = nop_log;

    // This indicates API 26+
    bool new_rules = exists("untrusted_app_25");

    // Prevent anything to change sepolicy except ourselves
    deny(ALL, "kernel", "security", "load_policy");

    type(SEPOL_PROC_DOMAIN, "domain");
    permissive(SEPOL_PROC_DOMAIN);  /* Just in case something is missing */
    typeattribute(SEPOL_PROC_DOMAIN, "mlstrustedsubject");
    typeattribute(SEPOL_PROC_DOMAIN, "netdomain");
    typeattribute(SEPOL_PROC_DOMAIN, "bluetoothdomain");
    type(SEPOL_FILE_TYPE, "file_type");
    typeattribute(SEPOL_FILE_TYPE, "mlstrustedobject");

    // Make our root domain unconstrained
    allow(SEPOL_PROC_DOMAIN, ALL, ALL, ALL);
    // Allow us to do any ioctl
    if (db->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL) {
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "blk_file", ALL);
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "fifo_file", ALL);
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "chr_file", ALL);
    }

    // Create unconstrained file type
    allow(ALL, SEPOL_FILE_TYPE, "file", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "dir", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "fifo_file", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "chr_file", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "lnk_file", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "sock_file", ALL);

    if (new_rules) {
        // Make client type literally untrusted_app
        type(SEPOL_CLIENT_DOMAIN, "domain");
        typeattribute(SEPOL_CLIENT_DOMAIN, "coredomain");
        typeattribute(SEPOL_CLIENT_DOMAIN, "appdomain");
        typeattribute(SEPOL_CLIENT_DOMAIN, "untrusted_app_all");
        typeattribute(SEPOL_CLIENT_DOMAIN, "netdomain");
        typeattribute(SEPOL_CLIENT_DOMAIN, "bluetoothdomain");

        type(SEPOL_EXEC_TYPE, "file_type");
        typeattribute(SEPOL_EXEC_TYPE, "exec_type");

        // Basic su client needs
        allow(SEPOL_CLIENT_DOMAIN, SEPOL_EXEC_TYPE, "file", ALL);
        allow(SEPOL_CLIENT_DOMAIN, SEPOL_CLIENT_DOMAIN, ALL, ALL);

        const char *pts[]{"devpts", "untrusted_app_devpts", "untrusted_app_25_devpts"};
        for (auto type : pts) {
            allow(SEPOL_CLIENT_DOMAIN, type, "chr_file", "getattr");
            allow(SEPOL_CLIENT_DOMAIN, type, "chr_file", "read");
            allow(SEPOL_CLIENT_DOMAIN, type, "chr_file", "write");
            allow(SEPOL_CLIENT_DOMAIN, type, "chr_file", "ioctl");
        }

        // Allow these processes to access MagiskSU
        vector<const char *> clients{ "init", "shell", "update_engine", "appdomain" };
        for (auto type : clients) {
            if (!exists(type))
                continue;
            // exec magisk
            allow(type, SEPOL_EXEC_TYPE, "file", "read");
            allow(type, SEPOL_EXEC_TYPE, "file", "open");
            allow(type, SEPOL_EXEC_TYPE, "file", "getattr");
            allow(type, SEPOL_EXEC_TYPE, "file", "execute");
            allow(SEPOL_CLIENT_DOMAIN, type, "process", "sigchld");

            // Auto transit to client domain
            allow(type, SEPOL_CLIENT_DOMAIN, "process", "transition");
            dontaudit(type, SEPOL_CLIENT_DOMAIN, "process", "siginh");
            dontaudit(type, SEPOL_CLIENT_DOMAIN, "process", "rlimitinh");
            dontaudit(type, SEPOL_CLIENT_DOMAIN, "process", "noatsecure");

            // Kill client process
            allow(type, SEPOL_CLIENT_DOMAIN, "process", "signal");
        }

        // type transition require actual types, not attributes
        const char *app_types[]{
            "system_app", "priv_app", "platform_app", "untrusted_app", "untrusted_app_25",
            "untrusted_app_27", "untrusted_app_29", "untrusted_app_30"};
        clients.pop_back();
        clients.insert(clients.end(), app_types, app_types + std::size(app_types));
        for (auto type : clients) {
            // Auto transit to client domain
            type_transition(type, SEPOL_EXEC_TYPE, "process", SEPOL_CLIENT_DOMAIN);
        }

        // Allow system_server to manage magisk_client
        allow("system_server", SEPOL_CLIENT_DOMAIN, "process", "getpgid");
        allow("system_server", SEPOL_CLIENT_DOMAIN, "process", "sigkill");

        // Don't allow pesky processes to monitor audit deny logs when poking magisk daemon socket
        dontaudit(ALL, SEPOL_PROC_DOMAIN, "unix_stream_socket", ALL);

        // Only allow client processes and zygote to connect to magisk daemon socket
        allow(SEPOL_CLIENT_DOMAIN, SEPOL_PROC_DOMAIN, "unix_stream_socket", ALL);
        allow("zygote", SEPOL_PROC_DOMAIN, "unix_stream_socket", ALL);
    } else {
        // Fallback to poking holes in sandbox as Android 4.3 to 7.1 set PR_SET_NO_NEW_PRIVS

        // Allow these processes to access MagiskSU
        const char *clients[] { "init", "shell", "appdomain", "zygote" };
        for (auto type : clients) {
            if (!exists(type))
                continue;
            allow(type, SEPOL_PROC_DOMAIN, "unix_stream_socket", "connectto");
            allow(type, SEPOL_PROC_DOMAIN, "unix_stream_socket", "getopt");
        }
    }

    // Let everyone access tmpfs files (for SAR sbin overlay)
    allow(ALL, "tmpfs", "file", ALL);

    // For relabelling files
    allow("rootfs", "labeledfs", "filesystem", "associate");
    allow(SEPOL_FILE_TYPE, "pipefs", "filesystem", "associate");
    allow(SEPOL_FILE_TYPE, "devpts", "filesystem", "associate");

    // Let init transit to SEPOL_PROC_DOMAIN
    allow("kernel", "kernel", "process", "setcurrent");
    allow("kernel", SEPOL_PROC_DOMAIN, "process", "dyntransition");

    // Let init run stuffs
    allow("kernel", SEPOL_PROC_DOMAIN, "fd", "use");
    allow("init", SEPOL_PROC_DOMAIN, "process", ALL);
    allow("init", "tmpfs", "file", "getattr");
    allow("init", "tmpfs", "file", "execute");

    // suRights
    allow("servicemanager", SEPOL_PROC_DOMAIN, "dir", "search");
    allow("servicemanager", SEPOL_PROC_DOMAIN, "dir", "read");
    allow("servicemanager", SEPOL_PROC_DOMAIN, "file", "open");
    allow("servicemanager", SEPOL_PROC_DOMAIN, "file", "read");
    allow("servicemanager", SEPOL_PROC_DOMAIN, "process", "getattr");
    allow(ALL, SEPOL_PROC_DOMAIN, "process", "sigchld");

    // allowLog
    allow("logd", SEPOL_PROC_DOMAIN, "dir", "search");
    allow("logd", SEPOL_PROC_DOMAIN, "file", "read");
    allow("logd", SEPOL_PROC_DOMAIN, "file", "open");
    allow("logd", SEPOL_PROC_DOMAIN, "file", "getattr");

    // dumpsys
    allow(ALL, SEPOL_PROC_DOMAIN, "fd", "use");
    allow(ALL, SEPOL_PROC_DOMAIN, "fifo_file", "write");
    allow(ALL, SEPOL_PROC_DOMAIN, "fifo_file", "read");
    allow(ALL, SEPOL_PROC_DOMAIN, "fifo_file", "open");
    allow(ALL, SEPOL_PROC_DOMAIN, "fifo_file", "getattr");

    // bootctl
    allow("hwservicemanager", SEPOL_PROC_DOMAIN, "dir", "search");
    allow("hwservicemanager", SEPOL_PROC_DOMAIN, "file", "read");
    allow("hwservicemanager", SEPOL_PROC_DOMAIN, "file", "open");
    allow("hwservicemanager", SEPOL_PROC_DOMAIN, "process", "getattr");

    // For mounting loop devices, mirrors, tmpfs
    allow("kernel", ALL, "file", "read");
    allow("kernel", ALL, "file", "write");

    // Allow all binder transactions
    allow(ALL, SEPOL_PROC_DOMAIN, "binder", ALL);

    // For changing file context
    allow("rootfs", "tmpfs", "filesystem", "associate");

    // Zygisk rules
    allow("zygote", "zygote", "capability", "sys_resource");  // prctl PR_SET_MM
    allow("zygote", "zygote", "process", "execmem");
    allow("zygote", "fs_type", "filesystem", "unmount");
    allow("system_server", "system_server", "process", "execmem");

    // Shut llkd up
    dontaudit("llkd", SEPOL_PROC_DOMAIN, "process", "ptrace");
    dontaudit("llkd", SEPOL_CLIENT_DOMAIN, "process", "ptrace");

    // Allow update_engine/addon.d-v2 to run permissive on all ROMs
    permissive("update_engine");

#if 0
    // Remove all dontaudit in debug mode
    impl->strip_dontaudit();
#endif

    log_cb.w = bak;
}
