#include <sys/mount.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <selinux.hpp>
#include <resetprop.hpp>

#include "magiskhide.hpp"

using namespace std;

static const char *prop_key[] =
		{ "ro.boot.vbmeta.device_state", "ro.boot.verifiedbootstate", "ro.boot.flash.locked",
		  "ro.boot.veritymode", "ro.boot.warranty_bit", "ro.warranty_bit", "ro.debuggable",
		  "ro.secure", "ro.build.type", "ro.build.tags", "ro.build.selinux",
		  "ro.vendor.boot.warranty_bit", "ro.vendor.warranty_bit", nullptr };

static const char *prop_value[] =
		{ "locked", "green", "1",
		  "enforcing", "0", "0", "0",
		  "1", "user", "release-keys", "0",
		  "0", "0", nullptr };

void hide_sensitive_props() {
	LOGI("hide_policy: Hiding sensitive props\n");

	// Hide all sensitive props
	for (int i = 0; prop_key[i]; ++i) {
		auto value = getprop(prop_key[i]);
		if (!value.empty() && value != prop_value[i])
			setprop(prop_key[i], prop_value[i], false);
	}

	// Hide that we booted from recovery when magisk is in recovery mode
	auto bootmode = getprop("ro.bootmode");
	if (!bootmode.empty() && bootmode.find("recovery") != string::npos) {
		setprop("ro.bootmode", "unknown", false);
	}
	bootmode = getprop("ro.boot.mode");
	if (!bootmode.empty() && bootmode.find("recovery") != string::npos) {
		setprop("ro.boot.mode", "unknown", false);
	}
	// Xiaomi cross region flash
	auto hwc = getprop("ro.boot.hwc");
	if (!hwc.empty() && hwc.find("CN") != string::npos) {
		setprop("ro.boot.hwc", "GLOBAL", false);
	}
	auto hwcountry = getprop("ro.boot.hwcountry");
	if (!hwcountry.empty() && hwcountry.find("China") != string::npos) {
		setprop("ro.boot.hwcountry", "GLOBAL", false);
	}
}

static inline void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		LOGD("hide_policy: Unmounted (%s)\n", mountpoint);
}

void hide_daemon(int pid) {
	run_finally fin([=]() -> void {
		// Send resume signal
		kill(pid, SIGCONT);
		_exit(0);
	});
	hide_unmount(pid);
}

#define TMPFS_MNT(dir) (mentry->mnt_type == "tmpfs"sv && \
strncmp(mentry->mnt_dir, "/" #dir, sizeof("/" #dir) - 1) == 0)

void hide_unmount(int pid) {
	if (switch_mnt_ns(pid))
		return;

	LOGD("hide_policy: handling PID=[%d]\n", pid);

	char val;
	int fd = xopen(SELINUX_ENFORCE, O_RDONLY);
	xxread(fd, &val, sizeof(val));
	close(fd);
	// Permissive
	if (val == '0') {
		chmod(SELINUX_ENFORCE, 0640);
		chmod(SELINUX_POLICY, 0440);
	}

	vector<string> targets;

	// Unmount dummy skeletons and /sbin links
	targets.push_back(MAGISKTMP);
	parse_mnt("/proc/self/mounts", [&](mntent *mentry) {
		if (TMPFS_MNT(system) || TMPFS_MNT(vendor) || TMPFS_MNT(product) || TMPFS_MNT(system_ext))
			targets.emplace_back(mentry->mnt_dir);
		return true;
	});

	for (auto &s : reversed(targets))
		lazy_unmount(s.data());
	targets.clear();

	// Unmount all Magisk created mounts
	parse_mnt("/proc/self/mounts", [&](mntent *mentry) {
		if (strstr(mentry->mnt_fsname, BLOCKDIR))
			targets.emplace_back(mentry->mnt_dir);
		return true;
	});

	for (auto &s : reversed(targets))
		lazy_unmount(s.data());
}

