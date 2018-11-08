#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "magisk.h"
#include "utils.h"
#include "resetprop.h"
#include "magiskhide.h"
#include "daemon.h"
#include "db.h"

Array<CharArray> hide_list;

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
		CharArray value = getprop(prop_key[i]);
		if (!value.empty() && value != prop_value[i])
			setprop(prop_key[i], prop_value[i], false);
	}
}

static void ps(void (*cb)(int, void*), void *arg) {
	DIR *dir;
	struct dirent *entry;

	if (!(dir = xopendir("/proc")))
		return;

	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR && is_num(entry->d_name))
			cb(atoi(entry->d_name), arg);
	}

	closedir(dir);
}

static bool check_proc_name(int pid, const char *name) {
	char buf[4019];
	FILE *f;
	sprintf(buf, "/proc/%d/comm", pid);
	if ((f = fopen(buf, "r"))) {
		fgets(buf, sizeof(buf), f);
		if (strcmp(buf, name) == 0)
			return true;
	} else {
		// The PID is already killed
		return false;
	}
	fclose(f);

	sprintf(buf, "/proc/%d/cmdline", pid);
	if ((f = fopen(buf, "r"))) {
		fgets(buf, sizeof(buf), f);
		if (strcmp(basename(buf), name) == 0)
			return true;
	} else {
		// The PID is already killed
		return false;
	}
	fclose(f);

	sprintf(buf, "/proc/%d/exe", pid);
	if (access(buf, F_OK) != 0)
		return false;
	xreadlink(buf, buf, sizeof(buf));
	return strcmp(basename(buf), name) == 0;
}

static void kill_proc_cb(int pid, void *v) {
	ps_arg *args = static_cast<ps_arg *>(v);
	if (check_proc_name(pid, args->name))
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
	ps_arg args = { .name = name };
	struct stat st;
	int fd = xopen("/data/data", O_RDONLY | O_CLOEXEC);
	if (fstatat(fd, name, &st, 0) == 0)
		args.uid = st.st_uid;
	else
		args.uid = 0;
	close(fd);
	ps(kill_proc_cb, &args);
}

void clean_magisk_props() {
	LOGD("hide_utils: Cleaning magisk props\n");
	getprop([](const char *name, auto, auto) -> void {
		if (strstr(name, "magisk"))
			deleteprop(name);
	}, nullptr, false);
}

static int add_list(sqlite3 *db, const char *proc) {
	for (auto &s : hide_list) {
		// They should be unique
		if (s == proc)
			return HIDE_ITEM_EXIST;
	}

	LOGI("hide_list add: [%s]\n", proc);

	// Critical region
	pthread_mutex_lock(&list_lock);
	hide_list.push_back(proc);
	kill_process(proc);
	pthread_mutex_unlock(&list_lock);

	// Add to database
	char sql[4096];
	snprintf(sql, sizeof(sql), "INSERT INTO hidelist (process) VALUES('%s')", proc);
	sqlite3_exec(db, sql, nullptr, nullptr, nullptr);

	return DAEMON_SUCCESS;
}

int add_list(const char *proc) {
	sqlite3 *db = get_magiskdb();
	if (db) {
		int ret = add_list(db, proc);
		sqlite3_close_v2(db);
		return ret;
	}
	return DAEMON_ERROR;
}

int add_list(int client) {
	char *proc = read_string(client);
	int ret = add_list(proc);
	free(proc);
	return ret;
}

static int rm_list(const char *proc) {
	// Update list in critical region
	bool do_rm = false;
	pthread_mutex_lock(&list_lock);
	for (auto it = hide_list.begin(); it != hide_list.end(); ++it) {
		if (*it == proc) {
			do_rm = true;
			LOGI("hide_list rm: [%s]\n", proc);
			hide_list.erase(it);
			kill_process(proc);
			break;
		}
	}
	pthread_mutex_unlock(&list_lock);

	if (do_rm) {
		sqlite3 *db = get_magiskdb();
		if (db == nullptr)
			return DAEMON_ERROR;
		char sql[4096];
		snprintf(sql, sizeof(sql), "DELETE FROM hidelist WHERE process='%s'", proc);
		sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
		sqlite3_close_v2(db);
		return DAEMON_SUCCESS;
	} else {
		return HIDE_ITEM_NOT_EXIST;
	}
}

int rm_list(int client) {
	char *proc = read_string(client);
	int ret = rm_list(proc);
	free(proc);
	return ret;
}

#define LEGACY_LIST MOUNTPOINT "/.core/hidelist"

bool init_list() {
	LOGD("hide_list: initialize\n");

	sqlite3 *db = get_magiskdb();
	if (db == nullptr)
		return false;

	sqlite3_exec(db, "SELECT process FROM hidelist",
				 [] (auto, auto, char **data, auto) -> int
				 {
				 	LOGI("hide_list: [%s]\n", data[0]);
				 	hide_list.push_back(data[0]);
				 	return 0;
				 }, nullptr, nullptr);

	// Migrate old hide list into database
	if (access(LEGACY_LIST, R_OK) == 0) {
		Array<CharArray> tmp;
		file_to_array(LEGACY_LIST, tmp);
		for (auto &s : tmp)
			add_list(db, s);
		unlink(LEGACY_LIST);
	}

	sqlite3_close_v2(db);
	return true;
}

void ls_list(int client) {
	write_int(client, DAEMON_SUCCESS);
	write_int(client, hide_list.size());
	for (auto &s : hide_list)
		write_string(client, s);
	close(client);
}
