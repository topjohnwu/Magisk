#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <magisk.h>
#include <utils.h>
#include <resetprop.h>
#include <db.h>

#include "magiskhide.h"

using namespace std;

// Protect access to both hide_list and hide_uid
pthread_mutex_t list_lock;
vector<string> hide_list;
set<int> hide_uid;

// Treat GMS separately as we're only interested in one component
int gms_uid = -1;

static pthread_t proc_monitor_thread;

static const char *prop_key[] =
	{ "ro.boot.vbmeta.device_state", "ro.boot.verifiedbootstate", "ro.boot.flash.locked",
	  "ro.boot.veritymode", "ro.boot.warranty_bit", "ro.warranty_bit", "ro.debuggable",
	  "ro.secure", "ro.build.type", "ro.build.tags", "ro.build.selinux", nullptr };

static const char *prop_value[] =
	{ "locked", "green", "1",
	  "enforcing", "0", "0", "0",
	  "1", "user", "release-keys", "0", nullptr };

void manage_selinux() {
	char val;
	int fd = xopen(SELINUX_ENFORCE, O_RDONLY);
	xxread(fd, &val, sizeof(val));
	close(fd);
	// Permissive
	if (val == '0') {
		chmod(SELINUX_ENFORCE, 0640);
		chmod(SELINUX_POLICY, 0440);
	}
}

static void hide_sensitive_props() {
	LOGI("hide_utils: Hiding sensitive props\n");

	// Hide all sensitive props
	for (int i = 0; prop_key[i]; ++i) {
		auto value = getprop(prop_key[i]);
		if (!value.empty() && value != prop_value[i])
			setprop(prop_key[i], prop_value[i], false);
	}
}

/*
 * Bionic's atoi runs through strtol().
 * Use our own implementation for faster conversion.
 */
static inline int parse_int(const char *s) {
	int val = 0;
	char c;
	while ((c = *(s++))) {
		if (c > '9' || c < '0')
			return -1;
		val = val * 10 + c - '0';
	}
	return val;
}

// Leave /proc fd opened as we're going to read from it repeatedly
static DIR *procfp;
void crawl_procfs(const function<bool (int)> &fn) {
	struct dirent *dp;
	int pid;
	rewinddir(procfp);
	while ((dp = readdir(procfp))) {
		pid = parse_int(dp->d_name);
		if (pid > 0 && !fn(pid))
			break;
	}
}

static bool proc_name_match(int pid, const char *name) {
	char buf[4019];
	FILE *f;
	sprintf(buf, "/proc/%d/comm", pid);
	if ((f = fopen(buf, "re"))) {
		fgets(buf, sizeof(buf), f);
		fclose(f);
		if (strcmp(buf, name) == 0)
			return true;
	} else {
		// The PID is already killed
		return false;
	}

	sprintf(buf, "/proc/%d/cmdline", pid);
	if ((f = fopen(buf, "re"))) {
		fgets(buf, sizeof(buf), f);
		fclose(f);
		if (strcmp(basename(buf), name) == 0)
			return true;
	} else {
		// The PID is already killed
		return false;
	}

	sprintf(buf, "/proc/%d/exe", pid);
	ssize_t len;
	if ((len = readlink(buf, buf, sizeof(buf))) < 0)
		return false;
	buf[len] = '\0';
	return strcmp(basename(buf), name) == 0;
}

static void kill_process(const char *name) {
	// We do NOT want to kill GMS itself
	if (strcmp(name, SAFETYNET_PKG) == 0)
		name = SAFETYNET_PROCESS;
	crawl_procfs([=](int pid) -> bool {
		if (proc_name_match(pid, name)) {
			if (kill(pid, SIGTERM) == 0)
				LOGD("hide_utils: killed PID=[%d] (%s)\n", pid, name);
			return false;
		}
		return true;
	});
}

static void kill_process(int uid) {
	// We do NOT want to kill all GMS processes
	if (uid == gms_uid) {
		kill_process(SAFETYNET_PROCESS);
		return;
	}
	crawl_procfs([=](int pid) -> bool {
		if (get_uid(pid) == uid && kill(pid, SIGTERM) == 0)
			LOGD("hide_utils: killed PID=[%d]\n", pid);
		return true;
	});
}

static int add_pkg_uid(const char *pkg) {
	char path[4096];
	struct stat st;
	const char *data = SDK_INT >= 24 ? "/data/user_de/0" : "/data/data";
	sprintf(path, "%s/%s", data, pkg);
	if (stat(path, &st) == 0) {
		hide_uid.insert(st.st_uid);
		return st.st_uid;
	}
	return -1;
}

void refresh_uid() {
	hide_uid.clear();
	for (auto &s : hide_list)
		add_pkg_uid(s.c_str());
}

void clean_magisk_props() {
	getprop([](const char *name, auto, auto) -> void {
		if (strstr(name, "magisk"))
			deleteprop(name);
	}, nullptr, false);
}

