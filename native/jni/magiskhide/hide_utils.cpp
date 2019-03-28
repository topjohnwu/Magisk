#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>

#include <magisk.h>
#include <utils.h>
#include <resetprop.h>
#include <db.h>
#include <selinux.h>

#include "magiskhide.h"

using namespace std;

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

// Leave /proc fd opened as we're going to read from it repeatedly
static DIR *procfp;
void crawl_procfs(const function<bool (int)> &fn) {
	rewinddir(procfp);
	crawl_procfs(procfp, fn);
}

void crawl_procfs(DIR *dir, const function<bool (int)> &fn) {
	struct dirent *dp;
	int pid;
	while ((dp = readdir(dir))) {
		pid = parse_int(dp->d_name);
		if (pid > 0 && !fn(pid))
			break;
	}
}

bool proc_name_match(int pid, const char *name) {
	char buf[4019];
	FILE *f;
#if 0
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
#endif

	sprintf(buf, "/proc/%d/cmdline", pid);
	if ((f = fopen(buf, "re"))) {
		fgets(buf, sizeof(buf), f);
		fclose(f);
		if (strcmp(basename(buf), name) == 0)
			return true;
	}
	return false;

#if 0
	sprintf(buf, "/proc/%d/exe", pid);
	ssize_t len;
	if ((len = readlink(buf, buf, sizeof(buf))) < 0)
		return false;
	buf[len] = '\0';
	return strcmp(basename(buf), name) == 0;
#endif
}

static void kill_process(const char *name) {
	crawl_procfs([=](int pid) -> bool {
		if (proc_name_match(pid, name)) {
			if (kill(pid, SIGTERM) == 0)
				LOGD("hide_utils: killed PID=[%d] (%s)\n", pid, name);
			return false;
		}
		return true;
	});
}

void clean_magisk_props() {
	getprop([](const char *name, auto, auto) -> void {
		if (strstr(name, "magisk"))
			deleteprop(name);
	}, nullptr, false);
}

static int add_list(const char *pkg, const char *proc = "") {
	if (proc[0] == '\0')
		proc = pkg;

	for (auto &hide : hide_set)
		if (hide.first == pkg && hide.second == proc)
			return HIDE_ITEM_EXIST;

	// Add to database
	char sql[4096];
	snprintf(sql, sizeof(sql),
			"INSERT INTO hidelist (package_name, process) VALUES('%s', '%s')", pkg, proc);
	char *err = db_exec(sql);
	db_err_cmd(err, return DAEMON_ERROR);

	LOGI("hide_list add: [%s/%s]\n", pkg, proc);

	// Critical region
	{
		MutexGuard lock(monitor_lock);
		hide_set.emplace(pkg, proc);
	}

	kill_process(proc);
	return DAEMON_SUCCESS;
}

int add_list(int client) {
	char *pkg = read_string(client);
	char *proc = read_string(client);
	int ret = add_list(pkg, proc);
	free(pkg);
	free(proc);
	update_uid_map();
	return ret;
}

static int rm_list(const char *pkg, const char *proc = "") {
	{
		// Critical region
		MutexGuard lock(monitor_lock);
		bool remove = false;
		auto next = hide_set.begin();
		decltype(next) cur;
		while (next != hide_set.end()) {
			cur = next;
			++next;
			if (cur->first == pkg && (proc[0] == '\0' || cur->second == proc)) {
				remove = true;
				LOGI("hide_list rm: [%s]\n", cur->second.data());
				hide_set.erase(cur);
			}
		}
		if (!remove)
			return HIDE_ITEM_NOT_EXIST;
	}

	char sql[4096];
	if (proc[0] == '\0')
		snprintf(sql, sizeof(sql), "DELETE FROM hidelist WHERE package_name='%s'", pkg);
	else
		snprintf(sql, sizeof(sql),
				"DELETE FROM hidelist WHERE package_name='%s' AND process='%s'", pkg, proc);
	char *err = db_exec(sql);
	db_err(err);
	return DAEMON_SUCCESS;
}

int rm_list(int client) {
	char *pkg = read_string(client);
	char *proc = read_string(client);
	int ret = rm_list(pkg, proc);
	free(pkg);
	free(proc);
	if (ret == DAEMON_SUCCESS)
		update_uid_map();
	return ret;
}

static void init_list(const char *pkg, const char *proc) {
	LOGI("hide_list init: [%s/%s]\n", pkg, proc);
	hide_set.emplace(pkg, proc);
	kill_process(proc);
}

#define LEGACY_LIST MODULEROOT "/.core/hidelist"

bool init_list() {
	LOGD("hide_list: initialize\n");

	char *err = db_exec("SELECT * FROM hidelist", [](db_row &row) -> bool {
		init_list(row["package_name"].data(), row["process"].data());
		return true;
	});
	db_err_cmd(err, return false);

	// Migrate old hide list into database
	if (access(LEGACY_LIST, R_OK) == 0) {
		file_readline(LEGACY_LIST, [](string_view s) -> bool {
			add_list(s.data());
			return true;
		});
		unlink(LEGACY_LIST);
	}

	// Add SafetyNet by default
	rm_list(SAFETYNET_COMPONENT);
	rm_list(SAFETYNET_PROCESS);
	init_list(SAFETYNET_PKG, SAFETYNET_PROCESS);
	init_list(MICROG_SAFETYNET, SAFETYNET_PROCESS);

	update_uid_map();
	return true;
}

void ls_list(int client) {
	FILE *out = fdopen(recv_fd(client), "a");
	for (auto &hide : hide_set)
		fprintf(out, "%s|%s\n", hide.first.data(), hide.second.data());
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

	if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
		LAUNCH_ERR;

	hide_sensitive_props();

	// Initialize the mutex lock
	pthread_mutex_init(&monitor_lock, nullptr);

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
	pthread_kill(proc_monitor_thread, SIGTERMTHRD);

	return DAEMON_SUCCESS;
}

void auto_start_magiskhide() {
	db_settings dbs;
	get_db_settings(dbs, HIDE_CONFIG);
	if (dbs[HIDE_CONFIG]) {
		new_daemon_thread([](auto) -> void* {
			launch_magiskhide(-1);
			return nullptr;
		});
	}
}
