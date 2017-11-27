/* magiskinit.c - Pre-init Magisk support
 *
 * This code has to be compiled statically to work properly.
 *
 * To unify Magisk support for both legacy "normal" devices and new skip_initramfs devices,
 * magisk binary compilation is split into two parts - first part only compiles "magisk".
 * The python build script will load the magisk main binary and compress with lzma2, dumping
 * the results into "dump.h". The "magisk" binary is embedded into this binary, and will
 * get extracted to the overlay folder along with init.magisk.rc.
 *
 * This tool does all pre-init operations to setup a Magisk environment, which pathces rootfs
 * on the fly, providing fundamental support such as init, init.rc, and sepolicy patching.
 *
 * Magiskinit is also responsible for constructing a proper rootfs on skip_initramfs devices.
 * On skip_initramfs devices, it will parse kernel cmdline, mount sysfs, parse through
 * uevent files to make the system (or vendor if available) block device node, then copy
 * rootfs files from system.
 *
 * This tool will be replaced with the real init to continue the boot process, but hardlinks are
 * preserved as it also provides CLI for sepolicy patching (magiskpolicy)
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/sysmacros.h>

#include <lzma.h>
#include <cil/cil.h>

#include "magisk.h"
#include "utils.h"
#include "magiskpolicy.h"
#include "magiskrc.h"
#include "dump.h"

// #define VLOG(fmt, ...) printf(fmt, __VA_ARGS__)   /* Enable to debug */
#define VLOG(fmt, ...)

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

static void parse_cmdline(struct cmdline *cmd) {
	// cleanup
	cmd->skip_initramfs = 0;
	cmd->slot[0] = '\0';

	char *tok;
	char cmdline[4096];
	mkdir("/proc", 0555);
	mount("proc", "/proc", "proc", 0, NULL);
	int fd = open("/proc/cmdline", O_RDONLY | O_CLOEXEC);
	cmdline[read(fd, cmdline, sizeof(cmdline))] = '\0';
	close(fd);
	umount("/proc");
	tok = strtok(cmdline, " ");
	while (tok != NULL) {
		if (strncmp(tok, "androidboot.slot_suffix", 23) == 0) {
			sscanf(tok, "androidboot.slot_suffix=%s", cmd->slot);
		} else if (strncmp(tok, "androidboot.slot", 16) == 0) {
			cmd->slot[0] = '_';
			sscanf(tok, "androidboot.slot=%s", cmd->slot + 1);
		} else if (strcmp(tok, "skip_initramfs") == 0) {
			cmd->skip_initramfs = 1;
		}
		tok = strtok(NULL, " ");
	}
}

static void parse_device(struct device *dev, char *uevent) {
	dev->partname[0] = '\0';
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
	VLOG("%s [%s] (%u, %u)\n", dev->devname, dev->partname, (unsigned) dev->major, (unsigned) dev->minor);
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

static void *patch_init_rc(char *data, uint32_t *size) {
	int injected = 0;
	char *new_data = malloc(*size + 23);
	char *old_data = data;
	uint32_t pos = 0;

	for (char *tok = strsep(&old_data, "\n"); tok; tok = strsep(&old_data, "\n")) {
		if (!injected && strncmp(tok, "import", 6) == 0) {
			if (strstr(tok, "init.magisk.rc")) {
				injected = 1;
			} else {
				strcpy(new_data + pos, "import /init.magisk.rc\n");
				pos += 23;
				injected = 1;
			}
		} else if (strstr(tok, "selinux.reload_policy")) {
			continue;
		}
		// Copy the line
		strcpy(new_data + pos, tok);
		pos += strlen(tok);
		new_data[pos++] = '\n';
	}

	*size = pos;
	return new_data;
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

static int compile_cil() {
	DIR *dir;
	struct dirent *entry;
	char path[128];

	struct cil_db *db = NULL;
	sepol_policydb_t *pdb = NULL;
	void *addr;
	size_t size;

	cil_db_init(&db);
	cil_set_mls(db, 1);
	cil_set_target_platform(db, SEPOL_TARGET_SELINUX);
	cil_set_policy_version(db, POLICYDB_VERSION_XPERMS_IOCTL);
	cil_set_attrs_expand_generated(db, 0);

	// plat
	mmap_ro("/system/etc/selinux/plat_sepolicy.cil", &addr, &size);
	cil_add_file(db, "/system/etc/selinux/plat_sepolicy.cil", addr, size);
	munmap(addr, size);

	// mapping
	char plat[10];
	int fd = open("/vendor/etc/selinux/plat_sepolicy_vers.txt", O_RDONLY | O_CLOEXEC);
	if (fd > 0) {
		plat[read(fd, plat, sizeof(plat)) - 1] = '\0';
		snprintf(path, sizeof(path), "/system/etc/selinux/mapping/%s.cil", plat);
		mmap_ro(path, &addr, &size);
		cil_add_file(db, path, addr, size);
		munmap(addr, size);
		close(fd);
	}

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

	return 0;
}

static int verify_precompiled() {
	DIR *dir;
	struct dirent *entry;
	int fd;
	char sys_sha[70], ven_sha[70];

	dir = opendir("/vendor/etc/selinux");
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (strend(entry->d_name, ".sha256") == 0) {
			fd = openat(dirfd(dir), entry->d_name, O_RDONLY | O_CLOEXEC);
			ven_sha[read(fd, ven_sha, sizeof(ven_sha))] = '\0';
			close(fd);
			break;
		}
	}
	closedir(dir);
	dir = opendir("/system/etc/selinux");
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (strend(entry->d_name, ".sha256") == 0) {
			fd = openat(dirfd(dir), entry->d_name, O_RDONLY | O_CLOEXEC);
			sys_sha[read(fd, sys_sha, sizeof(sys_sha))] = '\0';
			close(fd);
			break;
		}
	}
	closedir(dir);
	return strcmp(sys_sha, ven_sha);
}

