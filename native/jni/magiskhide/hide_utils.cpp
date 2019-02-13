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
set<uid_t> hide_uid;

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

struct ps_arg {
	const char *name;
	uid_t uid;
};

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

void hide_sensitive_props() {
	LOGI("hide_utils: Hiding sensitive props\n");

	// Hide all sensitive props
	for (int i = 0; prop_key[i]; ++i) {
		auto value = getprop(prop_key[i]);
		if (!value.empty() && value != prop_value[i])
			setprop(prop_key[i], prop_value[i], false);
	}
}

static bool is_digits(const char *s) {
	for (const char *c = s; *c; ++c) {
		if (*c < '0' || *c > '9')
			return false;
	}
	return true;
}

static void ps(void (*cb)(int, void*), void *arg) {
	DIR *dir;
	struct dirent *entry;

	if (!(dir = xopendir("/proc")))
		return;

	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR && is_digits(entry->d_name))
			cb(atoi(entry->d_name), arg);
	}

	closedir(dir);
}

static bool proc_name_match(int pid, const char *name) {
	char buf[4019];
	FILE *f;
	sprintf(buf, "/proc/%d/comm", pid);
	if ((f = fopen(buf, "re"))) {
		fgets(buf, sizeof(buf), f);
		if (strcmp(buf, name) == 0)
			return true;
	} else {
		// The PID is already killed
		return false;
	}
	fclose(f);

	sprintf(buf, "/proc/%d/cmdline", pid);
	if ((f = fopen(buf, "re"))) {
		fgets(buf, sizeof(buf), f);
		if (strcmp(basename(buf), name) == 0)
			return true;
	} else {
		// The PID is already killed
		return false;
	}
	fclose(f);

	sprintf(buf, "/proc/%d/exe", pid);
	ssize_t len;
	if ((len = readlink(buf, buf, sizeof(buf))) < 0)
		return false;
	buf[len] = '\0';
	return strcmp(basename(buf), name) == 0;
}

static void kill_proc_cb(int pid, void *v) {
	auto args = static_cast<ps_arg *>(v);
	if (proc_name_match(pid, args->name))
		kill(pid, SIGTERM);
	else if (args->uid > 0) {
		char buf[64];
		struct stat st;
		sprintf(buf, "/proc/%d", pid);
		stat(buf, &st);
		if (args->uid == st.st_uid)
			kill(pid, SIGTERM);
	}

}

static void kill_process(const char *name) {
	ps_arg args;
	char *slash = nullptr;
	if (strcmp(name, SAFETYNET_COMPONENT) == 0) {
		// We do NOT want to kill gms, it will cause massive system crashes
		args.name = SAFETYNET_PROCESS;
	} else {
		// Only leave the package name part of component name temporarily
		slash = strchr((char *)name, '/');
		if (slash)
			*slash = '\0';
		args.name = name;
	}
	struct stat st;
	int fd = xopen("/data/data", O_RDONLY | O_CLOEXEC);
	if (fstatat(fd, args.name, &st, 0) == 0)
		args.uid = st.st_uid;
	else
		args.uid = 0;
	close(fd);
	ps(kill_proc_cb, &args);
	// Revert back to component name
	if (slash)
		*slash = '/';
}

static int add_pkg_uid(const char *proc) {
	char path[4096];
	struct stat st;
	const char *data = SDK_INT >= 24 ? "/data/user_de/0" : "/data/data";
	sprintf(path, "%s/%s", data, proc);
	if (xstat(path, &st) == 0) {
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
	LOGD("hide_utils: Cleaning magisk props\n");
	getprop([](const char *name, auto, auto) -> void {
		if (strstr(name, "magisk"))
			deleteprop(name);
	}, nullptr, false);
}

int add_list(const char *proc) {
	for (auto &s : hide_list) {
		if (s == proc)
			return HIDE_ITEM_EXIST;
	}

	// Add to database
	char sql[4096];
	snprintf(sql, sizeof(sql), "INSERT INTO hidelist (process) VALUES('%s')", proc);
	char *err = db_exec(sql);
	db_err_cmd(err, return DAEMON_ERROR);

	LOGI("hide_list add: [%s]\n", proc);

	// Critical region
	pthread_mutex_lock(&list_lock);
	hide_list.emplace_back(proc);
	add_pkg_uid(proc);
	pthread_mutex_unlock(&list_lock);

	kill_process(proc);
	return DAEMON_SUCCESS;
}

int add_list(int client) {
	char *proc = read_string(client);
	int ret = add_list(proc);
	free(proc);
	update_inotify_mask();
	return ret;
}

static int rm_list(const char *proc) {
	// Critical region
	bool remove = false;
	pthread_mutex_lock(&list_lock);
	for (auto it = hide_list.begin(); it != hide_list.end(); ++it) {
		if (*it == proc) {
			remove = true;
			LOGI("hide_list rm: [%s]\n", proc);
			hide_list.erase(it);
			break;
		}
	}
	if (remove)
		refresh_uid();
	pthread_mutex_unlock(&list_lock);

	if (remove) {
		char sql[4096];
		snprintf(sql, sizeof(sql), "DELETE FROM hidelist WHERE process='%s'", proc);
		char *err = db_exec(sql);
		db_err(err);
		return DAEMON_SUCCESS;
	} else {
		return HIDE_ITEM_NOT_EXIST;
	}
}

int rm_list(int client) {
	char *proc = read_string(client);
	int ret = rm_list(proc);
	free(proc);
	update_inotify_mask();
	return ret;
}

int init_list(void *, int, char **data, char**) {
	LOGI("hide_list init: [%s]\n", *data);
	hide_list.emplace_back(*data);
	kill_process(*data);
	int uid = add_pkg_uid(*data);
	if (strcmp(*data, SAFETYNET_PKG) == 0)
		gms_uid = uid;
	return 0;
}

void init_list(const char *proc) {
	init_list(nullptr, 0, (char **) &proc, nullptr);
}

#define LEGACY_LIST MODULEROOT "/.core/hidelist"

bool init_list() {
	LOGD("hide_list: initialize\n");

	char *err = db_exec("SELECT process FROM hidelist", init_list);
	db_err_cmd(err, return false);

	// Migrate old hide list into database
	if (access(LEGACY_LIST, R_OK) == 0) {
		auto tmp = file_to_vector(LEGACY_LIST);
		for (auto &s : tmp)
			add_list(s.c_str());
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

int launch_magiskhide(int client) {
	if (SDK_INT < 19)
		goto error;

	if (hide_enabled)
		return HIDE_IS_ENABLED;

	hide_enabled = true;
	set_hide_config();
	LOGI("* Starting MagiskHide\n");

	hide_sensitive_props();

	// Initialize the mutex lock
	pthread_mutex_init(&list_lock, nullptr);

	// Initialize the hide list
	if (!init_list())
		goto error;

	// Get thread reference
	proc_monitor_thread = pthread_self();
	if (client >= 0) {
		write_int(client, DAEMON_SUCCESS);
		close(client);
	}
	// Start monitoring
	proc_monitor();

error:
	hide_enabled = false;
	return DAEMON_ERROR;
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
		pthread_t thread;
		xpthread_create(&thread, nullptr, [](void*) -> void* {
			launch_magiskhide(-1);
			return nullptr;
		}, nullptr);
		pthread_detach(thread);
	}
}
