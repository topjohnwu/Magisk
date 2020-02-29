#include <sys/sysmacros.h>
#include <string.h>
#include <stdio.h>
#include <vector>

#include <utils.h>
#include <logging.h>
#include <selinux.h>
#include <magisk.h>

#include "init.h"

using namespace std;

struct devinfo {
	int major;
	int minor;
	char devname[32];
	char partname[32];
};

static vector<devinfo> dev_list;

static char partname[32];
static char fstype[32];
static char block_dev[64];

static void parse_device(devinfo *dev, const char *uevent) {
	dev->partname[0] = '\0';
	parse_prop_file(uevent, [=](string_view key, string_view value) -> bool {
		if (key == "MAJOR")
			dev->major = parse_int(value.data());
		else if (key == "MINOR")
			dev->minor = parse_int(value.data());
		else if (key == "DEVNAME")
			strcpy(dev->devname, value.data());
		else if (key == "PARTNAME")
			strcpy(dev->partname, value.data());

		return true;
	});
}

static void collect_devices() {
	char path[128];
	devinfo dev{};
	if (auto dir = xopen_dir("/sys/dev/block"); dir) {
		for (dirent *entry; (entry = readdir(dir.get()));) {
			if (entry->d_name == "."sv || entry->d_name == ".."sv)
				continue;
			sprintf(path, "/sys/dev/block/%s/uevent", entry->d_name);
			parse_device(&dev, path);
			dev_list.push_back(dev);
		}
	}
}

static int64_t setup_block(bool write_block = true) {
	if (dev_list.empty())
		collect_devices();
	xmkdir("/dev", 0755);
	xmkdir("/dev/block", 0755);

	for (int tries = 0; tries < 3; ++tries) {
		for (auto &dev : dev_list) {
			if (strcasecmp(dev.partname, partname) == 0) {
				if (write_block) {
					sprintf(block_dev, "/dev/block/%s", dev.devname);
				}
				LOGD("Found %s: [%s] (%d, %d)\n", dev.partname, dev.devname, dev.major, dev.minor);
				dev_t rdev = makedev(dev.major, dev.minor);
				mknod(block_dev, S_IFBLK | 0600, rdev);
				return rdev;
			}
		}
		// Wait 10ms and try again
		usleep(10000);
		dev_list.clear();
		collect_devices();
	}

	// The requested partname does not exist
	return -1;
}

static bool is_lnk(const char *name) {
	struct stat st;
	if (lstat(name, &st))
		return false;
	return S_ISLNK(st.st_mode);
}

static bool read_dt_fstab(cmdline *cmd, const char *name) {
	char path[128];
	int fd;
	sprintf(path, "%s/fstab/%s/dev", cmd->dt_dir, name);
	if ((fd = open(path, O_RDONLY | O_CLOEXEC)) >= 0) {
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

#define mount_root(name) \
if (!is_lnk("/" #name) && read_dt_fstab(cmd, #name)) { \
	LOGD("Early mount " #name "\n"); \
	setup_block(); \
	xmkdir("/" #name, 0755); \
	xmount(block_dev, "/" #name, fstype, MS_RDONLY, nullptr); \
	mount_list.emplace_back("/" #name); \
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
			if (strncmp(me->mnt_dir, m.data(), m.length()) == 0 && me->mnt_dir[m.length()] == '/')
				return true;
		}
		mounts.emplace_back(me->mnt_dir);
		return true;
	});
	for (auto &dir : mounts) {
		auto new_path = path + dir;
		mkdir(new_path.data(), 0755);
		xmount(dir.data(), new_path.data(), nullptr, MS_MOVE, nullptr);
	}
	chdir(path.data());
	xmount(path.data(), "/", nullptr, MS_MOVE, nullptr);
	chroot(".");
}

static void mount_persist(const char *dev_base, const char *mnt_base) {
	string mnt_point = mnt_base + "/persist"s;
	strcpy(partname, "persist");
	sprintf(block_dev, "%s/persist", dev_base);
	if (setup_block(false) < 0) {
		// Fallback to cache
		strcpy(partname, "cache");
		sprintf(block_dev, "%s/cache", dev_base);
		if (setup_block(false) < 0) {
			// Try NVIDIA's BS
			strcpy(partname, "CAC");
			if (setup_block(false) < 0)
				return;
		}
		xsymlink("./cache", mnt_point.data());
		mnt_point = mnt_base + "/cache"s;
	}
	xmkdir(mnt_point.data(), 0755);
	xmount(block_dev, mnt_point.data(), "ext4", 0, nullptr);
}

void RootFSInit::early_mount() {
	full_read("/init", self.buf, self.sz);

	LOGD("Reverting /init\n");
	root = xopen("/", O_RDONLY | O_CLOEXEC);
	rename("/.backup/init", "/init");

	mount_root(system);
	mount_root(vendor);
	mount_root(product);
	mount_root(odm);

	xmkdir("/dev/mnt", 0755);
	mount_persist("/dev/block", "/dev/mnt");
	mount_list.emplace_back("/dev/mnt/persist");
	mount_list.emplace_back("/dev/mnt/cache");
}

void SARBase::backup_files(const char *self_path) {
	if (access("/overlay.d", F_OK) == 0)
		cp_afc("/overlay.d", "/dev/overlay.d");

	full_read(self_path, self.buf, self.sz);
	full_read("/.backup/.magisk", config.buf, config.sz);
}

void SARInit::early_mount() {
	// Make dev writable
	xmkdir("/dev", 0755);
	xmount("tmpfs", "/dev", "tmpfs", 0, "mode=755");
	mount_list.emplace_back("/dev");

	backup_files("/init");

	LOGD("Cleaning rootfs\n");
	int root = xopen("/", O_RDONLY | O_CLOEXEC);
	frm_rf(root, { "proc", "sys", "dev" });
	close(root);

	LOGD("Early mount system_root\n");
	sprintf(partname, "system%s", cmd->slot);
	strcpy(block_dev, "/dev/root");
	auto dev = setup_block(false);
	if (dev < 0) {
		// Try NVIDIA naming scheme
		strcpy(partname, "APP");
		dev = setup_block(false);
		if (dev < 0) {
			// We don't really know what to do at this point...
			LOGE("Cannot find root partition, abort\n");
			exit(1);
		}
	}
	system_dev = dev;
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

	backup_files("/system/bin/init");
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

void BaseInit::cleanup() {
	// Unmount in reverse order
	for (auto &p : reversed(mount_list)) {
		if (xumount(p.data()) == 0)
			LOGD("Unmount [%s]\n", p.data());
	}
	mount_list.clear();
	mount_list.shrink_to_fit();
}

void mount_sbin() {
	LOGD("Mount /sbin tmpfs overlay\n");
	xmount("tmpfs", "/sbin", "tmpfs", 0, "mode=755");

	xmkdir(MAGISKTMP, 0755);
	xmkdir(MIRRDIR, 0);
	xmkdir(BLOCKDIR, 0);

	mount_persist(BLOCKDIR, MIRRDIR);
}
