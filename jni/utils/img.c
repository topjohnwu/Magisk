/* img.c - All image related functions
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <linux/loop.h>

#include "magisk.h"
#include "utils.h"

static int e2fsck(const char *img) {
	// Check and repair ext4 image
	char buffer[128];
	int pid, fd = 0;
	char *const command[] = { "e2fsck", "-yf", (char *) img, NULL };
	pid = run_command(1, &fd, "/system/bin/e2fsck", command);
	if (pid < 0)
		return 1;
	while (fdgets(buffer, sizeof(buffer), fd))
		LOGD("magisk_img: %s", buffer);
	waitpid(pid, NULL, 0);
	close(fd);
	return 0;
}

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
	close(lfd);
	close(ffd);
	return strdup(device);
}

int create_img(const char *img, int size) {
	unlink(img);
	LOGI("Create %s with size %dM\n", img, size);
	// Create a temp file with the file contexts
	char file_contexts[] = "/magisk(/.*)? u:object_r:system_file:s0\n";
	// If not root, attempt to create in current diretory
	char *filename = getuid() == UID_ROOT ? "/dev/file_contexts_image" : "file_contexts_image";
	int pid, status, fd = xopen(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	xwrite(fd, file_contexts, sizeof(file_contexts));
	close(fd);

	char buffer[16];
	snprintf(buffer, sizeof(buffer), "%dM", size);
	char *const command[] = { "make_ext4fs", "-l", buffer, "-a", "/magisk", "-S", filename, (char *) img, NULL };
	pid = run_command(0, NULL, "/system/bin/make_ext4fs", command);
	if (pid < 0)
		return 1;
	waitpid(pid, &status, 0);
	unlink(filename);
	return WEXITSTATUS(status);
}

int get_img_size(const char *img, int *used, int *total) {
	if (access(img, R_OK) == -1)
		return 1;
	char buffer[PATH_MAX];
	int pid, fd = 0, status = 1;
	char *const command[] = { "e2fsck", "-n", (char *) img, NULL };
	pid = run_command(1, &fd, "/system/bin/e2fsck", command);
	if (pid < 0)
		return 1;
	while (fdgets(buffer, sizeof(buffer), fd)) {
		if (strstr(buffer, img)) {
			char *tok = strtok(buffer, ",");
			while(tok != NULL) {
				if (strstr(tok, "blocks")) {
					status = 0;
					break;
				}
				tok = strtok(NULL, ",");
			}
			if (status) continue;
			sscanf(tok, "%d/%d", used, total);
			*used = *used / 256 + 1;
			*total /= 256;
			break;
		}
	}
	close(fd);
	waitpid(pid, NULL, 0);
	return 0;
}

int resize_img(const char *img, int size) {
	LOGI("Resize %s to %dM\n", img, size);
	if (e2fsck(img))
		return 1;
	char buffer[128];
	int pid, status, fd = 0;
	snprintf(buffer, sizeof(buffer), "%dM", size);
	char *const command[] = { "resize2fs", (char *) img, buffer, NULL };
	pid = run_command(1, &fd, "/system/bin/resize2fs", command);
	if (pid < 0)
		return 1;
	while (fdgets(buffer, sizeof(buffer), fd))
		LOGD("magisk_img: %s", buffer);
	close(fd);
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

char *mount_image(const char *img, const char *target) {
	if (access(img, F_OK) == -1)
		return NULL;
	if (access(target, F_OK) == -1) {
		if (xmkdir(target, 0755) == -1) {
			xmount(NULL, "/", NULL, MS_REMOUNT, NULL);
			xmkdir(target, 0755);
			xmount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL);
		}
	}

	if (e2fsck(img))
		return NULL;

	char *device = loopsetup(img);
	if (device)
		xmount(device, target, "ext4", 0, NULL);
	return device;
}

void umount_image(const char *target, const char *device) {
	xumount(target);
	int fd = xopen(device, O_RDWR);
	ioctl(fd, LOOP_CLR_FD);
	close(fd);
}
