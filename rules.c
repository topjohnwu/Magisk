#include "magiskpolicy.h"
#include "sepolicy.h"

void samsung() {
	sepol_deny("init", "kernel", "security", "load_policy");
	sepol_deny("policyloader_app", "security_spota_file", "dir", "read");
	sepol_deny("policyloader_app", "security_spota_file", "dir", "write");
	sepol_deny("policyloader_app", "security_spota_file", "file", "read");
	sepol_deny("policyloader_app", "security_spota_file", "file", "write");
	sepol_deny("system_server", "security_spota_file", "dir", "read");
	sepol_deny("system_server", "security_spota_file", "dir", "write");
	sepol_deny("system_server", "security_spota_file", "file", "read");
	sepol_deny("system_server", "security_spota_file", "file", "write");
	sepol_deny("system_app", "security_spota_file", "dir", "read");
	sepol_deny("system_app", "security_spota_file", "dir", "write");
	sepol_deny("system_app", "security_spota_file", "file", "read");
	sepol_deny("system_app", "security_spota_file", "file", "write");
	sepol_deny("installd", "security_spota_file", "dir", "read");
	sepol_deny("installd", "security_spota_file", "dir", "write");
	sepol_deny("installd", "security_spota_file", "file", "read");
	sepol_deny("installd", "security_spota_file", "file", "write");
	sepol_deny("init", "security_spota_file", "dir", "read");
	sepol_deny("init", "security_spota_file", "dir", "write");
	sepol_deny("init", "security_spota_file", "file", "read");
	sepol_deny("init", "security_spota_file", "file", "write");
	sepol_deny("ueventd", "security_spota_file", "dir", "read");
	sepol_deny("ueventd", "security_spota_file", "dir", "write");
	sepol_deny("ueventd", "security_spota_file", "file", "read");
	sepol_deny("ueventd", "security_spota_file", "file", "write");
	sepol_deny("runas", "security_spota_file", "dir", "read");
	sepol_deny("runas", "security_spota_file", "dir", "write");
	sepol_deny("runas", "security_spota_file", "file", "read");
	sepol_deny("runas", "security_spota_file", "file", "write");
	sepol_deny("drsd", "security_spota_file", "dir", "read");
	sepol_deny("drsd", "security_spota_file", "dir", "write");
	sepol_deny("drsd", "security_spota_file", "file", "read");
	sepol_deny("drsd", "security_spota_file", "file", "write");
	sepol_deny("debuggerd", "security_spota_file", "dir", "read");
	sepol_deny("debuggerd", "security_spota_file", "dir", "write");
	sepol_deny("debuggerd", "security_spota_file", "file", "read");
	sepol_deny("debuggerd", "security_spota_file", "file", "write");
	sepol_deny("vold", "security_spota_file", "dir", "read");
	sepol_deny("vold", "security_spota_file", "dir", "write");
	sepol_deny("vold", "security_spota_file", "file", "read");
	sepol_deny("vold", "security_spota_file", "file", "write");
	sepol_deny("zygote", "security_spota_file", "dir", "read");
	sepol_deny("zygote", "security_spota_file", "dir", "write");
	sepol_deny("zygote", "security_spota_file", "file", "read");
	sepol_deny("zygote", "security_spota_file", "file", "write");
	sepol_deny("auditd", "security_spota_file", "dir", "read");
	sepol_deny("auditd", "security_spota_file", "dir", "write");
	sepol_deny("auditd", "security_spota_file", "file", "read");
	sepol_deny("auditd", "security_spota_file", "file", "write");
	sepol_deny("servicemanager", "security_spota_file", "dir", "read");
	sepol_deny("servicemanager", "security_spota_file", "dir", "write");
	sepol_deny("servicemanager", "security_spota_file", "file", "read");
	sepol_deny("servicemanager", "security_spota_file", "file", "write");
	sepol_deny("itsonbs", "security_spota_file", "dir", "read");
	sepol_deny("itsonbs", "security_spota_file", "dir", "write");
	sepol_deny("itsonbs", "security_spota_file", "file", "read");
	sepol_deny("itsonbs", "security_spota_file", "file", "write");
	sepol_deny("commonplatformappdomain", "security_spota_file", "dir", "read");
	sepol_deny("commonplatformappdomain", "security_spota_file", "dir", "write");
	sepol_deny("commonplatformappdomain", "security_spota_file", "file", "read");
	sepol_deny("commonplatformappdomain", "security_spota_file", "file", "write");
}

void allowSuClient(char *target) {
	sepol_allow(target, "rootfs", "file", ALL);
	sepol_allow(target, "rootfs", "lnk_file", ALL);
	sepol_allow(target, "su", "unix_stream_socket", "connectto");
	sepol_allow(target, "su", "unix_stream_socket", "getopt");
	sepol_allow(target, "su_device", "dir", "search");
	sepol_allow(target, "su_device", "dir", "read");
	sepol_allow(target, "su_device", "sock_file", "read");
	sepol_allow(target, "su_device", "sock_file", "write");
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
}

