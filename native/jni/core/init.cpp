/* init.cpp - Pre-init Magisk support
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
#include <sys/sendfile.h>
#include <sys/sysmacros.h>
#include <functional>
#include <string_view>

#include <xz.h>
#include <magisk.h>
#include <magiskpolicy.h>
#include <selinux.h>
#include <cpio.h>
#include <utils.h>
#include <flags.h>

#include "binaries.h"
#ifdef USE_64BIT
#include "binaries_arch64.h"
#else
#include "binaries_arch.h"
#endif
#include "magiskrc.h"

#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"

int (*init_applet_main[]) (int, char *[]) = { magiskpolicy_main, magiskpolicy_main, nullptr };

static bool mnt_system = false;
static bool mnt_vendor = false;

static void *self, *config;
static size_t self_sz, config_sz;

struct cmdline {
	bool early_boot;
	char slot[3];
	char dt_dir[128];
};

struct device {
	dev_t major;
	dev_t minor;
	char devname[32];
	char partname[32];
	char path[64];
};

static void parse_cmdline(const std::function<void (std::string_view, const char *)> &fn) {
	char cmdline[4096];
	int fd = open("/proc/cmdline", O_RDONLY | O_CLOEXEC);
	cmdline[read(fd, cmdline, sizeof(cmdline))] = '\0';
	close(fd);

	char *tok, *eql, *tmp, *saveptr;
	saveptr = cmdline;
	while ((tok = strtok_r(nullptr, " \n", &saveptr)) != nullptr) {
		eql = strchr(tok, '=');
		if (eql) {
			*eql = '\0';
			if (eql[1] == '"') {
				tmp = strchr(saveptr, '"');
				if (tmp != nullptr) {
					*tmp = '\0';
					saveptr[-1] = ' ';
					saveptr = tmp + 1;
					eql++;
				}
			}
			fn(tok, eql + 1);
		} else {
			fn(tok, "");
		}
	}
}

static void parse_cmdline(struct cmdline *cmd) {
	char cmdline[4096];
	int fd = open("/proc/cmdline", O_RDONLY | O_CLOEXEC);
	cmdline[read(fd, cmdline, sizeof(cmdline))] = '\0';
	close(fd);

	bool skip_initramfs = false, kirin = false, enter_recovery = false;

	parse_cmdline([&](auto key, auto value) -> void {
		if (key == "androidboot.slot_suffix") {
			strcpy(cmd->slot, value);
		} else if (key == "androidboot.slot") {
			cmd->slot[0] = '_';
			strcpy(cmd->slot + 1, value);
		} else if (key == "skip_initramfs") {
			skip_initramfs = true;
		} else if (key == "androidboot.android_dt_dir") {
			strcpy(cmd->dt_dir, value);
		} else if (key == "enter_recovery") {
			enter_recovery = value[0] == '1';
		} else if (key == "androidboot.hardware") {
			kirin = strstr(value, "kirin") || strstr(value, "hi3660");
		}
	});

	if (kirin && enter_recovery) {
		// Inform that we are actually booting as recovery
		FILE *f = fopen("/.backup/.magisk", "ae");
		fprintf(f, "RECOVERYMODE=true\n");
		fclose(f);
		cmd->early_boot = true;
	}

	cmd->early_boot |= skip_initramfs;

	if (cmd->dt_dir[0] == '\0')
		strcpy(cmd->dt_dir, DEFAULT_DT_DIR);

	LOGD("cmdline: early_boot[%d] slot[%s] dt_dir[%s]\n", cmd->early_boot, cmd->slot, cmd->dt_dir);
}

static void parse_device(struct device *dev, const char *uevent) {
	dev->partname[0] = '\0';
	FILE *fp = xfopen(uevent, "r");
	char buf[64];
	while (fgets(buf, sizeof(buf), fp)) {
		if (strncmp(buf, "MAJOR", 5) == 0) {
			sscanf(buf, "MAJOR=%ld", (long*) &dev->major);
		} else if (strncmp(buf, "MINOR", 5) == 0) {
			sscanf(buf, "MINOR=%ld", (long*) &dev->minor);
		} else if (strncmp(buf, "DEVNAME", 7) == 0) {
			sscanf(buf, "DEVNAME=%s", dev->devname);
		} else if (strncmp(buf, "PARTNAME", 8) == 0) {
			sscanf(buf, "PARTNAME=%s", dev->partname);
		}
	}
	fclose(fp);
	LOGD("%s [%s] (%u, %u)\n", dev->devname, dev->partname, (unsigned) dev->major, (unsigned) dev->minor);
}

static bool setup_block(struct device *dev, const char *partname) {
	char path[128];
	struct dirent *entry;
	DIR *dir = opendir("/sys/dev/block");
	if (dir == nullptr)
		return false;
	bool found = false;
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		sprintf(path, "/sys/dev/block/%s/uevent", entry->d_name);
		parse_device(dev, path);
		if (strcasecmp(dev->partname, partname) == 0) {
			sprintf(dev->path, "/dev/block/%s", dev->devname);
			found = true;
			break;
		}
	}
	closedir(dir);

	if (!found)
		return false;

	mkdir("/dev", 0755);
	mkdir("/dev/block", 0755);
	mknod(dev->path, S_IFBLK | 0600, makedev(dev->major, dev->minor));
	return true;
}

static bool read_fstab_dt(const struct cmdline *cmd, const char *mnt_point, char *partname, char *partfs) {
	char buf[128];
	struct stat st;
	sprintf(buf, "/%s", mnt_point);
	lstat(buf, &st);
	// Don't early mount if the mount point is symlink
	if (S_ISLNK(st.st_mode))
		return false;
	int fd;
	sprintf(buf, "%s/fstab/%s/dev", cmd->dt_dir, mnt_point);
	if ((fd = xopen(buf, O_RDONLY | O_CLOEXEC)) >= 0) {
		read(fd, buf, sizeof(buf));
		close(fd);
		char *name = strrchr(buf, '/') + 1;
		sprintf(partname, "%s%s", name, strend(name, cmd->slot) ? cmd->slot : "");
		sprintf(buf, "%s/fstab/%s/type", cmd->dt_dir, mnt_point);
		if ((fd = xopen(buf, O_RDONLY | O_CLOEXEC)) >= 0) {
			lstat(buf, &st);
			read(fd, partfs, st.st_size);
			close(fd);
			return true;
		}
	}
	return false;
}

static bool verify_precompiled() {
	DIR *dir;
	struct dirent *entry;
	int fd;
	char sys_sha[64], ven_sha[64];

	// init the strings with different value
	sys_sha[0] = 0;
	ven_sha[0] = 1;

	dir = opendir(NONPLAT_POLICY_DIR);
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (strend(entry->d_name, ".sha256") == 0) {
			fd = openat(dirfd(dir), entry->d_name, O_RDONLY | O_CLOEXEC);
			read(fd, ven_sha, sizeof(ven_sha));
			close(fd);
			break;
		}
	}
	closedir(dir);
	dir = opendir(PLAT_POLICY_DIR);
	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (strend(entry->d_name, ".sha256") == 0) {
			fd = openat(dirfd(dir), entry->d_name, O_RDONLY | O_CLOEXEC);
			read(fd, sys_sha, sizeof(sys_sha));
			close(fd);
			break;
		}
	}
	closedir(dir);
	LOGD("sys_sha[%.*s]\nven_sha[%.*s]\n", sizeof(sys_sha), sys_sha, sizeof(ven_sha), ven_sha);
	return memcmp(sys_sha, ven_sha, sizeof(sys_sha)) == 0;
}

static bool patch_sepolicy() {
	bool init_patch = false;
	if (access(SPLIT_PRECOMPILE, R_OK) == 0 && verify_precompiled()) {
		init_patch = true;
		load_policydb(SPLIT_PRECOMPILE);
	} else if (access(SPLIT_PLAT_CIL, R_OK) == 0) {
		init_patch = true;
		compile_split_cil();
	} else if (access("/sepolicy", R_OK) == 0) {
		load_policydb("/sepolicy");
	} else {
		return false;
	}

	sepol_magisk_rules();
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, ALL, ALL);
	dump_policydb("/sepolicy");

	// Remove the stupid debug sepolicy and use our own
	if (access("/sepolicy_debug", F_OK) == 0) {
		unlink("/sepolicy_debug");
		link("/sepolicy", "/sepolicy_debug");
	}

	if (init_patch) {
		// Force init to load /sepolicy
		uint8_t *addr;
		size_t size;
		mmap_rw("/init", addr, size);
		for (int i = 0; i < size; ++i) {
			if (memcmp(addr + i, SPLIT_PLAT_CIL, sizeof(SPLIT_PLAT_CIL) - 1) == 0) {
				memcpy(addr + i + sizeof(SPLIT_PLAT_CIL) - 4, "xxx", 3);
				break;
			}
		}
		munmap(addr, size);
	}

	return true;
}

static bool unxz(int fd, const uint8_t *buf, size_t size) {
	uint8_t out[8192];
	xz_crc32_init();
	struct xz_dec *dec = xz_dec_init(XZ_DYNALLOC, 1 << 26);
	struct xz_buf b = {
			.in = buf,
			.in_pos = 0,
			.in_size = size,
			.out = out,
			.out_pos = 0,
			.out_size = sizeof(out)
	};
	enum xz_ret ret;
	do {
		ret = xz_dec_run(dec, &b);
		if (ret != XZ_OK && ret != XZ_STREAM_END)
			return false;
		write(fd, out, b.out_pos);
		b.out_pos = 0;
	} while (b.in_pos != size);
	return true;
}

static void decompress_ramdisk() {
	constexpr char tmp[] = "tmp.cpio";
	constexpr char ramdisk_xz[] = "ramdisk.cpio.xz";
	if (access(ramdisk_xz, F_OK))
		return;
	uint8_t *buf;
	size_t sz;
	mmap_ro(ramdisk_xz, buf, sz);
	int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC);
	unxz(fd, buf, sz);
	munmap(buf, sz);
	close(fd);
	cpio_mmap cpio(tmp);
	cpio.extract();
	unlink(tmp);
	unlink(ramdisk_xz);
}

static int dump_magisk(const char *path, mode_t mode) {
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
	if (fd < 0)
		return 1;
	if (!unxz(fd, magisk_xz, sizeof(magisk_xz)))
		return 1;
	close(fd);
	return 0;
}

static int dump_manager(const char *path, mode_t mode) {
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
	if (fd < 0)
		return 1;
	if (!unxz(fd, manager_xz, sizeof(manager_xz)))
		return 1;
	close(fd);
	return 0;
}

static void patch_socket_name(const char *path) {
	uint8_t *buf;
	char name[sizeof(MAIN_SOCKET)];
	size_t size;
	mmap_rw(path, buf, size);
	for (int i = 0; i < size; ++i) {
		if (memcmp(buf + i, MAIN_SOCKET, sizeof(MAIN_SOCKET)) == 0) {
			gen_rand_str(name, sizeof(name));
			memcpy(buf + i, name, sizeof(name));
			i += sizeof(name);
		}
		if (memcmp(buf + i, LOG_SOCKET, sizeof(LOG_SOCKET)) == 0) {
			gen_rand_str(name, sizeof(name));
			memcpy(buf + i, name, sizeof(name));
			i += sizeof(name);
		}
	}
	munmap(buf, size);
}

static void setup_init_rc() {
	FILE *rc = xfopen("/init.rc", "ae");
	char pfd_svc[8], ls_svc[8];
	gen_rand_str(pfd_svc, sizeof(pfd_svc));
	do {
		gen_rand_str(ls_svc, sizeof(ls_svc));
	} while (strcmp(pfd_svc, ls_svc) == 0);
	fprintf(rc, magiskrc, pfd_svc, pfd_svc, ls_svc);
	fclose(rc);
}

static void setup_overlay() {
	char buf[128];
	int fd;

	// Wait for early-init start
	while (access(EARLYINIT, F_OK) != 0)
		usleep(10);
	selinux_builtin_impl();
	setcon("u:r:" SEPOL_PROC_DOMAIN ":s0");
	unlink(EARLYINIT);

	fd = open("/dev/null", O_RDWR);
	xdup2(fd, STDIN_FILENO);
	xdup2(fd, STDOUT_FILENO);
	xdup2(fd, STDERR_FILENO);

	// Mount the /sbin tmpfs overlay
	xmount("tmpfs", "/sbin", "tmpfs", 0, nullptr);
	chmod("/sbin", 0755);
	setfilecon("/sbin", "u:object_r:rootfs:s0");

	// Dump binaries
	mkdir(MAGISKTMP, 0755);
	fd = open(MAGISKTMP "/config", O_WRONLY | O_CREAT, 0000);
	write(fd, config, config_sz);
	close(fd);
	fd = open("/sbin/magiskinit", O_WRONLY | O_CREAT, 0755);
	write(fd, self, self_sz);
	close(fd);
	dump_magisk("/sbin/magisk", 0755);
	patch_socket_name("/sbin/magisk");
	setfilecon("/sbin/magisk", "u:object_r:" SEPOL_FILE_DOMAIN ":s0");
	setfilecon("/sbin/magiskinit", "u:object_r:" SEPOL_FILE_DOMAIN ":s0");

	// Create applet symlinks
	for (int i = 0; applet_names[i]; ++i) {
		sprintf(buf, "/sbin/%s", applet_names[i]);
		xsymlink("/sbin/magisk", buf);
	}
	for (int i = 0; init_applet[i]; ++i) {
		sprintf(buf, "/sbin/%s", init_applet[i]);
		xsymlink("/sbin/magiskinit", buf);
	}

	// Create symlinks pointing back to /root
	DIR *dir;
	struct dirent *entry;
	dir = xopendir("/root");
	fd = xopen("/sbin", O_RDONLY);
	while((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		snprintf(buf, PATH_MAX, "/root/%s", entry->d_name);
		xsymlinkat(buf, fd, entry->d_name);
	}
	closedir(dir);
	close(fd);

	close(xopen(EARLYINITDONE, O_RDONLY | O_CREAT, 0));
	exit(0);
}

static void exec_init(char *argv[]) {
	// Clean up
	umount("/proc");
	umount("/sys");
	if (mnt_system)
		umount("/system");
	if (mnt_vendor)
		umount("/vendor");

	execv("/init", argv);
}

int main(int argc, char *argv[]) {
	umask(0);

	for (int i = 0; init_applet[i]; ++i) {
		if (strcmp(basename(argv[0]), init_applet[i]) == 0)
			return (*init_applet_main[i])(argc, argv);
	}

	if (argc > 1 && strcmp(argv[1], "-x") == 0) {
		if (strcmp(argv[2], "magisk") == 0)
			return dump_magisk(argv[3], 0755);
		else if (strcmp(argv[2], "manager") == 0)
			return dump_manager(argv[3], 0644);
	}

	// Prevent file descriptor confusion
	mknod("/null", S_IFCHR | 0666, makedev(1, 3));
	int null = open("/null", O_RDWR | O_CLOEXEC);
	unlink("/null");
	xdup3(null, STDIN_FILENO, O_CLOEXEC);
	xdup3(null, STDOUT_FILENO, O_CLOEXEC);
	xdup3(null, STDERR_FILENO, O_CLOEXEC);
	if (null > STDERR_FILENO)
		close(null);

	// Communicate with kernel using procfs and sysfs
	mkdir("/proc", 0755);
	xmount("proc", "/proc", "proc", 0, nullptr);
	mkdir("/sys", 0755);
	xmount("sysfs", "/sys", "sysfs", 0, nullptr);

	struct cmdline cmd{};
	parse_cmdline(&cmd);

	// Backup stuffs
	full_read("/init", &self, &self_sz);
	full_read("/.backup/.magisk", &config, &config_sz);

	/* ***********
	 * Initialize
	 * ***********/

	int root, sbin;
	root = open("/", O_RDONLY | O_CLOEXEC);

	if (cmd.early_boot) {
		// Clear rootfs
		const char *excl[] = { "overlay", "proc", "sys", nullptr };
		excl_list = excl;
		frm_rf(root);
		excl_list = nullptr;
	} else {
		decompress_ramdisk();

		// Revert original init binary
		rename("/.backup/init", "/init");
		rm_rf("/.backup");

		// Do not go further if device is booting into recovery
		if (access("/sbin/recovery", F_OK) == 0)
			exec_init(argv);
	}

	/* ************
	 * Early Mount
	 * ************/

	struct device dev;
	char partname[32];
	char partfs[32];

	if (cmd.early_boot) {
		sprintf(partname, "system%s", cmd.slot);
		setup_block(&dev, partname);
		xmkdir("/system_root", 0755);
		xmount(dev.path, "/system_root", "ext4", MS_RDONLY, nullptr);
		int system_root = open("/system_root", O_RDONLY | O_CLOEXEC);

		// Clone rootfs except /system
		const char *excl[] = { "system", nullptr };
		excl_list = excl;
		clone_dir(system_root, root);
		close(system_root);
		excl_list = nullptr;

		xmkdir("/system", 0755);
		xmount("/system_root/system", "/system", nullptr, MS_BIND, nullptr);
	} else if (read_fstab_dt(&cmd, "system", partname, partfs)) {
		setup_block(&dev, partname);
		xmount(dev.path, "/system", partfs, MS_RDONLY, nullptr);
		mnt_system = true;
	}

	if (read_fstab_dt(&cmd, "vendor", partname, partfs)) {
		setup_block(&dev, partname);
		xmount(dev.path, "/vendor", partfs, MS_RDONLY, nullptr);
		mnt_vendor = true;
	}

	/* ****************
	 * Ramdisk Patches
	 * ****************/

	// Handle ramdisk overlays
	int fd = open("/overlay", O_RDONLY | O_CLOEXEC);
	if (fd >= 0) {
		mv_dir(fd, root);
		close(fd);
		rmdir("/overlay");
	}
	close(root);

	// Create hardlink mirror of /sbin to /root
	mkdir("/root", 0750);
	clone_attr("/sbin", "/root");
	root = xopen("/root", O_RDONLY | O_CLOEXEC);
	sbin = xopen("/sbin", O_RDONLY | O_CLOEXEC);
	link_dir(sbin, root);

	setup_init_rc();
	patch_sepolicy();

	// Close all file descriptors
	for (int i = 0; i < 30; ++i)
		close(i);

	// Launch daemon to setup overlay
	if (fork_dont_care() == 0)
		setup_overlay();

	exec_init(argv);
}
