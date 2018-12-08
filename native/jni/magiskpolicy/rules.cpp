#include "magisk.h"
#include "magiskpolicy.h"
#include "sepolicy.h"

static void allowSuClient(const char *target) {
	if (!sepol_exists(target))
		return;
	sepol_allow(target, SEPOL_PROC_DOMAIN, "unix_stream_socket", "connectto");
	sepol_allow(target, SEPOL_PROC_DOMAIN, "unix_stream_socket", "getopt");
	sepol_allow(SEPOL_PROC_DOMAIN, target, "fd", "use");
	sepol_allow(SEPOL_PROC_DOMAIN, target, "fifo_file", ALL);

	// Allow binder service
	sepol_allow(target, SEPOL_PROC_DOMAIN, "binder", "call");
	sepol_allow(target, SEPOL_PROC_DOMAIN, "binder", "transfer");

	// Allow termios ioctl
	sepol_allow(target, "devpts", "chr_file", "ioctl");
	sepol_allow(target, "untrusted_app_devpts", "chr_file", "ioctl");
	sepol_allow(target, "untrusted_app_25_devpts", "chr_file", "ioctl");
	sepol_allow(target, "untrusted_app_all_devpts", "chr_file", "ioctl");
	if (policydb->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL) {
		sepol_allowxperm(target, "devpts", "chr_file", "0x5400-0x54FF");
		sepol_allowxperm(target, "untrusted_app_devpts", "chr_file", "0x5400-0x54FF");
		sepol_allowxperm(target, "untrusted_app_25_devpts", "chr_file", "0x5400-0x54FF");
		sepol_allowxperm(target, "untrusted_app_all_devpts", "chr_file", "0x5400-0x54FF");
	}
}

