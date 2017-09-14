/* magiskinit.c - Workaround for skip_initramfs devices
 *
 * This code has to be compiled statically to work properly.
 *
 * Magiskinit will mount sysfs, parse through uevent files to make the system block device,
 * then it'll mount the system partition and clone rootfs except files under /system.
 * Folders placed in "overlay" will then be overlayed to the root.
 * Lastly before giving control back to the real init, it'll patch the root files to load Magisk.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/sysmacros.h>

struct cmdline {
	int skip_initramfs;
	char slot[3];
};

struct device {
	dev_t major;
	dev_t minor;
	char devname[32];
	char partname[32];
	char path[64];
};

extern void mmap_ro(const char *filename, void **buf, size_t *size);
extern void mmap_rw(const char *filename, void **buf, size_t *size);
extern void *patch_init_rc(char *data, uint32_t *size);

static void clone_dir(int src, int dest) {
	struct dirent *entry;
	DIR *dir;
	int srcfd, destfd, newsrc, newdest;
	struct stat st;
	char buf[PATH_MAX];
	ssize_t size;

	dir = fdopendir(src);
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		fstatat(src, entry->d_name, &st, AT_SYMLINK_NOFOLLOW);
		switch (entry->d_type) {
		case DT_DIR:
			mkdirat(dest, entry->d_name, st.st_mode & 0777);
			fchownat(dest, entry->d_name, st.st_uid, st.st_gid, 0);
			// Don't clone recursive if it's /system
			if (strcmp(entry->d_name, "system") == 0)
				continue;
			newsrc = openat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			newdest = openat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
			clone_dir(newsrc, newdest);
			close(newsrc);
			close(newdest);
			break;
		case DT_REG:
			destfd = openat(dest, entry->d_name, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, st.st_mode & 0777);
			srcfd = openat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			sendfile(destfd, srcfd, 0, st.st_size);
			fchownat(dest, entry->d_name, st.st_uid, st.st_gid, 0);
			close(destfd);
			close(srcfd);
			break;
		case DT_LNK:
			size = readlinkat(src, entry->d_name, buf, sizeof(buf));
			buf[size] = '\0';
			symlinkat(buf, dest, entry->d_name);
			fchownat(dest, entry->d_name, st.st_uid, st.st_gid, AT_SYMLINK_NOFOLLOW);
			break;
		}
	}
}

static void mv_dir(int src, int dest) {
	struct dirent *entry;
	DIR *dir;
	int newsrc, newdest;
	struct stat st;
	char buf[PATH_MAX];
	ssize_t size;

	dir = fdopendir(src);
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		fstatat(src, entry->d_name, &st, AT_SYMLINK_NOFOLLOW);
		switch (entry->d_type) {
		case DT_DIR:
			mkdirat(dest, entry->d_name, st.st_mode & 0777);
			fchownat(dest, entry->d_name, st.st_uid, st.st_gid, 0);
			newsrc = openat(src, entry->d_name, O_RDONLY | O_CLOEXEC);
			newdest = openat(dest, entry->d_name, O_RDONLY | O_CLOEXEC);
			mv_dir(newsrc, newdest);
			close(newsrc);
			close(newdest);
			break;
		case DT_REG:
			renameat(src, entry->d_name, dest, entry->d_name);
			fchmodat(dest, entry->d_name, st.st_mode & 0777, 0);
			fchownat(dest, entry->d_name, st.st_uid, st.st_gid, 0);
			break;
		case DT_LNK:
			size = readlinkat(src, entry->d_name, buf, sizeof(buf));
			buf[size] = '\0';
			symlinkat(buf, dest, entry->d_name);
			fchownat(dest, entry->d_name, st.st_uid, st.st_gid, AT_SYMLINK_NOFOLLOW);
			break;
		}
		unlinkat(src, entry->d_name, AT_REMOVEDIR);
	}
}

static void rm_rf(int path) {
	struct dirent *entry;
	int newfd;
	DIR *dir = fdopendir(path);

	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		switch (entry->d_type) {
		case DT_DIR:
			// Preserve overlay
			if (strcmp(entry->d_name, "overlay") == 0)
				continue;
			newfd = openat(path, entry->d_name, O_RDONLY | O_CLOEXEC);
			rm_rf(newfd);
			close(newfd);
			unlinkat(path, entry->d_name, AT_REMOVEDIR);
			break;
		default:
			unlinkat(path, entry->d_name, 0);
			break;
		}
	}
}

static void parse_cmdline(struct cmdline *cmd) {
	char *tok;
	char buffer[4096];
	mkdir("/proc", 0555);
	mount("proc", "/proc", "proc", 0, NULL);
	int fd = open("/proc/cmdline", O_RDONLY | O_CLOEXEC);
	ssize_t size = read(fd, buffer, sizeof(buffer));
	buffer[size] = '\0';
	close(fd);
	umount("/proc");
	tok = strtok(buffer, " ");
	cmd->skip_initramfs = 0;
	cmd->slot[0] = '\0';
	while (tok != NULL) {
		if (strncmp(tok, "androidboot.slot_suffix", 23) == 0) {
			sscanf(tok, "androidboot.slot_suffix=%s", cmd->slot);
		} else if (strcmp(tok, "skip_initramfs") == 0) {
			cmd->skip_initramfs = 1;
			break;
		}
		tok = strtok(NULL, " ");
	}
}

static void parse_device(struct device *dev, char *uevent) {
	char *tok;
	tok = strtok(uevent, "\n");
	while (tok != NULL) {
		if (strncmp(tok, "MAJOR", 5) == 0) {
			sscanf(tok, "MAJOR=%ld", (long*) &dev->major);
		} else if (strncmp(tok, "MINOR", 5) == 0) {
			sscanf(tok, "MINOR=%ld", (long*) &dev->minor);
		} else if (strncmp(tok, "DEVNAME", 7) == 0) {
			sscanf(tok, "DEVNAME=%s", dev->devname);
		} else if (strncmp(tok, "PARTNAME", 8) == 0) {
			sscanf(tok, "PARTNAME=%s", dev->partname);
		}
		tok = strtok(NULL, "\n");
	}
}

static int setup_block(struct device *dev, const char *partname) {
	char buffer[1024], path[128];
	mkdir("/sys", 0755);
	mount("sysfs", "/sys", "sysfs", 0, NULL);
	struct dirent *entry;
	DIR *dir = opendir("/sys/dev/block");
	if (dir == NULL)
		return 1;
	int found = 0;
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		snprintf(path, sizeof(path), "/sys/dev/block/%s/uevent", entry->d_name);
		int fd = open(path, O_RDONLY | O_CLOEXEC);
		ssize_t size = read(fd, buffer, sizeof(buffer));
		buffer[size] = '\0';
		close(fd);
		parse_device(dev, buffer);
		if (strcmp(dev->partname, partname) == 0) {
			snprintf(dev->path, sizeof(dev->path), "/dev/block/%s", dev->devname);
			found = 1;
			break;
		}
	}
	closedir(dir);

	if (!found)
		return 1;

	mkdir("/dev", 0755);
	mkdir("/dev/block", 0755);
	mknod(dev->path, S_IFBLK | 0600, makedev(dev->major, dev->minor));
	return 0;
}

static void patch_ramdisk() {
	void *addr;
	size_t size;
	mmap_rw("/init", &addr, &size);
	for (int i = 0; i < size; ++i) {
		if (memcmp(addr + i, "/system/etc/selinux/plat_sepolicy.cil", 37) == 0) {
			memcpy(addr + i, "/system/etc/selinux/plat_sepolicy.xxx", 37);
			break;
		}
	}
	munmap(addr, size);
	mmap_rw("/init.rc", &addr, &size);
	uint32_t new_size = size;
	void *init_rc = patch_init_rc(addr, &new_size);
	munmap(addr, size);
	int fd = open("/init.rc", O_WRONLY | O_TRUNC | O_CLOEXEC);
	write(fd, init_rc, new_size);
	close(fd);
	free(init_rc);
}

int main(int argc, char *argv[]) {
	umask(0);

	struct cmdline cmd;
	parse_cmdline(&cmd);

	if (cmd.skip_initramfs) {
		// Normal boot mode
		// Clear rootfs
		int root = open("/", O_RDONLY | O_CLOEXEC);
		rm_rf(root);

		char partname[32];
		snprintf(partname, sizeof(partname), "system%s", cmd.slot);

		struct device dev;
		setup_block(&dev, partname);

		mkdir("/system_root", 0755);
		mount(dev.path, "/system_root", "ext4", MS_RDONLY, NULL);
		int system_root = open("/system_root", O_RDONLY | O_CLOEXEC);
		clone_dir(system_root, root);
		mount("/system_root/system", "/system", NULL, MS_BIND, NULL);

		int overlay = open("/overlay", O_RDONLY | O_CLOEXEC);
		if (overlay > 0)
			mv_dir(overlay, root);

		patch_ramdisk();

		close(root);
		close(system_root);
		close(overlay);
		rmdir("/overlay");
	} else {
		// Recovery mode
		// Revert original init binary
		unlink("/init");
		rename("/.backup/init", "/init");
	}

	execv("/init", argv);

	return 0;
}
