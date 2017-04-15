#include "magiskpolicy.h"

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
	sepol_allow(target, "rootfs", "file", "execute_no_trans");
	sepol_allow(target, "rootfs", "file", "execute");
	sepol_allow(target, "su", "unix_stream_socket", "connectto");
	sepol_allow(target, "su", "unix_stream_socket", "getopt");
	sepol_allow(target, "su_device", "dir", "search");
	sepol_allow(target, "su_device", "dir", "read");
	sepol_allow(target, "su_device", "sock_file", "read");
	sepol_allow(target, "su_device", "sock_file", "write");
}

void suRights() {
	sepol_allow("servicemanager", "su", "dir", "search");
	sepol_allow("servicemanager", "su", "dir", "read");
	sepol_allow("servicemanager", "su", "file", "open");
	sepol_allow("servicemanager", "su", "file", "read");
	sepol_allow("servicemanager", "su", "process", "getattr");
	sepol_allow("servicemanager", "su", "binder", "transfer");
	sepol_allow("system_server", "su", "binder", "call");
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
}

void sepol_full_rules() {
	// Samsung specific
	// Prevent system from loading policy
	if(sepol_exists("knox_system_app"))
		samsung();

	// Min rules first
	sepol_min_rules();

	// Create domains if they don't exist
	if (!sepol_exists("su_device"))
		sepol_create("su_device");
	sepol_enforce("su_device");

	// Patch su to everything
	sepol_allow("su", ALL, ALL, ALL);

	// Autotransition su's socket to su_device
	sepol_typetrans("su", "device", "file", "su_device", NULL);
	sepol_typetrans("su", "device", "dir", "su_device", NULL);
	sepol_allow("su_device", "tmpfs", "filesystem", "associate");

	// Transition from untrusted_app to su_client
	allowSuClient("shell");
	allowSuClient("untrusted_app");
	allowSuClient("system_app");
	allowSuClient("platform_app");
	if (sepol_exists("priv_app"))
		allowSuClient("priv_app");
	if (sepol_exists("ssd_tool"))
		allowSuClient("ssd_tool");

	// Allow init to execute su daemon/transition
	sepol_allow("init", "su", "process", "transition");
	sepol_allow("init", "su", "process", "rlimitinh");
	sepol_allow("init", "su", "process", "siginh");
	sepol_allow("init", "su", "process", "noatsecure");
	suRights();
	otherToSU();

	// Need to set su_device/su as trusted to be accessible from other categories
	sepol_attradd("su_device", "mlstrustedobject");
	sepol_attradd("su", "mlstrustedsubject");

}

// Minimal to run Magisk script before live patching
void sepol_min_rules() {

	if (!sepol_exists("su"))
		sepol_create("su");
	sepol_permissive("su");
	sepol_permissive("init");

	sepol_attradd("su", "mlstrustedsubject");

	// Let init run stuffs in su context
	sepol_allow("kernel", "su", "fd", "use");
	sepol_allow("init", "su", "process", ALL);
	sepol_allow("init", "system_file", "dir", ALL);
	sepol_allow("init", "system_file", "lnk_file", ALL);
	sepol_allow("init", "system_file", "file", ALL);

	// Misc: basic shell scripts, prop management etc.
	sepol_allow("su", "property_socket", "sock_file", "write");
	if (sepol_exists("default_prop"))
		sepol_allow("su", "default_prop", "property_service", "set");
	sepol_allow("su", "init", "unix_stream_socket", "connectto");
	sepol_allow("su", "su", "unix_dgram_socket", ALL);
	sepol_allow("su", "su", "unix_stream_socket", ALL);
	sepol_allow("su", "su", "process", ALL);
	sepol_allow("su", "su", "capability", ALL);
	sepol_allow("su", "su", "file", ALL);
	sepol_allow("su", "su", "fifo_file", ALL);
	sepol_allow("su", "su", "lnk_file", ALL);
	sepol_allow("su", "su", "dir", ALL);

	// Allow su to do anything to files/dir/links
	sepol_allow("su", ALL, "file", ALL);
	sepol_allow("su", ALL, "dir", ALL);
	sepol_allow("su", ALL, "lnk_file", ALL);

	// For sepolicy live patching
	sepol_allow("su", "kernel", "security", "read_policy");
	sepol_allow("su", "kernel", "security", "load_policy");

	// For mounting loop devices and mirrors
	sepol_allow("su", "kernel", "process", "setsched");
	sepol_allow("su", "labeledfs", "filesystem", "mount");
	sepol_allow("su", "labeledfs", "filesystem", "unmount");
	sepol_allow("su", "loop_device", "blk_file", ALL);
	sepol_allow("su", "block_device", "blk_file", ALL);
	sepol_allow("su", "system_block_device", "blk_file", ALL);

	// Xposed
	sepol_allow("untrusted_app", "untrusted_app", "capability", "setgid");
	sepol_allow("system_server", "dex2oat_exec", "file", ALL);

}
