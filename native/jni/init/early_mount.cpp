#include <sys/sysmacros.h>
#include <string.h>
#include <stdio.h>
#include <vector>

#include <utils.h>
#include <logging.h>
#include <selinux.h>

#include "init.h"

using namespace std;

struct devinfo {
	int major;
	int minor;
	char devname[32];
	char partname[32];
};

static vector<devinfo> dev_list;

static void parse_device(devinfo *dev, const char *uevent) {
	dev->partname[0] = '\0';
	parse_prop_file(uevent, [=](string_view key, string_view value) -> bool {
		if (key == "MAJOR")
			dev->major = atoi(value.data());
		else if (key == "MINOR")
			dev->minor = atoi(value.data());
		else if (key == "DEVNAME")
			strcpy(dev->devname, value.data());
		else if (key == "PARTNAME")
			strcpy(dev->partname, value.data());

		return true;
	});
}

static void collect_devices() {
	char path[128];
	struct dirent *entry;
	devinfo dev;
	DIR *dir = xopendir("/sys/dev/block");
	if (dir == nullptr)
		return;
	while ((entry = readdir(dir))) {
		if (entry->d_name == "."sv || entry->d_name == ".."sv)
			continue;
		sprintf(path, "/sys/dev/block/%s/uevent", entry->d_name);
		parse_device(&dev, path);
		dev_list.push_back(dev);
	}
	closedir(dir);
}

static dev_t setup_block(const char *partname, char *block_dev = nullptr) {
	if (dev_list.empty())
		collect_devices();
	for (;;) {
		for (auto &dev : dev_list) {
			if (strcasecmp(dev.partname, partname) == 0) {
				xmkdir("/dev", 0755);
				if (block_dev) {
					sprintf(block_dev, "/dev/block/%s", dev.devname);
					xmkdir("/dev/block", 0755);
				}
				LOGD("Found %s: [%s] (%d, %d)\n", dev.partname, dev.devname, dev.major, dev.minor);
				dev_t rdev = makedev(dev.major, dev.minor);
				mknod(block_dev ? block_dev : "/dev/root", S_IFBLK | 0600, rdev);
				return rdev;
			}
		}
		// Wait 10ms and try again
		usleep(10000);
		dev_list.clear();
		collect_devices();
	}
}

static bool is_lnk(const char *name) {
	struct stat st;
	if (lstat(name, &st))
		return false;
	return S_ISLNK(st.st_mode);
}

bool MagiskInit::read_dt_fstab(const char *name, char *partname, char *fstype) {
	char path[128];
	int fd;
	sprintf(path, "%s/fstab/%s/dev", cmd->dt_dir, name);
	if ((fd = xopen(path, O_RDONLY | O_CLOEXEC)) >= 0) {
		read(fd, path, sizeof(path));
		close(fd);
		// Some custom treble use different names, so use what we read
		char *part = rtrim(strrchr(path, '/') + 1);
		sprintf(partname, "%s%s", part, strend(part, cmd->slot) ? cmd->slot : "");
		sprintf(path, "%s/fstab/%s/type", cmd->dt_dir, name);
		if ((fd = xopen(path, O_RDONLY | O_CLOEXEC)) >= 0) {
			read(fd, fstype, 32);
			close(fd);
			return true;
		}
	}
	return false;
}

static char partname[32];
static char fstype[32];
static char block_dev[64];

#define link_root(name) \
if (is_lnk("/system_root" name)) \
	cp_afc("/system_root" name, name)

