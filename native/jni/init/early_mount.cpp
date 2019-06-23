#include <sys/mount.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <stdio.h>

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

static void setup_block(const char *partname, char *block_dev = nullptr) {
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
				mknod(block_dev ? block_dev : "/dev/root", S_IFBLK | 0600,
						makedev(dev.major, dev.minor));
				return;
			}
		}
		// Wait 10ms and try again
		usleep(10000);
		dev_list.clear();
		collect_devices();
	}
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
	char partname[32];
	char fstype[32];
	char block_dev[64];

	mount_root(system);
	mount_root(vendor);
	mount_root(product);
	mount_root(odm);
}

void SARCompatInit::early_mount() {
	char partname[32];
	char fstype[32];
	char block_dev[64];

	LOGD("Early mount system_root\n");
	sprintf(partname, "system%s", cmd->slot);
	setup_block(partname, block_dev);
	xmkdir("/system_root", 0755);
	if (xmount(block_dev, "/system_root", "ext4", MS_RDONLY, nullptr))
		xmount(block_dev, "/system_root", "erofs", MS_RDONLY, nullptr);
	xmkdir("/system", 0755);
	xmount("/system_root/system", "/system", nullptr, MS_BIND, nullptr);

	// Android Q
	if (is_lnk("/system_root/init"))
		load_sepol = true;

	// System-as-root with monolithic sepolicy
	if (access("/system_root/sepolicy", F_OK) == 0)
		cp_afc("/system_root/sepolicy", "/sepolicy");

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
		if (me->mnt_dir != "/"sv && me->mnt_dir != path)
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

void SARInit::early_mount() {
	char partname[32];

	LOGD("Early mount system_root\n");
	sprintf(partname, "system%s", cmd->slot);
	setup_block(partname);
	xmkdir("/system_root", 0755);
	if (xmount("/dev/root", "/system_root", "ext4", MS_RDONLY, nullptr))
		xmount("/dev/root", "/system_root", "erofs", MS_RDONLY, nullptr);
	switch_root("/system_root");
}

#define umount_root(name) \
if (mnt_##name) \
	umount("/" #name);

void MagiskInit::cleanup() {
	BaseInit::cleanup();
	umount(SELINUX_MNT);
	umount_root(system);
	umount_root(vendor);
	umount_root(product);
	umount_root(odm);
}