int add_list(const char *pkg) {
	for (auto &s : hide_list) {
		if (s == pkg)
			return HIDE_ITEM_EXIST;
	}

	// Add to database
	char sql[4096];
	snprintf(sql, sizeof(sql), "INSERT INTO hidelist (process) VALUES('%s')", pkg);
	char *err = db_exec(sql);
	db_err_cmd(err, return DAEMON_ERROR);

	LOGI("hide_list add: [%s]\n", pkg);

	// Critical region
	int uid;
	{
		MutexGuard lock(list_lock);
		hide_list.emplace_back(pkg);
		uid = add_pkg_uid(pkg);
	}

	kill_process(uid);
	return DAEMON_SUCCESS;
}

int add_list(int client) {
	char *pkg = read_string(client);
	int ret = add_list(pkg);
	free(pkg);
	update_inotify_mask();
	return ret;
}

static int rm_list(const char *pkg) {
	// Critical region
	{
		MutexGuard lock(list_lock);
		bool remove = false;
		for (auto it = hide_list.begin(); it != hide_list.end(); ++it) {
			if (*it == pkg) {
				remove = true;
				LOGI("hide_list rm: [%s]\n", pkg);
				hide_list.erase(it);
				break;
			}
		}
		if (!remove)
			return HIDE_ITEM_NOT_EXIST;
	}
	char sql[4096];
	snprintf(sql, sizeof(sql), "DELETE FROM hidelist WHERE process='%s'", pkg);
	char *err = db_exec(sql);
	db_err(err);
	return DAEMON_SUCCESS;
}

int rm_list(int client) {
	char *pkg = read_string(client);
	int ret = rm_list(pkg);
	free(pkg);
	if (ret == DAEMON_SUCCESS)
		update_inotify_mask(true);
	return ret;
}

static int init_list(void *, int, char **data, char**) {
	LOGI("hide_list init: [%s]\n", *data);
	hide_list.emplace_back(*data);
	kill_process(*data);
	int uid = add_pkg_uid(*data);
	if (strcmp(*data, SAFETYNET_PKG) == 0)
		gms_uid = uid;
	return 0;
}

static void init_list(const char *pkg) {
	init_list(nullptr, 0, (char **) &pkg, nullptr);
}

#define LEGACY_LIST MODULEROOT "/.core/hidelist"

bool init_list() {
	LOGD("hide_list: initialize\n");

	char *err = db_exec("SELECT process FROM hidelist", init_list);
	db_err_cmd(err, return false);

	// Migrate old hide list into database
	if (access(LEGACY_LIST, R_OK) == 0) {
		file_readline(LEGACY_LIST, [](string_view &s) -> bool {
			add_list(s.data());
			return true;
		});
		unlink(LEGACY_LIST);
	}

	// Add SafetyNet by default
	rm_list(SAFETYNET_PROCESS);
	rm_list(SAFETYNET_COMPONENT);
	init_list(SAFETYNET_PKG);

	update_inotify_mask();
	return true;
}

void ls_list(int client) {
	FILE *out = fdopen(recv_fd(client), "a");
	for (auto &s : hide_list)
		fprintf(out, "%s\n", s.c_str());
	fclose(out);
	write_int(client, DAEMON_SUCCESS);
	close(client);
}

static void set_hide_config() {
	char sql[64];
	sprintf(sql, "REPLACE INTO settings (key,value) VALUES('%s',%d)",
			DB_SETTING_KEYS[HIDE_CONFIG], hide_enabled);
	char *err = db_exec(sql);
	db_err(err);
}

static inline void launch_err(int client, int code = DAEMON_ERROR) {
	if (code != HIDE_IS_ENABLED)
		hide_enabled = false;
	if (client >= 0) {
		write_int(client, code);
		close(client);
	}
	pthread_exit(nullptr);
}

#define LAUNCH_ERR launch_err(client)

void launch_magiskhide(int client) {
	if (SDK_INT < 19)
		LAUNCH_ERR;

	if (hide_enabled)
		launch_err(client, HIDE_IS_ENABLED);

	if (access("/proc/1/ns/mnt", F_OK) != 0)
		launch_err(client, HIDE_NO_NS);

	hide_enabled = true;
	set_hide_config();
	LOGI("* Starting MagiskHide\n");

	if (procfp == nullptr) {
		int fd = xopen("/proc", O_RDONLY | O_CLOEXEC);
		if (fd < 0)
			LAUNCH_ERR;
		procfp = fdopendir(fd);
	}

	hide_sensitive_props();

	// Initialize the mutex lock
	pthread_mutex_init(&list_lock, nullptr);

	// Initialize the hide list
	if (!init_list())
		LAUNCH_ERR;

	// Get thread reference
	proc_monitor_thread = pthread_self();
	if (client >= 0) {
		write_int(client, DAEMON_SUCCESS);
		close(client);
		client = -1;
	}
	// Start monitoring
	proc_monitor();

	// proc_monitor should not return
	LAUNCH_ERR;
}

int stop_magiskhide() {
	LOGI("* Stopping MagiskHide\n");

	hide_enabled = false;
	set_hide_config();
	pthread_kill(proc_monitor_thread, TERM_THREAD);

	return DAEMON_SUCCESS;
}

void auto_start_magiskhide() {
	db_settings dbs;
	get_db_settings(&dbs, HIDE_CONFIG);
	if (dbs[HIDE_CONFIG]) {
		new_daemon_thread([](auto) -> void* {
			launch_magiskhide(-1);
			return nullptr;
		});
	}
}
