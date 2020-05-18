#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <db.hpp>

#include "magiskhide.hpp"

using namespace std;

static pthread_t proc_monitor_thread;
static bool hide_state = false;

// This locks the 2 variables above
static pthread_mutex_t hide_state_lock = PTHREAD_MUTEX_INITIALIZER;

// Leave /proc fd opened as we're going to read from it repeatedly
static DIR *procfp;
void crawl_procfs(const function<bool(int)> &fn) {
	rewinddir(procfp);
	crawl_procfs(procfp, fn);
}

void crawl_procfs(DIR *dir, const function<bool(int)> &fn) {
	struct dirent *dp;
	int pid;
	while ((dp = readdir(dir))) {
		pid = parse_int(dp->d_name);
		if (pid > 0 && !fn(pid))
			break;
	}
}

bool hide_enabled() {
	mutex_guard g(hide_state_lock);
	return hide_state;
}

void set_hide_state(bool state) {
	mutex_guard g(hide_state_lock);
	hide_state = state;
}

static bool proc_name_match(int pid, const char *name) {
	char buf[4019];
	sprintf(buf, "/proc/%d/cmdline", pid);
	if (FILE *f = fopen(buf, "re")) {
		fgets(buf, sizeof(buf), f);
		fclose(f);
		if (strcmp(buf, name) == 0)
			return true;
	}
	return false;
}

static void kill_process(const char *name, bool multi = false) {
	crawl_procfs([=](int pid) -> bool {
		if (proc_name_match(pid, name)) {
			if (kill(pid, SIGTERM) == 0)
				LOGD("hide_utils: killed PID=[%d] (%s)\n", pid, name);
			return multi;
		}
		return true;
	});
}

static bool validate(const char *s) {
	bool dot = false;
	for (char c; (c = *s); ++s) {
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9') || c == '_' || c == ':') {
			dot = false;
			continue;
		}
		if (c == '.') {
			if (dot)  // No consecutive dots
				return false;
			dot = true;
			continue;
		}
		return false;
	}
	return true;
}

static int add_list(const char *pkg, const char *proc = "") {
	if (proc[0] == '\0')
		proc = pkg;

	if (!validate(pkg) || !validate(proc))
		return HIDE_INVALID_PKG;

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
		mutex_guard lock(monitor_lock);
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
		mutex_guard lock(monitor_lock);
		bool remove = false;
		for (auto it = hide_set.begin(); it != hide_set.end();) {
			if (it->first == pkg && (proc[0] == '\0' || it->second == proc)) {
				remove = true;
				LOGI("hide_list rm: [%s]\n", it->second.data());
				it = hide_set.erase(it);
			} else {
				++it;
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

#define SNET_PROC    "com.google.android.gms.unstable"
#define GMS_PKG      "com.google.android.gms"
#define MICROG_PKG   "org.microg.gms.droidguard"

static bool init_list() {
	LOGD("hide_list: initialize\n");

	char *err = db_exec("SELECT * FROM hidelist", [](db_row &row) -> bool {
		init_list(row["package_name"].data(), row["process"].data());
		return true;
	});
	db_err_cmd(err, return false);

	// If Android Q+, also kill blastula pool
	if (SDK_INT >= 29) {
		kill_process("usap32", true);
		kill_process("usap64", true);
	}

	// Add SafetyNet by default
	init_list(GMS_PKG, SNET_PROC);
	init_list(MICROG_PKG, SNET_PROC);

	// We also need to hide the default GMS process if MAGISKTMP != /sbin
	// The snet process communicates with the main process and get additional info
	if (MAGISKTMP != "/sbin")
		init_list(GMS_PKG, GMS_PKG);

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

static void update_hide_config() {
	char sql[64];
	sprintf(sql, "REPLACE INTO settings (key,value) VALUES('%s',%d)",
			DB_SETTING_KEYS[HIDE_CONFIG], hide_state);
	char *err = db_exec(sql);
	db_err(err);
}

int launch_magiskhide() {
	mutex_guard g(hide_state_lock);

	if (SDK_INT < 19)
		return DAEMON_ERROR;

	if (hide_state)
		return HIDE_IS_ENABLED;

	if (access("/proc/1/ns/mnt", F_OK) != 0)
		return HIDE_NO_NS;

	if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
		return DAEMON_ERROR;

	LOGI("* Starting MagiskHide\n");

	// Initialize the hide list
	if (!init_list())
		return DAEMON_ERROR;

	hide_sensitive_props();
	if (DAEMON_STATE >= STATE_BOOT_COMPLETE)
		hide_late_sensitive_props();

	// Initialize the mutex lock
	pthread_mutex_init(&monitor_lock, nullptr);

	// Start monitoring
	void *(*start)(void*) = [](void*) -> void* { proc_monitor(); return nullptr; };
	if (xpthread_create(&proc_monitor_thread, nullptr, start, nullptr))
		return DAEMON_ERROR;

	hide_state = true;
	update_hide_config();
	return DAEMON_SUCCESS;
}

int stop_magiskhide() {
	mutex_guard g(hide_state_lock);

	if (hide_state) {
		LOGI("* Stopping MagiskHide\n");
		pthread_kill(proc_monitor_thread, SIGTERMTHRD);
	}

	hide_state = false;
	update_hide_config();
	return DAEMON_SUCCESS;
}

void auto_start_magiskhide() {
	if (hide_enabled()) {
		pthread_kill(proc_monitor_thread, SIGZYGOTE);
		hide_late_sensitive_props();
	} else if (SDK_INT >= 19) {
		db_settings dbs;
		get_db_settings(dbs, HIDE_CONFIG);
		if (dbs[HIDE_CONFIG])
			launch_magiskhide();
	}
}

void test_proc_monitor() {
	if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
		exit(1);
	proc_monitor();
	exit(0);
}
