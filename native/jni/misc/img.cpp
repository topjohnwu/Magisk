/* img.cpp - All image related functions
 */

#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#include <sys/statfs.h>
#include <sys/sysmacros.h>
#include <linux/loop.h>

#include "magisk.h"
#include "utils.h"
#include "img.h"
#include "flags.h"

#define round_size(a) ((((a) / 32) + 2) * 32)
#define SOURCE_TMP "/dev/.img_src"
#define TARGET_TMP "/dev/.img_tgt"
#define MERGE_TMP  "/dev/.img_mrg"

struct fs_info {
	unsigned size;
	unsigned free;
	unsigned used;
};

static char *loopsetup(const char *img) {
	char device[32];
	struct loop_info64 info;
	int lfd = -1, ffd;
	memset(&info, 0, sizeof(info));
	if (access(BLOCKDIR, F_OK) == 0) {
		for (int i = 8; i < 100; ++i) {
			sprintf(device, BLOCKDIR "/loop%02d", i);
			if (access(device, F_OK) != 0)
				mknod(device, S_IFBLK | 0600, makedev(7, i * 8));
			lfd = open(device, O_RDWR);
			if (lfd < 0) /* Kernel does not support this */
				break;
			if (ioctl(lfd, LOOP_GET_STATUS64, &info) == -1)
				break;
			close(lfd);
			lfd = -1;
		}
	}
	// Fallback to existing loop in dev, but in reverse order
	if (lfd < 0) {
		for (int i = 7; i >= 0; --i) {
			sprintf(device, "/dev/block/loop%d", i);
			lfd = xopen(device, O_RDWR);
			if (ioctl(lfd, LOOP_GET_STATUS64, &info) == -1)
				break;
			close(lfd);
			lfd = -1;
		}
	}
	if (lfd < 0)
		return NULL;
	ffd = xopen(img, O_RDWR);
	if (ioctl(lfd, LOOP_SET_FD, ffd) == -1)
		return NULL;
	strncpy((char *) info.lo_file_name, img, sizeof(info.lo_file_name));
	ioctl(lfd, LOOP_SET_STATUS64, &info);
	close(lfd);
	close(ffd);
	return strdup(device);
}

static void check_filesystem(struct fs_info *info, const char *img, const char *mount) {
	struct stat st;
	struct statfs fs;
	stat(img, &st);
	statfs(mount, &fs);
	info->size = st.st_size / 1048576;
	info->free = fs.f_bfree * (uint64_t)fs.f_frsize / 1048576;
	info->used = (fs.f_blocks - fs.f_bfree) * (uint64_t)fs.f_frsize / 1048576;
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
			"   merge  SRC TGT       merge SRC to TGT\n"
			"   trim   IMG           trim IMG to save space\n"
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
		return resize_img(argv[2], atoi(argv[3]));
	} else if (strcmp(argv[1], "mount") == 0) {
		if (argc < 4)
			usage();
		// Redirect 1 > /dev/null
		int fd = open("/dev/null", O_WRONLY);
		int out = dup(STDOUT_FILENO);
		xdup2(fd, STDOUT_FILENO);
		char *loop = mount_image(argv[2], argv[3]);
		// Restore stdin
		xdup2(out, STDOUT_FILENO);
		close(fd);
		close(out);
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
	} else if (strcmp(argv[1], "merge") == 0) {
		if (argc < 4)
			usage();
		return merge_img(argv[2], argv[3]);
	} else if (strcmp(argv[1], "trim") == 0) {
		if (argc < 3)
			usage();
		xmkdir(SOURCE_TMP, 0755);
		char *loop = mount_image(argv[2], SOURCE_TMP);
		int ret = trim_img(argv[2], SOURCE_TMP, loop);
		umount_image(SOURCE_TMP, loop);
		rmdir(SOURCE_TMP);
		free(loop);
		return ret;
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

int resize_img(const char *img, int size) {
	LOGI("Resize %s to %dM\n", img, size);
	exec_command_sync("/system/bin/e2fsck", "-yf", img, NULL);
	char ss[16];
	snprintf(ss, sizeof(ss), "%dM", size);
	return exec_command_sync("/system/bin/resize2fs", img, ss, NULL);
}

char *mount_image(const char *img, const char *target) {
	if (access(img, F_OK) == -1 || access(target, F_OK) == -1)
		return NULL;
	exec_command_sync("/system/bin/e2fsck", "-yf", img, NULL);
	char *device = loopsetup(img);
	if (device)
		xmount(device, target, "ext4", MS_NOATIME, NULL);
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
	if (access(target, F_OK) == -1) {
		LOGI("* Move %s  -> %s\n", source, target);
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

	char buf[PATH_MAX];

	xmkdir(SOURCE_TMP, 0755);
	xmkdir(TARGET_TMP, 0755);
	char *s_loop, *t_loop, *m_loop;
	s_loop = mount_image(source, SOURCE_TMP);
	if (s_loop == NULL)
		return 1;
	t_loop = mount_image(target, TARGET_TMP);
	if (t_loop == NULL)
		return 1;

	snprintf(buf, sizeof(buf), "%s/%s", SOURCE_TMP, "lost+found");
	rm_rf(buf);
	snprintf(buf, sizeof(buf), "%s/%s", TARGET_TMP, "lost+found");
	rm_rf(buf);
	DIR *dir;
	struct dirent *entry;
	if (!(dir = xopendir(SOURCE_TMP)))
		return 1;
	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				strcmp(entry->d_name, ".core") == 0)
				continue;
			// Cleanup old module if exists
			snprintf(buf, sizeof(buf), "%s/%s", TARGET_TMP, entry->d_name);
			if (access(buf, F_OK) == 0)
				rm_rf(buf);
			LOGI("Upgrade/New module: %s\n", entry->d_name);
		}
	}
	closedir(dir);

	struct fs_info src, tgt;
	check_filesystem(&src, source, SOURCE_TMP);
	check_filesystem(&tgt, target, TARGET_TMP);
	snprintf(buf, sizeof(buf), "%s/tmp.img", dirname(target));
	create_img(buf, round_size(src.used + tgt.used));
	xmkdir(MERGE_TMP, 0755);
	m_loop = mount_image(buf, MERGE_TMP);
	if (m_loop == NULL)
		return 1;

	LOGI("* Merging %s + %s -> %s", source, target, buf);
	cp_afc(TARGET_TMP, MERGE_TMP);
	cp_afc(SOURCE_TMP, MERGE_TMP);

	// Unmount all loop devices
	umount_image(SOURCE_TMP, s_loop);
	umount_image(TARGET_TMP, t_loop);
	umount_image(MERGE_TMP, m_loop);
	rmdir(SOURCE_TMP);
	rmdir(TARGET_TMP);
	rmdir(MERGE_TMP);
	free(s_loop);
	free(t_loop);
	free(m_loop);
	// Cleanup
	unlink(source);
	LOGI("* Move %s -> %s", buf, target);
	rename(buf, target);
	return 0;
}

int trim_img(const char *img, const char *mount, char *loop) {
	struct fs_info info;
	check_filesystem(&info, img, mount);
	int new_size = round_size(info.used);
	if (info.size > new_size) {
		umount_image(mount, loop);
		resize_img(img, new_size);
		char *loop2 = mount_image(img, mount);
		if (loop2 == NULL)
			return 1;
		strcpy(loop, loop2);
		free(loop2);
	}
	return 0;
}