// Minimal boot image patch, Samsung requires these patches
void sepol_min_rules() {
	if (!sepol_exists("su"))
		sepol_create("su");
	if (!sepol_exists("su_device"))
		sepol_create("su_device");
	sepol_permissive("su");
	sepol_permissive("init");

	sepol_attradd("su", "mlstrustedsubject");
	sepol_attradd("su_device", "mlstrustedobject");

	// Let init run stuffs in su context
	sepol_allow("kernel", "su", "fd", "use");
	sepol_allow("init", "su", "process", ALL);
	sepol_allow("init", "system_file", "dir", ALL);
	sepol_allow("init", "system_file", "lnk_file", ALL);
	sepol_allow("init", "system_file", "file", ALL);

	// Samsung specific
	// Prevent system from loading policy
	if(sepol_exists("knox_system_app"))
		samsung();

	// Shell, prop management, simple su rights, logs
	sepol_allow("su", "property_socket", "sock_file", "write");
	sepol_allow("su", "properties_device", "file", "write");
	if (sepol_exists("default_prop")) {
		sepol_allow("su", "default_prop", "property_service", "set");
		sepol_allow("su", "default_prop", "file", "write");
	}
	if (sepol_exists("properties_serial"))
		sepol_allow("su", "properties_serial", "file", "write");
	sepol_allow("su", "init", "unix_stream_socket", "connectto");
	sepol_allow("su", "rootfs", "file", "entrypoint");
	sepol_allow("su", "rootfs", "file", "getattr");
	sepol_allow("su", "rootfs", "file", "read");
	sepol_allow("su", "rootfs", "filesystem", "remount");
	sepol_allow("su", "devpts", "chr_file", ALL);
	sepol_allow("su", "untrusted_app_devpts", "chr_file", ALL);
	sepol_allow("su", "su_device", "dir", ALL);
	sepol_allow("su", "su_device", "sock_file", ALL);
	sepol_allow("su", "zygote_exec", "file", ALL);
	sepol_allow("su", "zygote_exec", "lnk_file", ALL);
	sepol_allow("su", "app_data_file", "dir", ALL);
	sepol_allow("su", "app_data_file", "file", ALL);
	sepol_allow("su", "shell_exec", "file", ALL);
	sepol_allow("su", "su", "unix_dgram_socket", ALL);
	sepol_allow("su", "su", "unix_stream_socket", ALL);
	sepol_allow("su", "su", "process", ALL);
	sepol_allow("su", "su", "capability", ALL);
	sepol_allow("su", "su", "file", ALL);
	sepol_allow("su", "su", "fifo_file", ALL);
	sepol_allow("su", "su", "lnk_file", ALL);
	sepol_allow("su", "su", "dir", ALL);
	if (sepol_exists("toolbox_exec"))
		sepol_allow("su", "toolbox_exec", "file", ALL);
	if (sepol_exists("logdr_socket"))
		sepol_allow("su", "logdr_socket", "sock_file", "write");
	if (sepol_exists("logd"))
		sepol_allow("su", "logd", "unix_stream_socket", "connectto");
	sepol_allow("su_device", "tmpfs", "filesystem", "associate");

	// For sepolicy live patching
	sepol_allow("su", "kernel", "security", "read_policy");
	sepol_allow("su", "kernel", "security", "load_policy");
	sepol_allow("su", "selinuxfs", "file", "read");
	sepol_allow("su", "selinuxfs", "file", "open");
	sepol_allow("su", "selinuxfs", "file", "write");

	// For mounting loop devices and mirrors
	sepol_allow("su", "kernel", "process", "setsched");
	sepol_allow("su", "labeledfs", "filesystem", "mount");
	sepol_allow("su", "labeledfs", "filesystem", "unmount");
	sepol_allow("kernel", "system_data_file", "file", "read");

	// Xposed
	sepol_allow("untrusted_app", "untrusted_app", "capability", "setgid");
	sepol_allow("system_server", "dex2oat_exec", "file", ALL);

	// xperms
	if (policydb->policyvers >= POLICYDB_VERSION_XPERMS_IOCTL)
		sepol_allowxperm("domain", "devpts", "chr_file", "0x5400-0x54FF");
}

void sepol_med_rules() {
	sepol_min_rules();

	// Allow su to do anything to any files/dir/links
	sepol_allow("su", ALL, "file", ALL);
	sepol_allow("su", ALL, "dir", ALL);
	sepol_allow("su", ALL, "lnk_file", ALL);
	sepol_allow("su", ALL, "blk_file", ALL);

	// Allow these client to access su
	allowSuClient("shell");
	allowSuClient("untrusted_app");
	allowSuClient("system_app");
	allowSuClient("platform_app");
	if (sepol_exists("priv_app"))
		allowSuClient("priv_app");
	if (sepol_exists("ssd_tool"))
		allowSuClient("ssd_tool");
	if (sepol_exists("untrusted_app_25"))
		allowSuClient("untrusted_app_25");

	suRights();
	otherToSU();
}

void sepol_full_rules() {
	// Patch su to everything
	sepol_allow("su", ALL, ALL, ALL);
}

