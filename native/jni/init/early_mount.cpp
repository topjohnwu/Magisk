#include <sys/mount.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <stdio.h>

#include <utils.h>
#include <logging.h>

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

static void setup_block(const char *partname, char *block_dev) {
	if (dev_list.empty())
		collect_devices();
	for (;;) {
		for (auto &dev : dev_list) {
			if (strcasecmp(dev.partname, partname) == 0) {
				sprintf(block_dev, "/dev/block/%s", dev.devname);
				LOGD("Found %s: [%s] (%d, %d)\n", dev.partname, dev.devname, dev.major, dev.minor);
				xmkdir("/dev", 0755);
				xmkdir("/dev/block", 0755);
				mknod(block_dev, S_IFBLK | 0600, makedev(dev.major, dev.minor));
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
	sprintf(path, "%s/fstab/%s/dev", cmd.dt_dir, name);
	if ((fd = xopen(path, O_RDONLY | O_CLOEXEC)) >= 0) {
		read(fd, path, sizeof(path));
		close(fd);
		// Some custom treble use different names, so use what we read
		char *part = rtrim(strrchr(path, '/') + 1);
		sprintf(partname, "%s%s", part, strend(part, cmd.slot) ? cmd.slot : "");
		sprintf(path, "%s/fstab/%s/type", cmd.dt_dir, name);
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

void MagiskInit::early_mount() {
	char partname[32];
	char fstype[32];
	char block_dev[64];

	if (cmd.system_as_root) {
		LOGD("Early mount system_root\n");
		sprintf(partname, "system%s", cmd.slot);
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

		// Copy if these partitions are symlinks
		link_root("/vendor");
		link_root("/product");
		link_root("/odm");
	} else {
		mount_root(system);
	}

	mount_root(vendor);
	mount_root(product);
	mount_root(odm);
}
