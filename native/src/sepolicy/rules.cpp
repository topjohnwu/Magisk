#include <consts.hpp>
#include <base.hpp>

#include "policy.hpp"

using namespace std;

void sepolicy::magisk_rules() {
    // Temp suppress warnings
    set_log_level_state(LogLevel::Warn, false);

    // Prevent anything to change sepolicy except ourselves
    deny(ALL, "kernel", "security", "load_policy");

    type(SEPOL_PROC_DOMAIN, "domain");
    permissive(SEPOL_PROC_DOMAIN); // When we are subject, we can do anything
    typeattribute(SEPOL_PROC_DOMAIN, "mlstrustedsubject");
    typeattribute(SEPOL_PROC_DOMAIN, "netdomain");
    typeattribute(SEPOL_PROC_DOMAIN, "appdomain");
    type(SEPOL_FILE_TYPE, "file_type");
    typeattribute(SEPOL_FILE_TYPE, "mlstrustedobject");
    type(SEPOL_LOG_TYPE, "file_type");
    typeattribute(SEPOL_LOG_TYPE, "mlstrustedobject");

    // Create unconstrained file type
    allow("domain", SEPOL_FILE_TYPE, "file", ALL);
    allow("domain", SEPOL_FILE_TYPE, "dir", ALL);
    allow("domain", SEPOL_FILE_TYPE, "fifo_file", ALL);
    allow("domain", SEPOL_FILE_TYPE, "chr_file", ALL);
    allow("domain", SEPOL_FILE_TYPE, "lnk_file", ALL);
    allow("domain", SEPOL_FILE_TYPE, "sock_file", ALL);

    // Only allow zygote to open log pipe
    allow("zygote", SEPOL_LOG_TYPE, "fifo_file", "open");
    allow("zygote", SEPOL_LOG_TYPE, "fifo_file", "read");
    // Allow all processes to output logs
    allow("domain", SEPOL_LOG_TYPE, "fifo_file", "write");

    const char *object_attrs[]{
            "fs_type", "dev_type", "file_type",
            "domain", // subject as object
            "service_manager_type", "hwservice_manager_type", "vndservice_manager_type",
            "port_type", "node_type", "property_type",
            // others covered by permissive
    };

    // To suppress avc logs, explicitly allow us to do anything
    for (auto type: object_attrs) {
        if (!exists(type))
            continue;
        allow(SEPOL_PROC_DOMAIN, type, ALL, ALL);
    }
    // and any ioctl
    if (impl->db->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL) {
        argument all;
        all.first.push_back(nullptr);
        for (int i = 0; i <= 3; i++) {
            allowxperm(SEPOL_PROC_DOMAIN, object_attrs[i], "blk_file", all);
            allowxperm(SEPOL_PROC_DOMAIN, object_attrs[i], "fifo_file", all);
            allowxperm(SEPOL_PROC_DOMAIN, object_attrs[i], "chr_file", all);
        }
        allowxperm(SEPOL_PROC_DOMAIN, SEPOL_PROC_DOMAIN, "tcp_socket", all);
        allowxperm(SEPOL_PROC_DOMAIN, SEPOL_PROC_DOMAIN, "udp_socket", all);
        allowxperm(SEPOL_PROC_DOMAIN, SEPOL_PROC_DOMAIN, "rawip_socket", all);
        // others covered by permissive
    }

    // Let binder IPC work with our processes
    for (auto type: {"servicemanager", "vndservicemanager", "hwservicemanager"}) {
        if (!exists(type))
            continue;
        allow(type, SEPOL_PROC_DOMAIN, "dir", "search");
        allow(type, SEPOL_PROC_DOMAIN, "file", "open");
        allow(type, SEPOL_PROC_DOMAIN, "file", "read");
        allow(type, SEPOL_PROC_DOMAIN, "file", "map");
        allow(type, SEPOL_PROC_DOMAIN, "process", "getattr");
    }
    allow("domain", SEPOL_PROC_DOMAIN, "binder", "call");
    allow("domain", SEPOL_PROC_DOMAIN, "binder", "transfer");

    // other common IPC cases
    allow("domain", SEPOL_PROC_DOMAIN, "process", "sigchld");
    allow("domain", SEPOL_PROC_DOMAIN, "fd", "use");
    allow("domain", SEPOL_PROC_DOMAIN, "fifo_file", "write");
    allow("domain", SEPOL_PROC_DOMAIN, "fifo_file", "read");
    allow("domain", SEPOL_PROC_DOMAIN, "fifo_file", "open");
    allow("domain", SEPOL_PROC_DOMAIN, "fifo_file", "getattr");

    // Allow these processes to access MagiskSU and output logs
    const char *clients[]{
            "zygote", "shell", "system_app", "platform_app",
            "priv_app", "untrusted_app", "untrusted_app_all"
    };
    for (auto type: clients) {
        if (!exists(type))
            continue;
        allow(type, SEPOL_PROC_DOMAIN, "unix_stream_socket", "connectto");
        allow(type, SEPOL_PROC_DOMAIN, "unix_stream_socket", "getopt");
    }

    // Let everyone access tmpfs files (for SAR sbin overlay)
    allow("domain", "tmpfs", "file", ALL);

    // Allow magiskinit daemon to handle mock selinuxfs
    allow("kernel", "tmpfs", "fifo_file", "write");

    // For relabelling files
    allow("rootfs", "labeledfs", "filesystem", "associate");
    allow("rootfs", "tmpfs", "filesystem", "associate");
    allow(SEPOL_FILE_TYPE, "pipefs", "filesystem", "associate");
    allow(SEPOL_FILE_TYPE, "devpts", "filesystem", "associate");

    // Let init transit to SEPOL_PROC_DOMAIN
    allow("kernel", "kernel", "process", "setcurrent");
    allow("kernel", SEPOL_PROC_DOMAIN, "process", "dyntransition");

    // Let init run stuffs
    allow("init", SEPOL_PROC_DOMAIN, "process", ALL);

    // For mounting loop devices, mirrors, tmpfs
    for (int i = 0; i <= 2; i++) {
        allow("kernel", object_attrs[i], "file", "read");
        allow("kernel", object_attrs[i], "file", "write");
    }

    // Zygisk rules
    allow("zygote", "zygote", "process", "execmem");
    allow("zygote", "fs_type", "filesystem", "unmount");
    allow("system_server", "system_server", "process", "execmem");

    // Shut llkd up
    dontaudit("llkd", SEPOL_PROC_DOMAIN, "process", "ptrace");

    // Keep /data/adb/* context
    deny("init", "adb_data_file", "dir", "search");
    deny("vendor_init", "adb_data_file", "dir", "search");

#if 0
    // Remove all dontaudit in debug mode
    impl->strip_dontaudit();
#endif

    set_log_level_state(LogLevel::Warn, true);
}
