#include <stdio.h>
#include <sepol/policydb/policydb.h>

#define ALL NULL

extern int add_rule(char *s, char *t, char *c, char *p, int effect, int not, policydb_t *policy);
extern void create_domain(char *d, policydb_t *policy);
extern int add_transition(char *srcS, char *origS, char *tgtS, char *c, policydb_t *policy);
extern int add_type(char *domainS, char *typeS, policydb_t *policy);

policydb_t *policy;

void allow(char *s, char *t, char *c, char *p) {
	add_rule(s, t, c, p, AVTAB_ALLOWED, 0, policy);
}

void noaudit(char *s, char *t, char *c, char *p) {
	add_rule(s, t, c, p, AVTAB_AUDITDENY, 0, policy);
}

void deny(char *s, char *t, char *c, char *p) {
	add_rule(s, t, c, p, AVTAB_ALLOWED, 1, policy);
}

void setPermissive(char* permissive, int permissive_value) {
	type_datum_t *type;
	create_domain(permissive, policy);
	type = hashtab_search(policy->p_types.table, permissive);
	if (type == NULL) {
			fprintf(stderr, "type %s does not exist\n", permissive);
			return;
	}
	if (ebitmap_set_bit(&policy->permissive_map, type->s.value, permissive_value)) {
		fprintf(stderr, "Could not set bit in permissive map\n");
		return;
	}
}

int exists(char* source) {
	return (int) hashtab_search(policy->p_types.table, source);
}

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
	allow(target, "su_daemon", "unix_stream_socket", "connectto");
	allow(target, "su_daemon", "unix_stream_socket", "getopt");
	allow(target, "su_device", "dir", "search");
	allow(target, "su_device", "dir", "read");
	allow(target, "su_device", "sock_file", "read");
	allow(target, "su_device", "sock_file", "write");
	allow("su_daemon", target, "fd", "use");
	allow("su_daemon", target, "fifo_file", "read");
	allow("su_daemon", target, "fifo_file", "write");
	allow("su_daemon", target, "fifo_file", "getattr");
	allow("su_daemon", target, "fifo_file", "ioctl");
	allow("su_daemon", target, "dir", "search");
	allow("su_daemon", target, "file", "read");
	allow("su_daemon", target, "file", "open");
	allow("su_daemon", target, "lnk_file", "read");
	allow("su_daemon", "su_daemon", "capability", "sys_ptrace");
}

