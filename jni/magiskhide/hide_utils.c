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

static void rm_magisk_prop(const char *name) {
	if (strstr(name, "magisk")) {
		deleteprop(name, 0);
	}
}

void clean_magisk_props() {
	LOGD("hide_utils: Cleaning magisk props\n");
	getprop_all(rm_magisk_prop);
}

void relink_sbin() {
    struct stat st;
    DIR *dir;
    struct dirent *entry;
    char from[PATH_MAX], to[PATH_MAX];

    if (stat("/sbin_orig", &st) == -1 && errno == ENOENT) {
	// Re-link all binaries and bind mount 
	LOGI("hide_utils: Re-linking /sbin (initial)\n");

	xmount(NULL, "/", NULL, MS_REMOUNT, NULL);

	clone_dir("/sbin", "/sbin_orig");
	xchmod("/sbin_orig", 0755);
	unlink("/sbin/magiskpolicy");
	unlink("/sbin/sepolicy-inject");
	unlink("/sbin/resetprop");
	unlink("/sbin/su");
	unlink("/sbin/supolicy");

	xmount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL);
	xmkdir("/dev/sbin_bind", 0755);
	xchmod("/dev/sbin_bind", 0755);
	dir = xopendir("/sbin_orig");

	while ((entry = xreaddir(dir))) {
	    if (strcmp(entry->d_name, "..") == 0)
		continue;
	    snprintf(from, sizeof(from), "/sbin_orig/%s", entry->d_name);
	    if (entry->d_type == DT_LNK)
		xreadlink(from, from, sizeof(from));
	    snprintf(to, sizeof(to), "/dev/sbin_bind/%s", entry->d_name);
	    symlink(from, to);
	    lsetfilecon(to, "u:object_r:rootfs:s0");
	}
	
	closedir(dir);

	xmount("/dev/sbin_bind", "/sbin", NULL, MS_BIND, NULL);
    }else
    if (stat("/dev/sbin_bind", &st) == -1 && errno == ENOENT) {
	// Re-link all binaries and bind mount 
	LOGI("hide_utils: Re-linking /sbin\n");

	xmount(NULL, "/", NULL, MS_REMOUNT, NULL);

	unlink("/sbin/magiskpolicy");
	unlink("/sbin/sepolicy-inject");
	unlink("/sbin/resetprop");
	unlink("/sbin/su");
	unlink("/sbin/supolicy");

	xmount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL);
	xmkdir("/dev/sbin_bind", 0755);
	xchmod("/dev/sbin_bind", 0755);
	dir = xopendir("/sbin_orig");

	while ((entry = xreaddir(dir))) {
	    if (strcmp(entry->d_name, "..") == 0)
		continue;
	    snprintf(from, sizeof(from), "/sbin_orig/%s", entry->d_name);
	    if (entry->d_type == DT_LNK)
		xreadlink(from, from, sizeof(from));
	    snprintf(to, sizeof(to), "/dev/sbin_bind/%s", entry->d_name);
	    symlink(from, to);
	    lsetfilecon(to, "u:object_r:rootfs:s0");
	}
	
	closedir(dir);

	xmount("/dev/sbin_bind", "/sbin", NULL, MS_BIND, NULL);
    }
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
	if (vector_to_file(HIDELIST, hide_list)) {
		pthread_mutex_unlock(&file_lock);
		return DAEMON_ERROR;
	}
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
		if (vector_to_file(HIDELIST, hide_list))
			ret = DAEMON_ERROR;
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
	err_handler = do_nothing;
	char *proc = read_string(client);
	// ack
	write_int(client, add_list(proc));
	close(client);
}

void rm_hide_list(int client) {
	err_handler = do_nothing;
	char *proc = read_string(client);
	// ack
	write_int(client, rm_list(proc));
	close(client);
}
