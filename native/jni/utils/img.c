/* img.c - All image related functions
 */

#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#include <sys/statvfs.h>
#include <linux/loop.h>

#include "magisk.h"
#include "utils.h"

struct fs_info {
	unsigned size;
	unsigned free;
	unsigned used;
};

static int e2fsck(const char *img) {
	// Check and repair ext4 image
	char buffer[128];
	int pid, fd = -1;
	pid = exec_command(1, &fd, NULL, "/system/bin/e2fsck", "-yf", img, NULL);
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

static void check_filesystem(struct fs_info *info, const char *img, const char *mount) {
	struct stat st;
	struct statvfs vfs;
	stat(img, &st);
	statvfs(mount, &vfs);
	info->size = st.st_size / 1048576;
	info->free = vfs.f_bfree * vfs.f_frsize / 1048576;
	info->used = (vfs.f_blocks - vfs.f_bfree) * vfs.f_frsize / 1048576;
}

static void usage() {
	fprintf(stderr,
			"ImgTool v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu) - EXT4 Image Tools\n"
			"\n"
			"Usage: imgtool <action> [args...]\n"
			"\n"
			"Actions:\n"
			"   create IMG SIZE      create ext4 image. SIZE is interpreted in MB\n"
			"   resize IMG SIZE      resize ext4 image. SIZE is interpreted in MB\n"
			"   mount  IMG PATH      mount IMG to PATH and prints the loop device\n"
			"   umount PATH LOOP     unmount PATH and delete LOOP device\n"
	);
	exit(1);
}

int imgtool_main(int argc, char *argv[]) {
	if (argc < 2)
		usage();
	if (strcmp(argv[1], "create") == 0) {
		if (argc < 4)
			usage();
		return create_img(argv[2], atoi(argv[3]));
	} else if (strcmp(argv[1], "resize") == 0) {
		if (argc < 4)
			usage();
		return resize_img(argv[2], atoi(argv[3]), 1);
	} else if (strcmp(argv[1], "mount") == 0) {
		if (argc < 4)
			usage();
		char *loop = mount_image(argv[2], argv[3]);
		if (loop == NULL) {
			fprintf(stderr, "Cannot mount image!\n");
			return 1;
		} else {
			printf("%s\n", loop);
			free(loop);
			return 0;
		}
	} else if (strcmp(argv[1], "umount") == 0) {
		if (argc < 4)
			usage();
		umount_image(argv[2], argv[3]);
		return 0;
	}
	usage();
	return 1;
}

int create_img(const char *img, int size) {
	if (size == 128)  /* WTF...? */
		size = 132;
	unlink(img);
	LOGI("Create %s with size %dM\n", img, size);
	char size_str[16];
	snprintf(size_str, sizeof(size_str), "%dM", size);
	if (access("/system/bin/make_ext4fs", X_OK) == 0)
		return exec_command_sync("/system/bin/make_ext4fs", "-b", "4096", "-l", size_str, img, NULL);
	else if (access("/system/bin/mke2fs", X_OK) == 0)
		// On Android P there is no make_ext4fs, use mke2fs
		return exec_command_sync("/system/bin/mke2fs", "-b", "4096", "-t", "ext4", img, size_str, NULL);
	else
		return 1;
}

int resize_img(const char *img, int size, int enforce) {
	LOGI("Resize %s to %dM\n", img, size);
	if (e2fsck(img))
		return 1;
	char buffer[128];
	int pid, fd = -1;
	snprintf(buffer, sizeof(buffer), "%dM", size);
	pid = exec_command(1, &fd, NULL, "/system/bin/resize2fs", img, buffer, NULL);
	if (pid < 0)
		return 1;
	while (fdgets(buffer, sizeof(buffer), fd))
		LOGD("magisk_img: %s", buffer);
	close(fd);
	waitpid(pid, NULL, 0);

	if (enforce) {
		// Check the image size
		struct stat st;
		stat(img, &st);
		if (st.st_size / 1048576 != size) {
			// Sammy crap occurs or resize2fs failed, lets create a new image!
			snprintf(buffer, sizeof(buffer), "%s/tmp.img", dirname(img));
			create_img(buffer, size);
			char *s_loop, *t_loop;
			s_loop = mount_image(img, SOURCE_TMP);
			if (s_loop == NULL)
				return 1;
			t_loop = mount_image(buffer, TARGET_TMP);
			if (t_loop == NULL)
				return 1;

			cp_afc(SOURCE_TMP, TARGET_TMP);
			umount_image(SOURCE_TMP, s_loop);
			umount_image(TARGET_TMP, t_loop);
			rmdir(SOURCE_TMP);
			rmdir(TARGET_TMP);
			free(s_loop);
			free(t_loop);
			rename(buffer, img);
		}
	}
	return 0;
}

char *mount_image(const char *img, const char *target) {
	if (access(img, F_OK) == -1 || access(target, F_OK) == -1)
		return NULL;

	if (e2fsck(img))
		return NULL;

	char *device = loopsetup(img);
	if (device)
		xmount(device, target, "ext4", 0, NULL);
	return device;
}

int umount_image(const char *target, const char *device) {
	int ret = 0;
	ret |= xumount(target);
	int fd = xopen(device, O_RDWR);
	ret |= ioctl(fd, LOOP_CLR_FD);
	close(fd);
	return ret;
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

	xmkdir(SOURCE_TMP, 0755);
	xmkdir(TARGET_TMP, 0755);
	char *s_loop, *t_loop;
	s_loop = mount_image(source, SOURCE_TMP);
	if (s_loop == NULL)
		return 1;
	t_loop = mount_image(target, TARGET_TMP);
	if (t_loop == NULL)
		return 1;

	struct fs_info src, tgt;
	check_filesystem(&src, source, SOURCE_TMP);
	check_filesystem(&tgt, target, TARGET_TMP);

	// resize target to worst case
	if (src.used >= tgt.free) {
		umount_image(TARGET_TMP, t_loop);
		free(t_loop);
		resize_img(target, round_size(tgt.size + src.used - tgt.free), 1);
		t_loop = mount_image(target, TARGET_TMP);
	}

	snprintf(buffer, sizeof(buffer), "%s/%s", TARGET_TMP, "lost+found");
	rm_rf(buffer);
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
			if (access(buffer, F_OK) == 0)
				rm_rf(buffer);
			LOGI("Upgrade/New module: %s\n", entry->d_name);
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

int trim_img(const char *img, const char *mount, char *loop) {
	struct fs_info info;
	check_filesystem(&info, img, mount);
	int new_size = round_size(info.used);
	if (info.size > new_size) {
		umount_image(mount, loop);
		free(loop);
		resize_img(img, new_size, 0);
		loop = mount_image(img, mount);
		if (loop == NULL)
			return 1;
	}
	free(loop);
	return 0;
}
