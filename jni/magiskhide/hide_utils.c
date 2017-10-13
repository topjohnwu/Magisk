/* hide_utils.c - Some utility functions for MagiskHide
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"
#include "resetprop.h"
#include "magiskhide.h"
#include "daemon.h"

static char *prop_key[] =
	{ "ro.boot.verifiedbootstate", "ro.boot.flash.locked", "ro.boot.veritymode", "ro.boot.warranty_bit", "ro.warranty_bit",
	  "ro.debuggable", "ro.secure", "ro.build.type", "ro.build.tags", "ro.build.selinux", NULL };

static char *prop_value[] =
	{ "green", "1", "enforcing", "0", "0", "0", "1", "user", "release-keys", "0", NULL };

static int mocked = 0;

void manage_selinux() {
	if (mocked) return;
	char val[1];
	int fd = xopen(SELINUX_ENFORCE, O_RDONLY);
	xxread(fd, val, 1);
	close(fd);
	// Permissive
	if (val[0] == '0') {
		LOGI("hide_daemon: Permissive detected, hide the state\n");

		chmod(SELINUX_ENFORCE, 0640);
		chmod(SELINUX_POLICY, 0440);
		mocked = 1;
	}
}

void hide_sensitive_props() {
	LOGI("hide_utils: Hiding sensitive props\n");

	// Hide all sensitive props
	char *value;
	for (int i = 0; prop_key[i]; ++i) {
		value = getprop(prop_key[i]);
		if (value) {
			if (strcmp(value, prop_value[i]) != 0)
				setprop2(prop_key[i], prop_value[i], 0);
			free(value);
		}
	}
}

static void rm_magisk_prop(const char *name, const char *value) {
	if (strstr(name, "magisk")) {
		deleteprop2(name, 0);
	}
}

void clean_magisk_props() {
	LOGD("hide_utils: Cleaning magisk props\n");
	getprop_all(rm_magisk_prop);
}

int add_list(char *proc) {
	if (!hideEnabled) {
		free(proc);
		return HIDE_NOT_ENABLED;
	}

	char *line;
	struct vector *new_list = xmalloc(sizeof(*new_list));
	if (new_list == NULL)
		return DAEMON_ERROR;
	vec_init(new_list);

	vec_for_each(hide_list, line) {
		// They should be unique
		if (strcmp(line, proc) == 0) {
			free(proc);
			vec_destroy(new_list);
			free(new_list);
			return HIDE_ITEM_EXIST;
		}
		vec_push_back(new_list, line);
	}

	vec_push_back(new_list, proc);
	LOGI("hide_list add: [%s]\n", proc);
	ps_filter_proc_name(proc, kill_proc);

	// Critical region
	pthread_mutex_lock(&hide_lock);
	vec_destroy(hide_list);
	free(hide_list);
	hide_list = new_list;
	pthread_mutex_unlock(&hide_lock);

	pthread_mutex_lock(&file_lock);
	vector_to_file(HIDELIST, hide_list); // Do not complain if file not found
	pthread_mutex_unlock(&file_lock);
	return DAEMON_SUCCESS;
}

int rm_list(char *proc) {
	if (!hideEnabled) {
		free(proc);
		return HIDE_NOT_ENABLED;
	}

	daemon_response ret = DAEMON_ERROR;
	char *line;
	int do_rm = 0;
	struct vector *new_list = xmalloc(sizeof(*new_list));
	if (new_list == NULL)
		goto error;
	vec_init(new_list);

	vec_for_each(hide_list, line) {
		if (strcmp(line, proc) == 0) {
			free(proc);
			proc = line;
			do_rm = 1;
			continue;
		}
		vec_push_back(new_list, line);
	}

	if (do_rm) {
		LOGI("hide_list rm: [%s]\n", proc);
		ps_filter_proc_name(proc, kill_proc);
		// Critical region
		pthread_mutex_lock(&hide_lock);
		vec_destroy(hide_list);
		free(hide_list);
		hide_list = new_list;
		pthread_mutex_unlock(&hide_lock);

		ret = DAEMON_SUCCESS;
		pthread_mutex_lock(&file_lock);
		vector_to_file(HIDELIST, hide_list); // Do not complain if file not found
		pthread_mutex_unlock(&file_lock);
	} else {
		ret = HIDE_ITEM_NOT_EXIST;
		vec_destroy(new_list);
		free(new_list);
	}

error:
	free(proc);
	return ret;
}

int init_list() {
	LOGD("hide_list: initialize...\n");
	if ((hide_list = xmalloc(sizeof(*hide_list))) == NULL)
		return 1;
	vec_init(hide_list);

	// Might error if file doesn't exist, no need to report
	file_to_vector(HIDELIST, hide_list);

	char *line;
	vec_for_each(hide_list, line) {
		LOGI("hide_list: [%s]\n", line);
		ps_filter_proc_name(line, kill_proc);
	}
	return 0;
}

int destroy_list() {
	char *line;
	vec_for_each(hide_list, line) {
		ps_filter_proc_name(line, kill_proc);
	}
	vec_deep_destroy(hide_list);
	free(hide_list);
	hide_list = NULL;
	return 0;
}

void add_hide_list(int client) {
	char *proc = read_string(client);
	// ack
	write_int(client, add_list(proc));
	close(client);
}

void rm_hide_list(int client) {
	char *proc = read_string(client);
	// ack
	write_int(client, rm_list(proc));
	close(client);
}

void ls_hide_list(int client) {
	if (!hideEnabled) {
		write_int(client, HIDE_NOT_ENABLED);
		return;
	}
	write_int(client, DAEMON_SUCCESS);
	write_int(client, vec_size(hide_list));
	char *s;
	vec_for_each(hide_list, s) {
		write_string(client, s);
	}
	close(client);
}
