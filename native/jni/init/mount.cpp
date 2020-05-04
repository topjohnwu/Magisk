#include <sys/sysmacros.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <vector>

#include <utils.hpp>
#include <logging.hpp>
#include <selinux.hpp>
#include <magisk.hpp>

#include "init.hpp"

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

static struct {
	char partname[32];
	char block_dev[64];
} blk_info;

static int64_t setup_block(bool write_block = true) {
	if (dev_list.empty())
		collect_devices();
	xmkdir("/dev", 0755);
	xmkdir("/dev/block", 0755);

	for (int tries = 0; tries < 3; ++tries) {
		for (auto &dev : dev_list) {
			if (strcasecmp(dev.partname, blk_info.partname) == 0) {
				if (write_block) {
					sprintf(blk_info.block_dev, "/dev/block/%s", dev.devname);
				}
				LOGD("Setup %s: [%s] (%d, %d)\n", dev.partname, dev.devname, dev.major, dev.minor);
				dev_t rdev = makedev(dev.major, dev.minor);
				mknod(blk_info.block_dev, S_IFBLK | 0600, rdev);
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

static string rtrim(string &&str) {
	// Trim space, newline, and null byte from end of string
	while (memchr(" \n\r", str[str.length() - 1], 4))
		str.pop_back();
	return std::move(str);
}

#define read_info(val) \
if (access(#val, F_OK) == 0) {\
	entry.val = rtrim(full_read(#val)); \
}

void BaseInit::read_dt_fstab(vector<fstab_entry> &fstab) {
	if (access(cmd->dt_dir, F_OK) != 0)
		return;

	char cwd[128];
	getcwd(cwd, sizeof(cwd));
	chdir(cmd->dt_dir);
	run_finally cd([&]{ chdir(cwd); });

	if (access("fstab", F_OK) != 0)
		return;
	chdir("fstab");

	// Make sure dt fstab is enabled
	if (access("status", F_OK) == 0) {
		auto status = rtrim(full_read("status"));
		if (status != "okay" && status != "ok")
			return;
	}

	auto dir = xopen_dir(".");
	for (dirent *dp; (dp = xreaddir(dir.get()));) {
		if (dp->d_type != DT_DIR)
			continue;
		chdir(dp->d_name);
		run_finally f([]{ chdir(".."); });

		if (access("status", F_OK) == 0) {
			auto status = rtrim(full_read("status"));
			if (status != "okay" && status != "ok")
				continue;
		}

		fstab_entry entry;

		read_info(dev);
		read_info(mnt_point) else {
			entry.mnt_point = "/";
			entry.mnt_point += dp->d_name;
		}
		read_info(type);
		read_info(mnt_flags);
		read_info(fsmgr_flags);

		fstab.emplace_back(std::move(entry));
	}
}

void BaseInit::dt_early_mount() {
	vector<fstab_entry> fstab;
	read_dt_fstab(fstab);
	for (const auto &entry : fstab) {
		if (is_lnk(entry.mnt_point.data()))
			continue;
		// Derive partname from dev
		sprintf(blk_info.partname, "%s%s", basename(entry.dev.data()), cmd->slot);
		setup_block();
		xmkdir(entry.mnt_point.data(), 0755);
		xmount(blk_info.block_dev, entry.mnt_point.data(), entry.type.data(), MS_RDONLY, nullptr);
		mount_list.push_back(entry.mnt_point);
	}
}

static void switch_root(const string &path) {
	LOGD("Switch root to %s\n", path.data());
	int root = xopen("/", O_RDONLY);
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

	LOGD("Cleaning rootfs\n");
	frm_rf(root);
}

static void mount_persist(const char *dev_base, const char *mnt_base) {
	string mnt_point = mnt_base + "/persist"s;
	strcpy(blk_info.partname, "persist");
	xrealpath(dev_base, blk_info.block_dev);
	char *s = blk_info.block_dev + strlen(blk_info.block_dev);
	strcpy(s, "/persist");
	if (setup_block(false) < 0) {
		// Fallback to cache
		strcpy(blk_info.partname, "cache");
		strcpy(s, "/cache");
		if (setup_block(false) < 0) {
			// Try NVIDIA's BS
			strcpy(blk_info.partname, "CAC");
			if (setup_block(false) < 0)
				return;
		}
		xsymlink("./cache", mnt_point.data());
		mnt_point = mnt_base + "/cache"s;
	}
	xmkdir(mnt_point.data(), 0755);
	xmount(blk_info.block_dev, mnt_point.data(), "ext4", 0, nullptr);
}

void RootFSInit::early_mount() {
	full_read("/init", self.buf, self.sz);

	LOGD("Restoring /init\n");
	rename("/.backup/init", "/init");

	dt_early_mount();

	xmkdir("/dev/mnt", 0755);
	mount_persist("/dev/block", "/dev/mnt");
	mount_list.emplace_back("/dev/mnt/persist");
	persist_dir = "/dev/mnt/persist/magisk";
}

void SARBase::backup_files() {
	if (access("/overlay.d", F_OK) == 0)
		backup_folder("/overlay.d", overlays);

	full_read("/proc/self/exe", self.buf, self.sz);
	if (access("/.backup/.magisk", R_OK) == 0)
		full_read("/.backup/.magisk", config.buf, config.sz);
}

void SARBase::mount_system_root() {
	LOGD("Early mount system_root\n");
	sprintf(blk_info.partname, "system%s", cmd->slot);
	strcpy(blk_info.block_dev, "/dev/root");
	auto dev = setup_block(false);
	if (dev < 0) {
		// Try NVIDIA naming scheme
		strcpy(blk_info.partname, "APP");
		dev = setup_block(false);
		if (dev < 0) {
			// We don't really know what to do at this point...
			LOGE("Cannot find root partition, abort\n");
			exit(1);
		}
	}
	xmkdir("/system_root", 0755);
	if (xmount("/dev/root", "/system_root", "ext4", MS_RDONLY, nullptr))
		xmount("/dev/root", "/system_root", "erofs", MS_RDONLY, nullptr);
}

void SARInit::early_mount() {
	// Make dev writable
	xmkdir("/dev", 0755);
	xmount("tmpfs", "/dev", "tmpfs", 0, "mode=755");
	mount_list.emplace_back("/dev");

	backup_files();

	mount_system_root();
	switch_root("/system_root");

	dt_early_mount();
}

void SARFirstStageInit::early_mount() {
	backup_files();
	mount_system_root();
	switch_root("/system_root");
}

void SecondStageInit::early_mount() {
	backup_files();

	umount2("/init", MNT_DETACH);
	umount2("/proc/self/exe", MNT_DETACH);

	if (access("/system_root", F_OK) == 0)
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

static void patch_socket_name(const char *path) {
	char rstr[16];
	gen_rand_str(rstr, sizeof(rstr));
	char *buf;
	size_t size;
	mmap_rw(path, buf, size);
	raw_data_patch(buf, size, { make_pair(MAIN_SOCKET, rstr) });
	munmap(buf, size);
}

void setup_tmp(const char *path, const raw_data &self, const raw_data &config) {
	LOGD("Setup Magisk tmp at %s\n", path);
	xmount("tmpfs", path, "tmpfs", 0, "mode=755");

	chdir(path);

	xmkdir(INTLROOT, 0755);
	xmkdir(MIRRDIR, 0);
	xmkdir(BLOCKDIR, 0);

	mount_persist(BLOCKDIR, MIRRDIR);

	int fd = xopen(INTLROOT "/config", O_WRONLY | O_CREAT, 0);
	xwrite(fd, config.buf, config.sz);
	close(fd);
	fd = xopen("magiskinit", O_WRONLY | O_CREAT, 0755);
	xwrite(fd, self.buf, self.sz);
	close(fd);
	dump_magisk("magisk", 0755);
	patch_socket_name("magisk");

	// Create applet symlinks
	for (int i = 0; applet_names[i]; ++i)
		xsymlink("./magisk", applet_names[i]);
	xsymlink("./magiskinit", "magiskpolicy");
	xsymlink("./magiskinit", "supolicy");

	chdir("/");
}