void sepol_magisk_rules() {
	// Temp suppress warnings
	auto bak = log_cb.w;
	log_cb.w = nop_log;

	// First prevent anything to change sepolicy except ourselves
	sepol_deny(ALL, "kernel", "security", "load_policy");

	if (!sepol_exists(SEPOL_PROC_DOMAIN))
		sepol_create(SEPOL_PROC_DOMAIN);
	if (!sepol_exists(SEPOL_FILE_DOMAIN))
		sepol_create(SEPOL_FILE_DOMAIN);
	sepol_permissive(SEPOL_PROC_DOMAIN);

	sepol_attradd(SEPOL_PROC_DOMAIN, "mlstrustedsubject");
	sepol_attradd(SEPOL_PROC_DOMAIN, "netdomain");
	sepol_attradd(SEPOL_PROC_DOMAIN, "bluetoothdomain");
	sepol_attradd(SEPOL_FILE_DOMAIN, "mlstrustedobject");

	// Let init run stuffs
	sepol_allow("kernel", SEPOL_PROC_DOMAIN, "fd", "use");
	sepol_allow("init", SEPOL_PROC_DOMAIN, "process", ALL);

	// Shell, properties, logs
	if (sepol_exists("default_prop"))
		sepol_allow(SEPOL_PROC_DOMAIN, "default_prop", "property_service", "set");
	sepol_allow(SEPOL_PROC_DOMAIN, "init", "unix_stream_socket", "connectto");
	sepol_allow(SEPOL_PROC_DOMAIN, "rootfs", "filesystem", "remount");
	if (sepol_exists("logd"))
		sepol_allow(SEPOL_PROC_DOMAIN, "logd", "unix_stream_socket", "connectto");
	sepol_allow(SEPOL_PROC_DOMAIN, SEPOL_PROC_DOMAIN, ALL, ALL);

	// For sepolicy live patching
	sepol_allow(SEPOL_PROC_DOMAIN, "kernel", "security", "read_policy");
	sepol_allow(SEPOL_PROC_DOMAIN, "kernel", "security", "load_policy");

	// Allow these processes to access MagiskSU
	allowSuClient("init");
	allowSuClient("shell");
	allowSuClient("system_app");
	allowSuClient("priv_app");
	allowSuClient("platform_app");
	allowSuClient("untrusted_app");
	allowSuClient("untrusted_app_25");
	allowSuClient("untrusted_app_27");
	allowSuClient("update_engine");

	// suRights
	sepol_allow("servicemanager", SEPOL_PROC_DOMAIN, "dir", "search");
	sepol_allow("servicemanager", SEPOL_PROC_DOMAIN, "dir", "read");
	sepol_allow("servicemanager", SEPOL_PROC_DOMAIN, "file", "open");
	sepol_allow("servicemanager", SEPOL_PROC_DOMAIN, "file", "read");
	sepol_allow("servicemanager", SEPOL_PROC_DOMAIN, "process", "getattr");
	sepol_allow("servicemanager", SEPOL_PROC_DOMAIN, "binder", "transfer");
	sepol_allow(SEPOL_PROC_DOMAIN, "servicemanager", "dir", "search");
	sepol_allow(SEPOL_PROC_DOMAIN, "servicemanager", "dir", "read");
	sepol_allow(SEPOL_PROC_DOMAIN, "servicemanager", "file", "open");
	sepol_allow(SEPOL_PROC_DOMAIN, "servicemanager", "file", "read");
	sepol_allow(SEPOL_PROC_DOMAIN, "servicemanager", "process", "getattr");
	sepol_allow(SEPOL_PROC_DOMAIN, "servicemanager", "binder", "transfer");
	sepol_allow(SEPOL_PROC_DOMAIN, "servicemanager", "binder", "call");
	sepol_allow(ALL, SEPOL_PROC_DOMAIN, "process", "sigchld");

	// allowLog
	sepol_allow("logd", SEPOL_PROC_DOMAIN, "dir", "search");
	sepol_allow("logd", SEPOL_PROC_DOMAIN, "file", "read");
	sepol_allow("logd", SEPOL_PROC_DOMAIN, "file", "open");
	sepol_allow("logd", SEPOL_PROC_DOMAIN, "file", "getattr");

	// suBackL0
	sepol_allow("system_server", SEPOL_PROC_DOMAIN, "binder", "call");
	sepol_allow("system_server", SEPOL_PROC_DOMAIN, "binder", "transfer");
	sepol_allow(SEPOL_PROC_DOMAIN, "system_server", "binder", "call");
	sepol_allow(SEPOL_PROC_DOMAIN, "system_server", "binder", "transfer");

	// suBackL6
	sepol_allow("surfaceflinger", "app_data_file", "dir", ALL);
	sepol_allow("surfaceflinger", "app_data_file", "file", ALL);
	sepol_allow("surfaceflinger", "app_data_file", "lnk_file", ALL);
	sepol_attradd("surfaceflinger", "mlstrustedsubject");

	// suMiscL6
	if (sepol_exists("audioserver"))
		sepol_allow("audioserver", "audioserver", "process", "execmem");

	// Liveboot
	sepol_allow("surfaceflinger", SEPOL_PROC_DOMAIN, "process", "ptrace");
	sepol_allow("surfaceflinger", SEPOL_PROC_DOMAIN, "binder", "transfer");
	sepol_allow("surfaceflinger", SEPOL_PROC_DOMAIN, "binder", "call");
	sepol_allow("surfaceflinger", SEPOL_PROC_DOMAIN, "fd", "use");
	sepol_allow("debuggerd", SEPOL_PROC_DOMAIN, "process", "ptrace");

	// dumpsys
	sepol_allow(ALL, SEPOL_PROC_DOMAIN, "fd", "use");
	sepol_allow(ALL, SEPOL_PROC_DOMAIN, "fifo_file", "write");
	sepol_allow(ALL, SEPOL_PROC_DOMAIN, "fifo_file", "read");
	sepol_allow(ALL, SEPOL_PROC_DOMAIN, "fifo_file", "open");
	sepol_allow(ALL, SEPOL_PROC_DOMAIN, "fifo_file", "getattr");

	// bootctl
	sepol_allow("hwservicemanager", SEPOL_PROC_DOMAIN, "dir", "search");
	sepol_allow("hwservicemanager", SEPOL_PROC_DOMAIN, "file", "read");
	sepol_allow("hwservicemanager", SEPOL_PROC_DOMAIN, "file", "open");
	sepol_allow("hwservicemanager", SEPOL_PROC_DOMAIN, "process", "getattr");
	sepol_allow("hwservicemanager", SEPOL_PROC_DOMAIN, "binder", "transfer");

	// For mounting loop devices, mirrors, tmpfs
	sepol_allow(SEPOL_PROC_DOMAIN, "kernel", "process", "setsched");
	sepol_allow(SEPOL_PROC_DOMAIN, "labeledfs", "filesystem", "mount");
	sepol_allow(SEPOL_PROC_DOMAIN, "labeledfs", "filesystem", "unmount");
	sepol_allow(SEPOL_PROC_DOMAIN, "tmpfs", "filesystem", "mount");
	sepol_allow(SEPOL_PROC_DOMAIN, "tmpfs", "filesystem", "unmount");
	sepol_allow("kernel", ALL, "file", "read");

	// Allow su to do anything to any files/dir/links
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, "file", ALL);
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, "dir", ALL);
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, "lnk_file", ALL);
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, "blk_file", ALL);
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, "sock_file", ALL);
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, "chr_file", ALL);
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, "fifo_file", ALL);

	// Super files
	sepol_allow(ALL, SEPOL_FILE_DOMAIN, "file", ALL);
	sepol_allow(ALL, SEPOL_FILE_DOMAIN, "dir", ALL);
	sepol_allow(ALL, SEPOL_FILE_DOMAIN, "fifo_file", ALL);
	sepol_allow(ALL, SEPOL_FILE_DOMAIN, "chr_file", ALL);
	sepol_allow(SEPOL_FILE_DOMAIN, ALL, "filesystem", "associate");

	// For changing attributes
	sepol_allow("rootfs", "tmpfs", "filesystem", "associate");

	// Xposed
	sepol_allow("untrusted_app", "untrusted_app", "capability", "setgid");
	sepol_allow("system_server", "dex2oat_exec", "file", ALL);

	// Support deodexed ROM on Oreo
	sepol_allow("zygote", "dalvikcache_data_file", "file", "execute");

	// Allow update engine to source addon.d.sh
	sepol_allow("update_engine", "adb_data_file", "dir", ALL);

	// Remove all dontaudit
	avtab_ptr_t av;
	avtab_for_each(&policydb->te_avtab, av, {
		if (av->key.specified == AVTAB_AUDITDENY || av->key.specified == AVTAB_XPERMS_DONTAUDIT)
			avtab_remove_node(&policydb->te_avtab, av);
	})

	log_cb.w = bak;
}
