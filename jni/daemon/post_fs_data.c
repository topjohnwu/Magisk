/* post_fs_data.c - post-fs-data actions
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/loop.h>
#include <sys/ioctl.h>
#include <sys/mount.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"
#include "resetprop.h"

static char *loopsetup(const char *img) {
	char device[20];
	struct loop_info64 info;
	int i, lfd, ffd;
	memset(&info, 0, sizeof(info));
	// First get an empty loop device
	for (i = 0; i <= 7; ++i) {
		sprintf(device, "/dev/block/loop%d", i);
		lfd = xopen(device, O_RDWR);
		if (ioctl(lfd, LOOP_GET_STATUS64, &info) == -1)
			break;
		close(lfd);
	}
	if (i == 8) return NULL;
	ffd = xopen(img, O_RDWR);
	if (ioctl(lfd, LOOP_SET_FD, ffd) == -1)
		return NULL;
	strcpy((char *) info.lo_file_name, img);
	ioctl(lfd, LOOP_SET_STATUS64, &info);
	return strdup(device);
}

static void *start_magisk_hide(void *args) {
	// Setup default error handler for thread
	err_handler = exit_thread;
	launch_magiskhide(-1);
	return NULL;
}

char *mount_image(const char *img, const char *target) {
	char *device = loopsetup(img);
	if (device)
		xmount(device, target, "ext4", 0, NULL);
	return device;
}

void post_fs_data(int client) {
	// ack
	write_int(client, 0);
	close(client);
	if (!check_data())
		goto unblock;

	LOGI("** post-fs-data mode running\n");
	LOGI("* Mounting magisk.img\n");
	// Mounting magisk image
	char *magiskimg = mount_image("/data/magisk.img", "/magisk");
	free(magiskimg);

	// TODO: Magic Mounts, modules etc.

	// Run common scripts
	exec_common_script("post-fs-data");

	// Start magiskhide if enabled
	char *hide_prop = getprop("persist.magisk.hide");
	if (hide_prop) {
		if (strcmp(hide_prop, "1") == 0) {
			pthread_t thread;
			xpthread_create(&thread, NULL, start_magisk_hide, NULL);
		}
		free(hide_prop);
	}

unblock:
	unblock_boot_process();
}
