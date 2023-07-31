#include <base.hpp>

#include "policy.hpp"

using namespace std;

void sepolicy::magisk_rules() {
    // Temp suppress warnings
    set_log_level_state(LogLevel::Warn, false);

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
    if (impl->db->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL) {
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "blk_file", ALL_XPERM);
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "fifo_file", ALL_XPERM);
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "chr_file", ALL_XPERM);
    }

    // Create unconstrained file type
    allow(ALL, SEPOL_FILE_TYPE, "file", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "dir", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "fifo_file", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "chr_file", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "lnk_file", ALL);
    allow(ALL, SEPOL_FILE_TYPE, "sock_file", ALL);

    // Allow these processes to access MagiskSU
    const char *clients[]{"zygote", "shell",
                          "system_app", "platform_app", "priv_app",
                          "untrusted_app", "untrusted_app_all"};
    for (auto type: clients) {
        if (!exists(type))
            continue;
        allow(type, SEPOL_PROC_DOMAIN, "unix_stream_socket", "connectto");
        allow(type, SEPOL_PROC_DOMAIN, "unix_stream_socket", "getopt");
    }

    // Let everyone access tmpfs files (for SAR sbin overlay)
    allow(ALL, "tmpfs", "file", ALL);

    // Allow magiskinit daemon to handle mock selinuxfs
    allow("kernel", "tmpfs", "fifo_file", "write");

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

    // Keep /data/adb/* context
    deny("init", "adb_data_file", "dir", "search");
    deny("vendor_init", "adb_data_file", "dir", "search");

#if 0
    // Remove all dontaudit in debug mode
    impl->strip_dontaudit();
#endif

    set_log_level_state(LogLevel::Warn, true);
}
