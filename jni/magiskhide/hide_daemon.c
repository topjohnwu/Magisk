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

static int pid;

static void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		LOGI("hide_daemon: Unmounted (%s)\n", mountpoint);
	else
		LOGI("hide_daemon: Unmount Failed (%s)\n", mountpoint);
}

static void hide_daemon_err() {
	LOGD("hide_daemon: error occured, stopping magiskhide services\n");
	// Resume process if possible
	kill(pid, SIGCONT);
	int err = 1;
	write(sv[1], &err, sizeof(err));
	_exit(-1);
}

int hide_daemon() {
	// Fork to a new process
	hide_pid = fork();
	switch(hide_pid) {
	case -1:
		PLOGE("fork");
		return 1;
	case 0:
		break;
	default:
		return 0;
	}

	close(sv[0]);

	// Set the process name
	strcpy(argv0, "magiskhide_daemon");
	// When an error occurs, report its failure to main process
	err_handler = hide_daemon_err;
	
	int fd;
	FILE *fp;
	char buffer[4096], cache_block[256], *line;
	struct vector mount_list;
	
	cache_block[0] = '\0';

	while(1) {
		xxread(sv[1], &pid, sizeof(pid));
		// Termination called
		if(pid == -1) {
			LOGD("hide_daemon: received termination request\n");
			_exit(0);
		}

		manage_selinux();
		relink_sbin();

		snprintf(buffer, sizeof(buffer), "/proc/%d/ns/mnt", pid);
		if(access(buffer, F_OK) == -1) continue; // Maybe process died..

		fd = xopen(buffer, O_RDONLY);
		// Switch to its namespace
		xsetns(fd, 0);
		close(fd);

		snprintf(buffer, sizeof(buffer), "/proc/%d/mounts", pid);
		vec_init(&mount_list);
		file_to_vector(buffer, &mount_list);

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
				sscanf(line, "%*s %512s", buffer);
				lazy_unmount(buffer);
			}
			free(line);
		}
		vec_destroy(&mount_list);

		// Re-read mount infos
		snprintf(buffer, sizeof(buffer), "/proc/%d/mounts", pid);
		vec_init(&mount_list);
		file_to_vector(buffer, &mount_list);

		// Unmount loop mounts
		vec_for_each_r(&mount_list, line) {
			if (strstr(line, "/dev/block/loop") && !strstr(line, DUMMYPATH)) {
				sscanf(line, "%*s %512s", buffer);
				lazy_unmount(buffer);
			}
			free(line);
		}
		vec_destroy(&mount_list);

		// All done, send resume signal
		kill(pid, SIGCONT);

		// Tell main process all is well
		pid = 0;
		xwrite(sv[1], &pid, sizeof(pid));
	}

	// Should never go here
}
