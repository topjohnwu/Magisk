#include "magiskpolicy.h"
#include "sepolicy.h"

void allowSuClient(char *target) {
	sepol_allow(target, "rootfs", "file", ALL);
	sepol_allow(target, "rootfs", "lnk_file", ALL);
	sepol_allow(target, "su", "unix_stream_socket", "connectto");
	sepol_allow(target, "su", "unix_stream_socket", "getopt");
	sepol_allow(target, "su_file", "dir", "search");
	sepol_allow(target, "su_file", "dir", "read");
	sepol_allow(target, "su_file", "sock_file", "read");
	sepol_allow(target, "su_file", "sock_file", "write");
	sepol_allow(target, "su_file", "file", ALL);
	sepol_allow(target, "devpts", "chr_file", "ioctl");
	sepol_allow("su", target, "fd", "use");
	sepol_allow("su", target, "fifo_file", ALL);
}

void suRights() {
	sepol_allow("servicemanager", "su", "dir", "search");
	sepol_allow("servicemanager", "su", "dir", "read");
	sepol_allow("servicemanager", "su", "file", "open");
	sepol_allow("servicemanager", "su", "file", "read");
	sepol_allow("servicemanager", "su", "process", "getattr");
	sepol_allow("servicemanager", "su", "binder", "transfer");
	sepol_allow("system_server", "su", "binder", "call");
	sepol_allow("system_server", "su", "fd", "use");

	sepol_allow("su", "servicemanager", "dir", "search");
	sepol_allow("su", "servicemanager", "dir", "read");
	sepol_allow("su", "servicemanager", "file", "open");
	sepol_allow("su", "servicemanager", "file", "read");
	sepol_allow("su", "servicemanager", "process", "getattr");
	sepol_allow("su", "servicemanager", "binder", "transfer");
	sepol_allow("su", "servicemanager", "binder", "call");
	sepol_allow("su", "system_server", "binder", "transfer");
	sepol_allow("su", "system_server", "binder", "call");
}

void otherToSU() {
	// allowLog
	sepol_allow("logd", "su", "dir", "search");
	sepol_allow("logd", "su", "file", "read");
	sepol_allow("logd", "su", "file", "open");
	sepol_allow("logd", "su", "file", "getattr");

	// suBackL0
	sepol_allow("system_server", "su", "binder", "call");
	sepol_allow("system_server", "su", "binder", "transfer");

	// ES Explorer opens a sokcet
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "ioctl");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "read");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "getattr");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "write");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "setattr");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "lock");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "append");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "bind");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "connect");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "getopt");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "setopt");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "shutdown");
	sepol_allow("untrusted_app", "su", "unix_stream_socket", "connectto");

	// Any domain is allowed to send su "sigchld"
	sepol_allow(ALL, "su", "process", "sigchld");

	// uNetworkL0
	sepol_attradd("su", "netdomain");
	sepol_attradd("su", "bluetoothdomain");

	// suBackL6
	sepol_allow("surfaceflinger", "app_data_file", "dir", ALL);
	sepol_allow("surfaceflinger", "app_data_file", "file", ALL);
	sepol_allow("surfaceflinger", "app_data_file", "lnk_file", ALL);
	sepol_attradd("surfaceflinger", "mlstrustedsubject");

	// suMiscL6
	if (sepol_exists("audioserver"))
		sepol_allow("audioserver", "audioserver", "process", "execmem");

	// Liveboot
	sepol_allow("surfaceflinger", "su", "process", "ptrace");
	sepol_allow("surfaceflinger", "su", "binder", "transfer");
	sepol_allow("surfaceflinger", "su", "binder", "call");
	sepol_allow("surfaceflinger", "su", "fd", "use");
	sepol_allow("debuggerd", "su", "process", "ptrace");

	// dumpsys
	sepol_allow(ALL, "su", "fd", "use");
	sepol_allow(ALL, "su", "fifo_file", "write");
	sepol_allow(ALL, "su", "fifo_file", "read");
	sepol_allow(ALL, "su", "fifo_file", "open");
	sepol_allow(ALL, "su", "fifo_file", "getattr");
}

