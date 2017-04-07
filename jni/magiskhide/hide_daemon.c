/* hide_daemon.c - MagiskHide daemon
 *
 * A dedicated process to join the target namespace,
 * and hide all traces in that particular namespace
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include "magisk.h"
#include "utils.h"
#include "magiskhide.h"

static int isMocked = 0;

static void manage_selinux() {
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

static void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		LOGI("hide_daemon: Unmounted (%s)\n", mountpoint);
	else
		LOGI("hide_daemon: Unmount Failed (%s)\n", mountpoint);
}

void hide_daemon() {
	// Fork to a new process
	switch(fork()) {
	case -1:
		PLOGE("fork");
	case 0:
		break;
	default:
		return;
	}

	close(pipefd[1]);

	// Set the process name
	strcpy(argv0, "magiskhide_daemon");
	
	int pid, fd;
	FILE *fp;
	char cache_block[256], *line;
	struct vector mount_list;
	
	cache_block[0] = '\0';

	while(1) {
		xxread(pipefd[0], &pid, sizeof(pid));
		// Termination called
		if(pid == -1) exit(0);

		snprintf(magiskbuf, BUF_SIZE, "/proc/%d/ns/mnt", pid);
		if(access(magiskbuf, F_OK) == -1) continue; // Maybe process died..

		fd = xopen(magiskbuf, O_RDONLY);
		// Switch to its namespace
		xsetns(fd, 0);
		close(fd);

		manage_selinux();

		snprintf(magiskbuf, BUF_SIZE, "/proc/%d/mounts", pid);
		fp = xfopen(magiskbuf, "r");
		vec_init(&mount_list);
		file_to_vector(&mount_list, fp);

		// Find the cache block name if not found yet
		if (strlen(cache_block) == 0) {
			vec_for_each(&mount_list, line) {
				if (strstr(line, " /cache ")) {
					sscanf(line, "%256s", cache_block);
					break;
				}
			}
		}
		
		// First unmount the dummy skeletons, cache mounts, and /sbin links
		vec_for_each_r(&mount_list, line) {
			if (strstr(line, "tmpfs /system") || strstr(line, "tmpfs /vendor") || strstr(line, "tmpfs /sbin")
				|| (strstr(line, cache_block) && strstr(line, "/system/")) ) {
				sscanf(line, "%*s %512s", magiskbuf);
				lazy_unmount(magiskbuf);
			}
			free(line);
		}
		vec_destroy(&mount_list);

		// Re-read mount infos
		fseek(fp, 0, SEEK_SET);
		vec_init(&mount_list);
		file_to_vector(&mount_list, fp);
		fclose(fp);

		// Unmount loop mounts
		vec_for_each_r(&mount_list, line) {
			if (strstr(line, "/dev/block/loop") && !strstr(line, DUMMYPATH)) {
				sscanf(line, "%*s %512s", magiskbuf);
				lazy_unmount(magiskbuf);
			}
			free(line);
		}
		vec_destroy(&mount_list);

		// All done, send resume signal
		kill(pid, SIGCONT);
	}

	// Should never go here
}
