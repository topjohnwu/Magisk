#include <consts.hpp>
#include <base.hpp>

#include "policy.hpp"

using namespace std;

void sepolicy::magisk_rules() {
    // Temp suppress warnings
    set_log_level_state(LogLevel::Warn, false);

    type(SEPOL_PROC_DOMAIN, "domain");
    permissive(SEPOL_PROC_DOMAIN);  /* Just in case something is missing */

    typeattribute(SEPOL_PROC_DOMAIN, "mlstrustedsubject");
    typeattribute(SEPOL_PROC_DOMAIN, "netdomain");
    typeattribute(SEPOL_PROC_DOMAIN, "bluetoothdomain");

    type(SEPOL_FILE_TYPE, "file_type");
    typeattribute(SEPOL_FILE_TYPE, "mlstrustedobject");

    allow(SEPOL_PROC_DOMAIN, ALL, ALL, ALL);

    // Allow us to do any ioctl
    if (impl->db->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL) {
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "blk_file", ALL_XPERM);
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "fifo_file", ALL_XPERM);
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "chr_file", ALL_XPERM);
        allowxperm(SEPOL_PROC_DOMAIN, ALL, "file", ALL_XPERM);
    }

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

#if 0
    // Remove all dontaudit in debug mode
    impl->strip_dontaudit();
#endif

    set_log_level_state(LogLevel::Warn, true);
}
