#include <logging.hpp>
#include <flags.h>
#include <magiskpolicy.hpp>

#include "sepolicy.h"

void sepolicy::allow_su_client(const char *type) {
	if (!exists(type))
		return;
	allow(type, SEPOL_PROC_DOMAIN, "unix_stream_socket", "connectto");
	allow(type, SEPOL_PROC_DOMAIN, "unix_stream_socket", "getopt");
	allow(SEPOL_PROC_DOMAIN, type, "fd", "use");
	allow(SEPOL_PROC_DOMAIN, type, "fifo_file", ALL);

	// Allow binder service
	allow(type, SEPOL_PROC_DOMAIN, "binder", "call");
	allow(type, SEPOL_PROC_DOMAIN, "binder", "transfer");

	// Allow termios ioctl
	allow(type, "devpts", "chr_file", "ioctl");
	allow(type, "untrusted_app_devpts", "chr_file", "ioctl");
	allow(type, "untrusted_app_25_devpts", "chr_file", "ioctl");
	allow(type, "untrusted_app_all_devpts", "chr_file", "ioctl");
	if (db->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL) {
		allowxperm(type, "devpts", "chr_file", "0x5400-0x54FF");
		allowxperm(type, "untrusted_app_devpts", "chr_file", "0x5400-0x54FF");
		allowxperm(type, "untrusted_app_25_devpts", "chr_file", "0x5400-0x54FF");
		allowxperm(type, "untrusted_app_all_devpts", "chr_file", "0x5400-0x54FF");
	}
}