void suDaemonRights() {
	allow("su_daemon", "rootfs", "file", "entrypoint");
	allow("su_daemon", "su_daemon", "dir", "search");
	allow("su_daemon", "su_daemon", "dir", "read");
	allow("su_daemon", "su_daemon", "file", "read");
	allow("su_daemon", "su_daemon", "file", "write");
	allow("su_daemon", "su_daemon", "file", "open");
	allow("su_daemon", "su_daemon", "lnk_file", "read");
	allow("su_daemon", "su_daemon", "unix_dgram_socket", "create");
	allow("su_daemon", "su_daemon", "unix_dgram_socket", "connect");
	allow("su_daemon", "su_daemon", "unix_dgram_socket", "write");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "create");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "ioctl");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "read");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "getattr");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "write");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "setattr");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "lock");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "append");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "bind");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "connect");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "getopt");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "setopt");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "shutdown");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "listen");
	allow("su_daemon", "su_daemon", "unix_stream_socket", "accept");
	allow("su_daemon", "devpts", "chr_file", "read");
	allow("su_daemon", "devpts", "chr_file", "write");
	allow("su_daemon", "devpts", "chr_file", "open");
	allow("su_daemon", "devpts", "chr_file", "getattr");
	allow("su_daemon", "untrusted_app_devpts", "chr_file", "read");
	allow("su_daemon", "untrusted_app_devpts", "chr_file", "write");
	allow("su_daemon", "untrusted_app_devpts", "chr_file", "open");
	allow("su_daemon", "untrusted_app_devpts", "chr_file", "getattr");
	allow("su_daemon", "su_daemon", "capability", "setuid");
	allow("su_daemon", "su_daemon", "capability", "setgid");
	allow("su_daemon", "app_data_file", "dir", "getattr");
	allow("su_daemon", "app_data_file", "dir", "search");
	allow("su_daemon", "app_data_file", "dir", "write");
	allow("su_daemon", "app_data_file", "dir", "add_name");
	allow("su_daemon", "app_data_file", "file", "getattr");
	allow("su_daemon", "app_data_file", "file", "read");
	allow("su_daemon", "app_data_file", "file", "open");
	allow("su_daemon", "app_data_file", "file", "lock");
	allow("su_daemon", "su_daemon", "capability", "dac_override");
	allow("su_daemon", "su_daemon", "process", "fork");
	allow("su_daemon", "su_daemon", "process", "sigchld");
	allow("su_daemon", "toolbox_exec", "file", "execute");
	allow("su_daemon", "toolbox_exec", "file", "read");
	allow("su_daemon", "toolbox_exec", "file", "open");
	allow("su_daemon", "toolbox_exec", "file", "execute_no_trans");
	allow("su_daemon", "device", "dir", "write");
	allow("su_daemon", "device", "dir", "add_name");
	allow("su_daemon", "su_device", "dir", "create");
	allow("su_daemon", "su_device", "dir", "setattr");
	allow("su_daemon", "su_device", "dir", "remove_name");
	allow("su_daemon", "su_device", "dir", "add_name");
	allow("su_daemon", "su_device", "sock_file", "create");
	allow("su_daemon", "su_device", "sock_file", "unlink");
	allow("su_daemon", "zygote_exec", "file", "execute");
	allow("su_daemon", "zygote_exec", "file", "read");
	allow("su_daemon", "zygote_exec", "file", "open");
	allow("su_daemon", "zygote_exec", "file", "execute_no_trans");
	allow("su_daemon", "zygote_exec", "lnk_file", "read");
	allow("su_daemon", "zygote_exec", "lnk_file", "getattr");
	allow("su_daemon", "su_device", "dir", "search");
	allow("su_daemon", "su_device", "dir", "write");
	allow("su_daemon", "su_device", "dir", "add_name");
	allow("su_daemon", "su_daemon", "process", "setexec");
	allow("su_daemon", "shell_exec", "file", "execute");
	allow("su_daemon", "shell_exec", "file", "read");
	allow("su_daemon", "shell_exec", "file", "open");
	allow("su_daemon", "su_daemon", "capability", "chown");
	allow("su_daemon", "su", "process", "transition");
	allow("su_daemon", "su", "process", "siginh");
	allow("su_daemon", "su", "process", "rlimitinh");
	allow("su_daemon", "su", "process", "noatsecure");

	// suL9
	allow("su_daemon", "su_daemon", "dir", ALL);
	allow("su_daemon", "su_daemon", "file", ALL);
	allow("su_daemon", "su_daemon", "lnk_file", ALL);
	allow("su_daemon", "system_data_file", "dir", ALL);
	allow("su_daemon", "system_data_file", "file", ALL);
	allow("su_daemon", "system_data_file", "lnk_file", ALL);
	allow("su_daemon", "labeledfs", "filesystem", "associate");
	allow("su_daemon", "su_daemon", "process", "setfscreate");
	allow("su_daemon", "tmpfs", "filesystem", "associate");
	allow("su_daemon", "su_daemon", "file", "relabelfrom");
	allow("su_daemon", "system_file", "file", "mounton");

	// Allow to start daemon by script in su domain
	allow("su_daemon", "su", "file", "write");
	allow("su_daemon", "proc", "file", "read");
	allow("su_daemon", "proc", "file", "open");
	allow("su_daemon", "su_daemon", "process", "setcurrent");
	allow("su_daemon", "system_file", "file", "execute_no_trans");
}

void suBind() {
	allow("su_daemon", "su_exec", "file", "mounton");
	allow("su_daemon", "su_exec", "file", "read");
	allow("su_daemon", "su_device", "dir", ALL);
	allow("su_daemon", "su_device", "file", ALL);
	allow("su_daemon", "su_device", "lnk_file", ALL);
	allow("su_daemon", "system_file", "file", "relabelto");
}

void suRights() {
	allow("su_daemon", "su_daemon", "capability", "sys_admin");
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
	add_type("su", "netdomain", policy);
	add_type("su", "bluetoothdomain", policy);

	// suBackL6
	allow("surfaceflinger", "app_data_file", "dir", ALL);
	allow("surfaceflinger", "app_data_file", "file", ALL);
	allow("surfaceflinger", "app_data_file", "lnk_file", ALL);
	add_type("surfaceflinger", "mlstrustedsubject", policy);
}

