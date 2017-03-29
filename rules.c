#include "magiskpolicy.h"

void samsung() {
	deny("init", "kernel", "security", "load_policy");
	deny("policyloader_app", "security_spota_file", "dir", "read");
	deny("policyloader_app", "security_spota_file", "dir", "write");
	deny("policyloader_app", "security_spota_file", "file", "read");
	deny("policyloader_app", "security_spota_file", "file", "write");
	deny("system_server", "security_spota_file", "dir", "read");
	deny("system_server", "security_spota_file", "dir", "write");
	deny("system_server", "security_spota_file", "file", "read");
	deny("system_server", "security_spota_file", "file", "write");
	deny("system_app", "security_spota_file", "dir", "read");
	deny("system_app", "security_spota_file", "dir", "write");
	deny("system_app", "security_spota_file", "file", "read");
	deny("system_app", "security_spota_file", "file", "write");
	deny("installd", "security_spota_file", "dir", "read");
	deny("installd", "security_spota_file", "dir", "write");
	deny("installd", "security_spota_file", "file", "read");
	deny("installd", "security_spota_file", "file", "write");
	deny("init", "security_spota_file", "dir", "read");
	deny("init", "security_spota_file", "dir", "write");
	deny("init", "security_spota_file", "file", "read");
	deny("init", "security_spota_file", "file", "write");
	deny("ueventd", "security_spota_file", "dir", "read");
	deny("ueventd", "security_spota_file", "dir", "write");
	deny("ueventd", "security_spota_file", "file", "read");
	deny("ueventd", "security_spota_file", "file", "write");
	deny("runas", "security_spota_file", "dir", "read");
	deny("runas", "security_spota_file", "dir", "write");
	deny("runas", "security_spota_file", "file", "read");
	deny("runas", "security_spota_file", "file", "write");
	deny("drsd", "security_spota_file", "dir", "read");
	deny("drsd", "security_spota_file", "dir", "write");
	deny("drsd", "security_spota_file", "file", "read");
	deny("drsd", "security_spota_file", "file", "write");
	deny("debuggerd", "security_spota_file", "dir", "read");
	deny("debuggerd", "security_spota_file", "dir", "write");
	deny("debuggerd", "security_spota_file", "file", "read");
	deny("debuggerd", "security_spota_file", "file", "write");
	deny("vold", "security_spota_file", "dir", "read");
	deny("vold", "security_spota_file", "dir", "write");
	deny("vold", "security_spota_file", "file", "read");
	deny("vold", "security_spota_file", "file", "write");
	deny("zygote", "security_spota_file", "dir", "read");
	deny("zygote", "security_spota_file", "dir", "write");
	deny("zygote", "security_spota_file", "file", "read");
	deny("zygote", "security_spota_file", "file", "write");
	deny("auditd", "security_spota_file", "dir", "read");
	deny("auditd", "security_spota_file", "dir", "write");
	deny("auditd", "security_spota_file", "file", "read");
	deny("auditd", "security_spota_file", "file", "write");
	deny("servicemanager", "security_spota_file", "dir", "read");
	deny("servicemanager", "security_spota_file", "dir", "write");
	deny("servicemanager", "security_spota_file", "file", "read");
	deny("servicemanager", "security_spota_file", "file", "write");
	deny("itsonbs", "security_spota_file", "dir", "read");
	deny("itsonbs", "security_spota_file", "dir", "write");
	deny("itsonbs", "security_spota_file", "file", "read");
	deny("itsonbs", "security_spota_file", "file", "write");
	deny("commonplatformappdomain", "security_spota_file", "dir", "read");
	deny("commonplatformappdomain", "security_spota_file", "dir", "write");
	deny("commonplatformappdomain", "security_spota_file", "file", "read");
	deny("commonplatformappdomain", "security_spota_file", "file", "write");
}

void allowSuClient(char *target) {
	allow(target, "rootfs", "file", "execute_no_trans");
	allow(target, "rootfs", "file", "execute");
	allow(target, "su", "unix_stream_socket", "connectto");
	allow(target, "su", "unix_stream_socket", "getopt");
	allow(target, "su_device", "dir", "search");
	allow(target, "su_device", "dir", "read");
	allow(target, "su_device", "sock_file", "read");
	allow(target, "su_device", "sock_file", "write");
}

void suRights() {
	allow("servicemanager", "su", "dir", "search");
	allow("servicemanager", "su", "dir", "read");
	allow("servicemanager", "su", "file", "open");
	allow("servicemanager", "su", "file", "read");
	allow("servicemanager", "su", "process", "getattr");
	allow("servicemanager", "su", "binder", "transfer");
	allow("system_server", "su", "binder", "call");
}

