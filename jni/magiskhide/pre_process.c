/* pre_process.c - Some pre-processes for MagiskHide to hide properly
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

static char *prop_key[] =
	{ "ro.boot.verifiedbootstate", "ro.boot.flash.locked", "ro.boot.veritymode", "ro.boot.warranty_bit", "ro.warranty_bit", 
	  "ro.debuggable", "ro.secure", NULL };

static char *prop_value[] =
	{ "green", "1", "enforcing", "0", "0", "0", "1", NULL };

static int isMocked = 0;

void manage_selinux() {
	if (isMocked) return;
	char val[1];
	int fd = xopen(ENFORCE_FILE, O_RDONLY);
	xxread(fd, val, 1);
	close(fd);
	// Permissive
	if (val[0] == '0') {
		LOGI("hide_daemon: Permissive detected, hide the state\n");

		chmod(ENFORCE_FILE, 0640);
		chmod(POLICY_FILE, 0440);
		isMocked = 1;
	}
}

void hide_sensitive_props() {
	LOGI("hide_pre_proc: Hiding sensitive props\n");

	// Hide all sensitive props
	init_resetprop();
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

void relink_sbin() {
	struct stat st;
	if (stat("/sbin_orig", &st) == -1 && errno == ENOENT) {
		// Re-link all binaries and bind mount 
		DIR *dir;
		struct dirent *entry;
		char from[PATH_MAX], to[PATH_MAX];

		LOGI("hide_pre_proc: Re-linking /sbin\n");

		xmount(NULL, "/", NULL, MS_REMOUNT, NULL);
		xrename("/sbin", "/sbin_orig");
		xmkdir("/sbin", 0755);
		xchmod("/sbin", 0755);
		xmount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL);
		xmkdir("/dev/sbin_bind", 0755);
		xchmod("/dev/sbin_bind", 0755);
		dir = xopendir("/sbin_orig");

		while ((entry = xreaddir(dir))) {
			snprintf(from, sizeof(from), "%s/%s", "/sbin_orig", entry->d_name);
			snprintf(to, sizeof(to), "%s/%s", "/dev/sbin_bind", entry->d_name);
			symlink(from, to);
			lsetfilecon(to, "u:object_r:system_file:s0");
		}
		
		closedir(dir);

		xmount("/dev/sbin_bind", "/sbin", NULL, MS_BIND, NULL);
	}
}
