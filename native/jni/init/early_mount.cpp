#include <sys/mount.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <stdio.h>

#include <utils.h>
#include <logging.h>

#include "init.h"

using namespace std;

static void parse_device(device *dev, const char *uevent) {
	dev->partname[0] = '\0';
	FILE *fp = xfopen(uevent, "re");
	char buf[64];
	while (fgets(buf, sizeof(buf), fp)) {
		if (strncmp(buf, "MAJOR", 5) == 0) {
			sscanf(buf, "MAJOR=%d", &dev->major);
		} else if (strncmp(buf, "MINOR", 5) == 0) {
			sscanf(buf, "MINOR=%d", &dev->minor);
		} else if (strncmp(buf, "DEVNAME", 7) == 0) {
			sscanf(buf, "DEVNAME=%s", dev->devname);
		} else if (strncmp(buf, "PARTNAME", 8) == 0) {
			sscanf(buf, "PARTNAME=%s", dev->partname);
		}
	}
	fclose(fp);
}

static vector<device> dev_list;

static void collect_devices() {
	char path[128];
	struct dirent *entry;
	device dev;
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

static bool setup_block(const char *partname, char *block_dev) {
	if (dev_list.empty())
		collect_devices();
	for (auto &dev : dev_list) {
		if (strcasecmp(dev.partname, partname) == 0) {
			sprintf(block_dev, "/dev/block/%s", dev.devname);
			LOGD("Found %s: [%s] (%d, %d)\n", dev.partname, dev.devname, dev.major, dev.minor);
			xmkdir("/dev", 0755);
			xmkdir("/dev/block", 0755);
			mknod(block_dev, S_IFBLK | 0600, makedev(dev.major, dev.minor));
			return true;
		}
	}
	return false;
}

bool MagiskInit::read_dt_fstab(const char *name, char *partname, char *partfs) {
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
			read(fd, partfs, 32);
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
		xmount(block_dev, "/system_root", "ext4", MS_RDONLY, nullptr);
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