#define mount_root(name) \
if (!is_lnk("/" #name) && read_dt_fstab(#name, partname, fstype)) { \
	LOGD("Early mount " #name "\n"); \
	setup_block(partname, block_dev); \
	xmkdir("/" #name, 0755); \
	xmount(block_dev, "/" #name, fstype, MS_RDONLY, nullptr); \
	mnt_##name = true; \
}

void LegacyInit::early_mount() {
	full_read("/init", self.buf, self.sz);

	LOGD("Reverting /init\n");
	root = xopen("/", O_RDONLY | O_CLOEXEC);
	rename("/.backup/init", "/init");

	mount_root(system);
	mount_root(vendor);
	mount_root(product);
	mount_root(odm);
}

void SARCompatInit::early_mount() {
	full_read("/init", self.buf, self.sz);

	LOGD("Cleaning rootfs\n");
	root = xopen("/", O_RDONLY | O_CLOEXEC);
	frm_rf(root, { ".backup", "overlay", "overlay.d", "proc", "sys" });

	LOGD("Early mount system_root\n");
	sprintf(partname, "system%s", cmd->slot);
	setup_block(partname, block_dev);
	xmkdir("/system_root", 0755);
	if (xmount(block_dev, "/system_root", "ext4", MS_RDONLY, nullptr))
		xmount(block_dev, "/system_root", "erofs", MS_RDONLY, nullptr);
	xmkdir("/system", 0755);
	xmount("/system_root/system", "/system", nullptr, MS_BIND, nullptr);

	link_root("/vendor");
	link_root("/product");
	link_root("/odm");
	mount_root(vendor);
	mount_root(product);
	mount_root(odm);
}

static void switch_root(const string &path) {
	LOGD("Switch root to %s\n", path.data());
	vector<string> mounts;
	parse_mnt("/proc/mounts", [&](mntent *me) {
		// Skip root and self
		if (me->mnt_dir == "/"sv || me->mnt_dir == path)
			return true;
		// Do not include subtrees
		for (const auto &m : mounts) {
			if (strncmp(me->mnt_dir, m.data(), m.length()) == 0)
				return true;
		}
		mounts.emplace_back(me->mnt_dir);
		return true;
	});
	for (auto &dir : mounts) {
		auto new_path = path + dir;
		mkdir(new_path.data(), 0755);
		xmount(dir.data(), new_path.c_str(), nullptr, MS_MOVE, nullptr);
	}
	chdir(path.data());
	xmount(path.data(), "/", nullptr, MS_MOVE, nullptr);
	chroot(".");
}

void SARCommon::backup_files() {
	if (access("/overlay.d", F_OK) == 0)
		cp_afc("/overlay.d", "/dev/overlay.d");

	full_read("/init", self.buf, self.sz);
	full_read("/.backup/.magisk", config.buf, config.sz);
}

void SARInit::early_mount() {
	// Make dev writable
	xmkdir("/dev", 0755);
	xmount("tmpfs", "/dev", "tmpfs", 0, "mode=755");

	backup_files();

	LOGD("Cleaning rootfs\n");
	int root = xopen("/", O_RDONLY | O_CLOEXEC);
	frm_rf(root, { "proc", "sys", "dev" });
	close(root);

	LOGD("Early mount system_root\n");
	sprintf(partname, "system%s", cmd->slot);
	system_dev = setup_block(partname);
	xmkdir("/system_root", 0755);
	if (xmount("/dev/root", "/system_root", "ext4", MS_RDONLY, nullptr))
		xmount("/dev/root", "/system_root", "erofs", MS_RDONLY, nullptr);
	switch_root("/system_root");

	mount_root(vendor);
	mount_root(product);
	mount_root(odm);
}

void SecondStageInit::early_mount() {
	// Early mounts should already be done by first stage init

	backup_files();
	rm_rf("/system");
	rm_rf("/.backup");
	rm_rf("/overlay.d");

	// Find system_dev
	parse_mnt("/proc/mounts", [&](mntent *me) -> bool {
		if (me->mnt_dir == "/system_root"sv) {
			struct stat st;
			stat(me->mnt_fsname, &st);
			system_dev = st.st_rdev;
			return false;
		}
		return true;
	});

	switch_root("/system_root");
}

#define umount_root(name) \
if (mnt_##name) \
	umount("/" #name);

void MagiskInit::cleanup() {
	umount(SELINUX_MNT);
	BaseInit::cleanup();
	umount_root(system);
	umount_root(vendor);
	umount_root(product);
	umount_root(odm);
}