static void patch_sepolicy() {
	if (access("/sepolicy", R_OK) == 0) {
		load_policydb("/sepolicy");
	} else if (access("/vendor/etc/selinux/precompiled_sepolicy", R_OK) == 0
		&& verify_precompiled() == 0) {
		load_policydb("/vendor/etc/selinux/precompiled_sepolicy");
	} else if (access("/system/etc/selinux/plat_sepolicy.cil", R_OK) == 0) {
		compile_cil();
	}

	sepol_med_rules();
	dump_policydb("/sepolicy");
}

#define BUFSIZE (1 << 20)

static int unxz(const void *buf, size_t size, int fd) {
	lzma_stream strm = LZMA_STREAM_INIT;
	if (lzma_auto_decoder(&strm, UINT64_MAX, 0) != LZMA_OK)
		return 1;
	lzma_ret ret = 0;
	void *out = malloc(BUFSIZE);
	strm.next_in = buf;
	strm.avail_in = size;
	do {
		strm.next_out = out;
		strm.avail_out = BUFSIZE;
		ret = lzma_code(&strm, LZMA_RUN);
		write(fd, out, BUFSIZE - strm.avail_out);
	} while (strm.avail_out == 0 && ret == LZMA_OK);

	free(out);
	lzma_end(&strm);

	if (ret != LZMA_OK && ret != LZMA_STREAM_END)
		return 1;
	return 0;
}

static int dump_magisk(const char *path, mode_t mode) {
	unlink(path);
	int fd = creat(path, mode);
	int ret = unxz(magisk_dump, sizeof(magisk_dump), fd);
	close(fd);
	return ret;
}

static int dump_magiskrc(const char *path, mode_t mode) {
	int fd = creat(path, mode);
	write(fd, magiskrc, sizeof(magiskrc));
	close(fd);
	return 0;
}

static void magisk_init_daemon() {
	// Fork a new process for full patch
	setsid();
	sepol_allow("su", ALL, ALL, ALL);
	wait_till_exists("/dev/.coldboot_done");
	dump_policydb(SELINUX_LOAD);
	close(open(PATCHDONE, O_RDONLY | O_CREAT, 0));
	destroy_policydb();
	exit(0);
}

int main(int argc, char *argv[]) {
	umask(0);

	if (strcmp(basename(argv[0]), "magiskpolicy") == 0 || strcmp(basename(argv[0]), "supolicy") == 0)
		return magiskpolicy_main(argc, argv);
	if (argc > 1 && (strcmp(argv[1], "magiskpolicy") == 0 || strcmp(argv[1], "supolicy") == 0))
		return magiskpolicy_main(argc - 1, argv + 1);

	if (argc > 1 && strcmp(argv[1], "-x") == 0) {
		if (strcmp(argv[2], "magisk") == 0)
			return dump_magisk(argv[3], 0755);
		else if (strcmp(argv[2], "magiskrc") == 0)
			return dump_magiskrc(argv[3], 0755);
	}

	// Extract and link files
	mkdir("/overlay", 0000);
	dump_magiskrc("/overlay/init.magisk.rc", 0750);
	mkdir("/overlay/sbin", 0755);
	dump_magisk("/overlay/sbin/magisk", 0755);
	mkdir("/overlay/root", 0755);
	link("/init", "/overlay/root/magiskinit");
	symlink("/root/magiskinit", "/overlay/root/magiskpolicy");
	symlink("/root/magiskinit", "/overlay/root/supolicy");

	struct cmdline cmd;
	parse_cmdline(&cmd);

	VLOG("cmdline: skip_initramfs=[%d] slot_suffix=[%s]\n", cmd.skip_initramfs, cmd.slot);

	int root = open("/", O_RDONLY | O_CLOEXEC);

	if (cmd.skip_initramfs) {
		// Exclude overlay folder
		excl_list = (char *[]) { "overlay", ".backup", NULL };
		// Clear rootfs
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

		snprintf(partname, sizeof(partname), "vendor%s", cmd.slot);

		// We need to mount independent vendor partition
		if (setup_block(&dev, partname) == 0)
			mount(dev.path, "/vendor", "ext4", MS_RDONLY, NULL);

		close(system_root);
	} else {
		// Revert original init binary
		unlink("/init");
		link("/.backup/init", "/init");
	}

	int overlay = open("/overlay", O_RDONLY | O_CLOEXEC);
	mv_dir(overlay, root);

	// Clean up
	rmdir("/overlay");
	close(overlay);
	close(root);

	patch_ramdisk();
	patch_sepolicy();

	umount("/vendor");

	if (fork() == 0)
		magisk_init_daemon();

	// Finally, give control back!
	execv("/init", argv);
}