void phh_rules(policydb_t *policydb) {
	policy = policydb;

	// Samsung specific
	// Prevent system from loading policy
	if(exists("knox_system_app")) {
		samsung();
	}

	// Create domains if they don't exist
	setPermissive("su", 1);
	setPermissive("su_device", 0);
	setPermissive("su_daemon", 0);

	// Autotransition su's socket to su_device
	add_transition("su_daemon", "device", "su_device", "file", policy);
	add_transition("su_daemon", "device", "su_device", "dir", policy);
	allow("su_device", "tmpfs", "filesystem", "associate");

	// Transition from untrusted_app to su_client
	allowSuClient("shell");
	allowSuClient("untrusted_app");
	allowSuClient("system_app");
	allowSuClient("platform_app");
	allowSuClient("su");

	if(exists("ssd_tool")) {
		allowSuClient("ssd_tool");
	}

	// Allow init to execute su daemon/transition
	allow("init", "su_daemon", "process", "transition");
	allow("init", "su_daemon", "process", "rlimitinh");
	allow("init", "su_daemon", "process", "siginh");
	allow("init", "su_daemon", "process", "noatsecure");
	suDaemonRights();
	suBind();
	suRights();
	otherToSU();

	// Need to set su_device/su as trusted to be accessible from other categories
	add_type("su_device", "mlstrustedobject", policy);
	add_type("su_daemon", "mlstrustedsubject", policy);
	add_type("su", "mlstrustedsubject", policy);
}

void magisk_rules(policydb_t *policydb) {
	policy = policydb;

	setPermissive("su", 1);
	setPermissive("init", 1);

	add_type("su", "mlstrustedsubject", policy);

	// Minimal to run Magisk script before live patching
	allow("kernel", "su", "fd", "use");
	allow("init", "su", "process", ALL);
	allow("init", "system_file", "dir", ALL);
	allow("init", "system_file", "lnk_file", ALL);
	allow("init", "system_file", "file", ALL);
	allow("su", "property_socket", "sock_file", "write");
	allow("su", "shell_exec", "file", ALL);
	allow("su", "init", "unix_stream_socket", "connectto");
	allow("su", "su", "unix_dgram_socket", ALL);
	allow("su", "su", "unix_stream_socket", ALL);
	allow("su", "su", "process", ALL);
	allow("su", "su", "capability", ALL);
	allow("su", "su", "file", ALL);
	allow("su", "su", "fifo_file", ALL);
	allow("su", "su", "lnk_file", ALL);
	allow("su", "su", "dir", ALL);
	allow("su", "device", "file", ALL);
	allow("su", "device", "dir", ALL);
	allow("su", "storage_file", "file", ALL);
	allow("su", "storage_file", "dir", ALL);
	allow("su", "sysfs", "file", ALL);
	allow("su", "sysfs", "dir", ALL);
	allow("su", "block_device", "file", ALL);
	allow("su", "block_device", "dir", ALL);
	allow("su", "rootfs", "file", ALL);
	allow("su", "rootfs", "dir", ALL);
	allow("su", "toolbox_exec", "file", ALL);
	allow("su", "toolbox_exec", "dir", ALL);
	allow("su", "cache_file", "file", ALL);
	allow("su", "cache_file", "dir", ALL);
	allow("su", "system_file", "file", ALL);
	allow("su", "system_file", "dir", ALL);
	allow("su", "system_data_file", "file", ALL);
	allow("su", "system_data_file", "dir", ALL);
	allow("su", "kernel", "security", "read_policy");
	allow("su", "kernel", "security", "load_policy");
	allow("su", "selinuxfs", "file", ALL);

	// Xposed
	allow("untrusted_app", "untrusted_app", "capability", "setgid");
	allow("system_server", "dex2oat_exec", "file", ALL);

	// SuperSU
	allow("init", "system_file", "file", "execute_no_trans");
	allow("init", "su", "fd", "use");
	allow("init", "kernel", "security", "read_policy");
	allow("init", "kernel", "security", "load_policy");


}