void sepol_magisk_rules() {
	// First prevent anything to change sepolicy except ourselves
	sepol_deny(ALL, "kernel", "security", "load_policy");

	if (!sepol_exists("su"))
		sepol_create("su");
	if (!sepol_exists("su_file"))
		sepol_create("su_file");
	sepol_permissive("su");

	sepol_attradd("su", "mlstrustedsubject");
	sepol_attradd("su_file", "mlstrustedobject");

	// Allow magiskinit daemon to run and run in su context
	sepol_allow("kernel", "device", "dir", ALL);
	sepol_allow("kernel", "device", "file", ALL);
	sepol_allow("kernel", "su", "unix_stream_socket", "connectto");
	sepol_allow("kernel", "kernel", "process", "setcurrent");
	sepol_allow("kernel", "su", "process", "dyntransition");

	// Let init run stuffs
	sepol_allow("kernel", "su", "fd", "use");
	sepol_allow("init", "su", "process", ALL);
	sepol_allow("init", "system_file", "dir", ALL);
	sepol_allow("init", "system_file", "lnk_file", ALL);
	sepol_allow("init", "system_file", "file", ALL);
	sepol_allow("init", "rootfs", "lnk_file", ALL);

	// Shell, properties, logs
	if (sepol_exists("default_prop"))
		sepol_allow("su", "default_prop", "property_service", "set");
	sepol_allow("su", "init", "unix_stream_socket", "connectto");
	sepol_allow("su", "rootfs", "filesystem", "remount");
	if (sepol_exists("logd"))
		sepol_allow("su", "logd", "unix_stream_socket", "connectto");
	sepol_allow("su", "su", ALL, ALL);

	// For sepolicy live patching
	sepol_allow("su", "kernel", "security", "read_policy");
	sepol_allow("su", "kernel", "security", "load_policy");

	// Allow these client to access su
	allowSuClient("shell");
	allowSuClient("untrusted_app");
	allowSuClient("system_app");
	allowSuClient("platform_app");
	if (sepol_exists("priv_app"))
		allowSuClient("priv_app");
	if (sepol_exists("untrusted_app_25"))
		allowSuClient("untrusted_app_25");

	// Some superuser stuffs
	suRights();
	otherToSU();

	// For mounting loop devices, mirrors, tmpfs
	sepol_allow("su", "kernel", "process", "setsched");
	sepol_allow("su", "labeledfs", "filesystem", "mount");
	sepol_allow("su", "labeledfs", "filesystem", "unmount");
	sepol_allow("su", "tmpfs", "filesystem", "mount");
	sepol_allow("su", "tmpfs", "filesystem", "unmount");
	sepol_allow("kernel", ALL, "file", "read");

	// Allow su to do anything to any files/dir/links
	sepol_allow("su", ALL, "file", ALL);
	sepol_allow("su", ALL, "dir", ALL);
	sepol_allow("su", ALL, "lnk_file", ALL);
	sepol_allow("su", ALL, "blk_file", ALL);
	sepol_allow("su", ALL, "sock_file", ALL);
	sepol_allow("su", ALL, "chr_file", ALL);
	sepol_allow("su", ALL, "fifo_file", ALL);

	// For changing attributes
	sepol_allow("rootfs", "tmpfs", "filesystem", "associate");
	sepol_allow("su_file", "labeledfs", "filesystem", "associate");
	sepol_allow("su_file", "tmpfs", "filesystem", "associate");

	// Xposed
	sepol_allow("untrusted_app", "untrusted_app", "capability", "setgid");
	sepol_allow("system_server", "dex2oat_exec", "file", ALL);

	// xperms
	if (policydb->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL) {
		sepol_allowxperm("domain", "devpts", "chr_file", "0x5400-0x54FF");
		if (sepol_exists("untrusted_app_25_devpts"))
			sepol_allowxperm("domain", "untrusted_app_25_devpts", "chr_file", "0x5400-0x54FF");
		if (sepol_exists("untrusted_app_devpts"))
			sepol_allowxperm("domain", "untrusted_app_devpts", "chr_file", "0x5400-0x54FF");
	}
}

