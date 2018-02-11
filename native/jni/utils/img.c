/* img.c - All image related functions
 */

#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#include <linux/loop.h>

#include "magisk.h"
#include "utils.h"

static int e2fsck(const char *img) {
	// Check and repair ext4 image
	char buffer[128];
	int pid, fd = -1;
	pid = exec_command(1, &fd, NULL, "e2fsck", "-yf", img, NULL);
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
	if (size == 128)  /* WTF...? */
		size = 132;
	unlink(img);
	LOGI("Create %s with size %dM\n", img, size);
	int ret;

	char buffer[16];
	snprintf(buffer, sizeof(buffer), "%dM", size);
	ret = exec_command_sync("make_ext4fs", "-l", buffer, img, NULL);
	if (ret < 0)
		return 1;
	return ret;
}

int get_img_size(const char *img, int *used, int *total) {
	if (access(img, R_OK) == -1)
		return 1;
	char buffer[PATH_MAX];
	int pid, fd = -1, status = 1;
	pid = exec_command(1, &fd, NULL, "e2fsck", "-n", img, NULL);
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
			*used = (*used + 255) / 256;
			*total = (*total + 128) / 256;
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
	int pid, fd = -1, used, total;
	snprintf(buffer, sizeof(buffer), "%dM", size);
	pid = exec_command(1, &fd, NULL, "resize2fs", img, buffer, NULL);
	if (pid < 0)
		return 1;
	while (fdgets(buffer, sizeof(buffer), fd))
		LOGD("magisk_img: %s", buffer);
	close(fd);
	waitpid(pid, NULL, 0);

	// Double check our image size
	get_img_size(img, &used, &total);
	if (total != size) {
		// Sammy crap occurs or resize2fs failed, lets create a new image!
		char *dir = dirname(img);
		snprintf(buffer, sizeof(buffer), "%s/tmp.img", dir);
		create_img(buffer, size);
		char *s_loop, *t_loop;
		s_loop = mount_image(img, SOURCE_TMP);
		if (s_loop == NULL) return 1;
		t_loop = mount_image(buffer, TARGET_TMP);
		if (t_loop == NULL) return 1;

		cp_afc(SOURCE_TMP, TARGET_TMP);
		umount_image(SOURCE_TMP, s_loop);
		umount_image(TARGET_TMP, t_loop);
		rmdir(SOURCE_TMP);
		rmdir(TARGET_TMP);
		free(s_loop);
		free(t_loop);
		rename(buffer, img);
	}

	return 0;
}

char *mount_image(const char *img, const char *target) {
	if (access(img, F_OK) == -1)
		return NULL;
	if (access(target, F_OK) == -1) {
		if (xmkdirs(target, 0755) == -1) {
			xmount(NULL, "/", NULL, MS_REMOUNT, NULL);
			xmkdirs(target, 0755);
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

int merge_img(const char *source, const char *target) {
	if (access(source, F_OK) == -1)
		return 0;
	LOGI("* Merging %s  -> %s\n", source, target);
	if (access(target, F_OK) == -1) {
		if (rename(source, target) < 0) {
			// Copy and remove
			int tgt = creat(target, 0644);
			int src = xopen(source, O_RDONLY | O_CLOEXEC);
			sendfile(tgt, src, 0, INT_MAX);
			close(tgt);
			close(src);
			unlink(source);
		}
		return 0;
	}

	char buffer[PATH_MAX];

	// resize target to worst case
	int s_used, s_total, t_used, t_total, n_total;
	get_img_size(source, &s_used, &s_total);
	get_img_size(target, &t_used, &t_total);
	n_total = round_size(s_used + t_used);
	if (n_total > t_total)
		resize_img(target, n_total);

	xmkdir(SOURCE_TMP, 0755);
	xmkdir(TARGET_TMP, 0755);
	char *s_loop, *t_loop;
	s_loop = mount_image(source, SOURCE_TMP);
	if (s_loop == NULL) return 1;
	t_loop = mount_image(target, TARGET_TMP);
	if (t_loop == NULL) return 1;

	DIR *dir;
	struct dirent *entry;
	if (!(dir = xopendir(SOURCE_TMP)))
		return 1;
	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				strcmp(entry->d_name, ".core") == 0 ||
				strcmp(entry->d_name, "lost+found") == 0)
				continue;
			// Cleanup old module if exists
			snprintf(buffer, sizeof(buffer), "%s/%s", TARGET_TMP, entry->d_name);
			if (access(buffer, F_OK) == 0) {
				LOGI("Upgrade module: %s\n", entry->d_name);
				rm_rf(buffer);
			} else {
				LOGI("New module: %s\n", entry->d_name);
			}
		}
	}
	closedir(dir);
	cp_afc(SOURCE_TMP, TARGET_TMP);

	// Unmount all loop devices
	umount_image(SOURCE_TMP, s_loop);
	umount_image(TARGET_TMP, t_loop);
	rmdir(SOURCE_TMP);
	rmdir(TARGET_TMP);
	free(s_loop);
	free(t_loop);
	unlink(source);
	return 0;
}

void trim_img(const char *img) {
	int used, total, new_size;
	get_img_size(img, &used, &total);
	new_size = round_size(used);
	if (new_size != total)
		resize_img(img, new_size);
}