void sepolicy::magisk_rules() {
	// Temp suppress warnings
	auto bak = log_cb.w;
	log_cb.w = nop_log;

	// First prevent anything to change sepolicy except ourselves
	deny(ALL, "kernel", "security", "load_policy");

	if (!exists(SEPOL_PROC_DOMAIN))
		create(SEPOL_PROC_DOMAIN);
	if (!exists(SEPOL_FILE_DOMAIN))
		create(SEPOL_FILE_DOMAIN);
	permissive(SEPOL_PROC_DOMAIN);

	typeattribute(SEPOL_PROC_DOMAIN, "mlstrustedsubject");
	typeattribute(SEPOL_PROC_DOMAIN, "netdomain");
	typeattribute(SEPOL_PROC_DOMAIN, "bluetoothdomain");
	typeattribute(SEPOL_FILE_DOMAIN, "mlstrustedobject");

	// Let everyone access tmpfs files (for SAR sbin overlay)
	allow(ALL, "tmpfs", "file", ALL);

	// For normal rootfs file/directory operations when rw (for SAR / overlay)
	allow("rootfs", "labeledfs", "filesystem", "associate");

	// Let init transit to SEPOL_PROC_DOMAIN
	allow("kernel", "kernel", "process", "setcurrent");
	allow("kernel", SEPOL_PROC_DOMAIN, "process", "dyntransition");

	// Let init run stuffs
	allow("kernel", SEPOL_PROC_DOMAIN, "fd", "use");
	allow("init", SEPOL_PROC_DOMAIN, "process", ALL);
	allow("init", "tmpfs", "file", "getattr");
	allow("init", "tmpfs", "file", "execute");

	// Shell, properties, logs
	if (exists("default_prop"))
		allow(SEPOL_PROC_DOMAIN, "default_prop", "property_service", "set");
	allow(SEPOL_PROC_DOMAIN, "init", "unix_stream_socket", "connectto");
	allow(SEPOL_PROC_DOMAIN, "rootfs", "filesystem", "remount");
	if (exists("logd"))
		allow(SEPOL_PROC_DOMAIN, "logd", "unix_stream_socket", "connectto");
	allow(SEPOL_PROC_DOMAIN, SEPOL_PROC_DOMAIN, ALL, ALL);

	// For sepolicy live patching
	allow(SEPOL_PROC_DOMAIN, "kernel", "security", "read_policy");
	allow(SEPOL_PROC_DOMAIN, "kernel", "security", "load_policy");

	// Allow these processes to access MagiskSU
	allow_su_client("init");
	allow_su_client("shell");
	allow_su_client("system_app");
	allow_su_client("priv_app");
	allow_su_client("platform_app");
	allow_su_client("untrusted_app");
	allow_su_client("untrusted_app_25");
	allow_su_client("untrusted_app_27");
	allow_su_client("untrusted_app_29");
	allow_su_client("update_engine");

	// suRights
	allow("servicemanager", SEPOL_PROC_DOMAIN, "dir", "search");
	allow("servicemanager", SEPOL_PROC_DOMAIN, "dir", "read");
	allow("servicemanager", SEPOL_PROC_DOMAIN, "file", "open");
	allow("servicemanager", SEPOL_PROC_DOMAIN, "file", "read");
	allow("servicemanager", SEPOL_PROC_DOMAIN, "process", "getattr");
	allow("servicemanager", SEPOL_PROC_DOMAIN, "binder", "transfer");
	allow(SEPOL_PROC_DOMAIN, "servicemanager", "dir", "search");
	allow(SEPOL_PROC_DOMAIN, "servicemanager", "dir", "read");
	allow(SEPOL_PROC_DOMAIN, "servicemanager", "file", "open");
	allow(SEPOL_PROC_DOMAIN, "servicemanager", "file", "read");
	allow(SEPOL_PROC_DOMAIN, "servicemanager", "process", "getattr");
	allow(SEPOL_PROC_DOMAIN, "servicemanager", "binder", "transfer");
	allow(SEPOL_PROC_DOMAIN, "servicemanager", "binder", "call");
	allow(ALL, SEPOL_PROC_DOMAIN, "process", "sigchld");

	// allowLog
	allow("logd", SEPOL_PROC_DOMAIN, "dir", "search");
	allow("logd", SEPOL_PROC_DOMAIN, "file", "read");
	allow("logd", SEPOL_PROC_DOMAIN, "file", "open");
	allow("logd", SEPOL_PROC_DOMAIN, "file", "getattr");

	// suBackL0
	allow("system_server", SEPOL_PROC_DOMAIN, "binder", "call");
	allow("system_server", SEPOL_PROC_DOMAIN, "binder", "transfer");
	allow(SEPOL_PROC_DOMAIN, "system_server", "binder", "call");
	allow(SEPOL_PROC_DOMAIN, "system_server", "binder", "transfer");

	// suBackL6
	allow("surfaceflinger", "app_data_file", "dir", ALL);
	allow("surfaceflinger", "app_data_file", "file", ALL);
	allow("surfaceflinger", "app_data_file", "lnk_file", ALL);
	typeattribute("surfaceflinger", "mlstrustedsubject");

	// suMiscL6
	if (exists("audioserver"))
		allow("audioserver", "audioserver", "process", "execmem");

	// Liveboot
	allow("surfaceflinger", SEPOL_PROC_DOMAIN, "process", "ptrace");
	allow("surfaceflinger", SEPOL_PROC_DOMAIN, "binder", "transfer");
	allow("surfaceflinger", SEPOL_PROC_DOMAIN, "binder", "call");
	allow("surfaceflinger", SEPOL_PROC_DOMAIN, "fd", "use");
	allow("debuggerd", SEPOL_PROC_DOMAIN, "process", "ptrace");

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
	allow("hwservicemanager", SEPOL_PROC_DOMAIN, "binder", "transfer");

	// For mounting loop devices, mirrors, tmpfs
	allow(SEPOL_PROC_DOMAIN, "kernel", "process", "setsched");
	allow(SEPOL_PROC_DOMAIN, "labeledfs", "filesystem", "mount");
	allow(SEPOL_PROC_DOMAIN, "labeledfs", "filesystem", "unmount");
	allow(SEPOL_PROC_DOMAIN, "tmpfs", "filesystem", "mount");
	allow(SEPOL_PROC_DOMAIN, "tmpfs", "filesystem", "unmount");
	allow("kernel", ALL, "file", "read");
	allow("kernel", ALL, "file", "write");

	// Allow us to do anything to any files/dir/links
	allow(SEPOL_PROC_DOMAIN, ALL, "file", ALL);
	allow(SEPOL_PROC_DOMAIN, ALL, "dir", ALL);
	allow(SEPOL_PROC_DOMAIN, ALL, "lnk_file", ALL);
	allow(SEPOL_PROC_DOMAIN, ALL, "blk_file", ALL);
	allow(SEPOL_PROC_DOMAIN, ALL, "sock_file", ALL);
	allow(SEPOL_PROC_DOMAIN, ALL, "chr_file", ALL);
	allow(SEPOL_PROC_DOMAIN, ALL, "fifo_file", ALL);

	// Allow us to do any ioctl on all block devices
	if (db->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL)
		allowxperm(SEPOL_PROC_DOMAIN, ALL, "blk_file", "0x0000-0xFFFF");

	// Allow all binder transactions
	allow(ALL, SEPOL_PROC_DOMAIN, "binder", ALL);

	// Super files
	allow(ALL, SEPOL_FILE_DOMAIN, "file", ALL);
	allow(ALL, SEPOL_FILE_DOMAIN, "dir", ALL);
	allow(ALL, SEPOL_FILE_DOMAIN, "fifo_file", ALL);
	allow(ALL, SEPOL_FILE_DOMAIN, "chr_file", ALL);
	allow(SEPOL_FILE_DOMAIN, ALL, "filesystem", "associate");

	// For changing attributes
	allow("rootfs", "tmpfs", "filesystem", "associate");

	// Xposed
	allow("untrusted_app", "untrusted_app", "capability", "setgid");
	allow("system_server", "dex2oat_exec", "file", ALL);

	// Support deodexed ROM on Oreo
	allow("zygote", "dalvikcache_data_file", "file", "execute");

	// Support deodexed ROM on Pie (Samsung)
	allow("system_server", "dalvikcache_data_file", "file", "write");
	allow("system_server", "dalvikcache_data_file", "file", "execute");

	// Allow update_engine/addon.d-v2 to run permissive on all ROMs
	permissive("update_engine");

#if 0
	// Remove all dontaudit in debug mode
	strip_dontaudit(db);
#endif

	log_cb.w = bak;
}
