/* magiskinit.c - Workaround for skip_initramfs devices
 *
 * This code has to be compiled statically to work properly.
 *
 * Magiskinit will mount sysfs, parse through uevent files to make the system block device,
 * then it'll mount the system partition and clone rootfs except files under /system.
 * Folders placed in "overlay" will then be overlayed to the root.
 * Lastly, before giving control back to the real init, it'll patch the root files,
 * extract (or compile if needed) sepolicy and patch it to load Magisk.
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

#include <cil/cil.h>

#include "utils.h"
#include "magiskpolicy.h"

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

extern policydb_t *policydb;

extern void mmap_ro(const char *filename, void **buf, size_t *size);
extern void mmap_rw(const char *filename, void **buf, size_t *size);
extern void *patch_init_rc(char *data, uint32_t *size);

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

static int strend(const char *s1, const char *s2) {
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	return strcmp(s1 + l1 - l2, s2);
}

static void patch_sepolicy() {
	DIR *dir;
	struct dirent *entry;
	char *sepolicy = NULL, path[128];
	if (access("/system_root/sepolicy", R_OK) == 0)
		sepolicy = "/system_root/sepolicy";
	if (sepolicy == NULL && access("/vendor/etc/selinux/precompiled_sepolicy", R_OK) == 0) {
		void *sys_sha = NULL, *ven_sha = NULL;
		size_t sys_size = 0, ven_size = 0;
		if ((dir = opendir("/vendor/etc/selinux")) == NULL)
			goto check_done;
		while ((entry = readdir(dir))) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			if (strend(entry->d_name, ".sha256") == 0) {
				snprintf(path, sizeof(path), "/vendor/etc/selinux/%s", entry->d_name);
				mmap_ro(path, &ven_sha, &ven_size);
				break;
			}
		}
		closedir(dir);
		if ((dir = opendir("/system/etc/selinux")) == NULL)
			goto check_done;
		while ((entry = readdir(dir))) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			if (strend(entry->d_name, ".sha256") == 0) {
				snprintf(path, sizeof(path), "/system/etc/selinux/%s", entry->d_name);
				mmap_ro(path, &sys_sha, &sys_size);
				break;
			}
		}
		closedir(dir);
		if (sys_size == ven_size && memcmp(sys_sha, ven_sha, sys_size) == 0)
			sepolicy = "/vendor/etc/selinux/precompiled_sepolicy";
		munmap(sys_sha, sys_size);
		munmap(ven_sha, ven_size);
	}

check_done:

	if (sepolicy) {
		load_policydb(sepolicy);
	} else {
		// Compile cil
		struct cil_db *db = NULL;
		sepol_policydb_t *pdb = NULL;
		void *addr;
		size_t size;

		cil_db_init(&db);
		cil_set_mls(db, 1);
		cil_set_target_platform(db, SEPOL_TARGET_SELINUX);
		cil_set_policy_version(db, POLICYDB_VERSION_XPERMS_IOCTL);
		cil_set_attrs_expand_generated(db, 0);

		mmap_ro("/system/etc/selinux/plat_sepolicy.cil", &addr, &size);
		cil_add_file(db, "/system/etc/selinux/plat_sepolicy.cil", addr, size);
		munmap(addr, size);

		dir = opendir("/system/etc/selinux/mapping");
		while ((entry = readdir(dir))) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			if (strend(entry->d_name, ".cil") == 0) {
				snprintf(path, sizeof(path), "/system/etc/selinux/mapping/%s", entry->d_name);
				mmap_ro(path, &addr, &size);
				cil_add_file(db, path, addr, size);
				munmap(addr, size);
			}
		}
		closedir(dir);

		dir = opendir("/vendor/etc/selinux");
		while ((entry = readdir(dir))) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			if (strend(entry->d_name, ".cil") == 0) {
				snprintf(path, sizeof(path), "/vendor/etc/selinux/%s", entry->d_name);
				mmap_ro(path, &addr, &size);
				cil_add_file(db, path, addr, size);
				munmap(addr, size);
			}
		}
		closedir(dir);

		cil_compile(db);
		cil_build_policydb(db, &pdb);
		cil_db_destroy(&db);

		policydb = &pdb->p;
	}

	// Magisk patches
	sepol_min_rules();
	dump_policydb("/sepolicy");
	destroy_policydb();
}

int main(int argc, char *argv[]) {
	umask(0);

	struct cmdline cmd;
	parse_cmdline(&cmd);

	if (cmd.skip_initramfs) {
		// Normal boot mode
		// Clear rootfs
		int root = open("/", O_RDONLY | O_CLOEXEC);

		// Exclude overlay folder
		excl_list = (char *[]) { "overlay", NULL };
		frm_rf(root);

		mkdir("/sys", 0755);
		mount("sysfs", "/sys", "sysfs", 0, NULL);

		char partname[32];
		snprintf(partname, sizeof(partname), "system%s", cmd.slot);

		struct device dev;
		setup_block(&dev, partname);

		mkdir("/system_root", 0755);
		mount(dev.path, "/system_root", "ext4", MS_RDONLY, NULL);
		int system_root = open("/system_root", O_RDONLY | O_CLOEXEC);

		// Exclude system folder
		excl_list = (char *[]) { "system", NULL };
		clone_dir(system_root, root);
		mkdir("/system", 0755);
		mount("/system_root/system", "/system", NULL, MS_BIND, NULL);

		int overlay = open("/overlay", O_RDONLY | O_CLOEXEC);
		if (overlay > 0)
			mv_dir(overlay, root);

		snprintf(partname, sizeof(partname), "vendor%s", cmd.slot);

		// We need to mount independent vendor partition
		if (setup_block(&dev, partname) == 0)
			mount(dev.path, "/vendor", "ext4", MS_RDONLY, NULL);

		patch_ramdisk();
		patch_sepolicy();

		close(root);
		close(system_root);
		close(overlay);
		rmdir("/overlay");
		umount("/vendor");
	} else {
		// Recovery mode
		// Revert original init binary
		unlink("/init");
		rename("/.backup/init", "/init");
	}

	execv("/init", argv);

	return 0;
}