void otherToSU() {
	// allowLog
	allow("logd", "su", "dir", "search");
	allow("logd", "su", "file", "read");
	allow("logd", "su", "file", "open");
	allow("logd", "su", "file", "getattr");

	// suBackL0
	allow("system_server", "su", "binder", "call");
	allow("system_server", "su", "binder", "transfer");

	// ES Explorer opens a sokcet
	allow("untrusted_app", "su", "unix_stream_socket", "ioctl");
	allow("untrusted_app", "su", "unix_stream_socket", "read");
	allow("untrusted_app", "su", "unix_stream_socket", "getattr");
	allow("untrusted_app", "su", "unix_stream_socket", "write");
	allow("untrusted_app", "su", "unix_stream_socket", "setattr");
	allow("untrusted_app", "su", "unix_stream_socket", "lock");
	allow("untrusted_app", "su", "unix_stream_socket", "append");
	allow("untrusted_app", "su", "unix_stream_socket", "bind");
	allow("untrusted_app", "su", "unix_stream_socket", "connect");
	allow("untrusted_app", "su", "unix_stream_socket", "getopt");
	allow("untrusted_app", "su", "unix_stream_socket", "setopt");
	allow("untrusted_app", "su", "unix_stream_socket", "shutdown");
	allow("untrusted_app", "su", "unix_stream_socket", "connectto");

	// Any domain is allowed to send su "sigchld"
	allow(ALL, "su", "process", "sigchld");

	// uNetworkL0
	attradd("su", "netdomain");
	attradd("su", "bluetoothdomain");

	// suBackL6
	allow("surfaceflinger", "app_data_file", "dir", ALL);
	allow("surfaceflinger", "app_data_file", "file", ALL);
	allow("surfaceflinger", "app_data_file", "lnk_file", ALL);
	attradd("surfaceflinger", "mlstrustedsubject");

	// suMiscL6
	if (exists("audioserver"))
		allow("audioserver", "audioserver", "process", "execmem");
}

void full_rules() {
	// Samsung specific
	// Prevent system from loading policy
	if(exists("knox_system_app"))
		samsung();

	// Min rules first
	min_rules();

	// Create domains if they don't exist
	if (!exists("su_device"))
		create("su_device");
	enforce("su_device");

	// Patch su to everything
	allow("su", ALL, ALL, ALL);

	// Autotransition su's socket to su_device
	add_transition("su", "device", "file", "su_device");
	add_transition("su", "device", "dir", "su_device");
	allow("su_device", "tmpfs", "filesystem", "associate");

	// Transition from untrusted_app to su_client
	allowSuClient("shell");
	allowSuClient("untrusted_app");
	allowSuClient("system_app");
	allowSuClient("platform_app");
	if (exists("priv_app"))
		allowSuClient("priv_app");
	if (exists("ssd_tool"))
		allowSuClient("ssd_tool");

	// Allow init to execute su daemon/transition
	allow("init", "su", "process", "transition");
	allow("init", "su", "process", "rlimitinh");
	allow("init", "su", "process", "siginh");
	allow("init", "su", "process", "noatsecure");
	suRights();
	otherToSU();

	// Need to set su_device/su as trusted to be accessible from other categories
	attradd("su_device", "mlstrustedobject");
	attradd("su", "mlstrustedsubject");

}

// Minimal to run Magisk script before live patching
void min_rules() {

	if (!exists("su"))
		create("su");
	permissive("su");
	permissive("init");

	attradd("su", "mlstrustedsubject");

	// Let init run stuffs in su context
	allow("kernel", "su", "fd", "use");
	allow("init", "su", "process", ALL);
	allow("init", "system_file", "dir", ALL);
	allow("init", "system_file", "lnk_file", ALL);
	allow("init", "system_file", "file", ALL);

	// Misc: basic shell scripts, prop management etc.
	allow("su", "property_socket", "sock_file", "write");
	if (exists("default_prop"))
		allow("su", "default_prop", "property_service", "set");
	allow("su", "init", "unix_stream_socket", "connectto");
	allow("su", "su", "unix_dgram_socket", ALL);
	allow("su", "su", "unix_stream_socket", ALL);
	allow("su", "su", "process", ALL);
	allow("su", "su", "capability", ALL);
	allow("su", "su", "file", ALL);
	allow("su", "su", "fifo_file", ALL);
	allow("su", "su", "lnk_file", ALL);
	allow("su", "su", "dir", ALL);

	// Allow su to do anything to files/dir/links
	allow("su", ALL, "file", ALL);
	allow("su", ALL, "dir", ALL);
	allow("su", ALL, "lnk_file", ALL);

	// For sepolicy live patching
	allow("su", "kernel", "security", "read_policy");
	allow("su", "kernel", "security", "load_policy");

	// For mounting loop devices and mirrors
	allow("su", "kernel", "process", "setsched");
	allow("su", "labeledfs", "filesystem", "mount");
	allow("su", "labeledfs", "filesystem", "unmount");
	allow("su", "loop_device", "blk_file", ALL);
	allow("su", "block_device", "blk_file", ALL);
	allow("su", "system_block_device", "blk_file", ALL);

	// Xposed
	allow("untrusted_app", "untrusted_app", "capability", "setgid");
	allow("system_server", "dex2oat_exec", "file", ALL);

}
