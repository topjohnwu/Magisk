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
	return strdup(device);
}

char *mount_image(const char *img, const char *target) {
	char *device = loopsetup(img);
	if (device)
		mount(device, target, "ext4", 0, NULL);
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

	// Start magiskhide if enabled
	char *hide_prop = getprop("persist.magisk.hide");
	if (hide_prop) {
		if (strcmp(hide_prop, "1") == 0)
			launch_magiskhide(-1);
		free(hide_prop);
	}

unblock:
	unblock_boot_process();
	return;
}
