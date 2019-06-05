#include <sys/mount.h>

#include <magisk.h>
#include <utils.h>
#include <selinux.h>
#include <resetprop.h>

#include "magiskhide.h"

using namespace std;

static const char *prop_key[] =
		{ "ro.boot.vbmeta.device_state", "ro.boot.verifiedbootstate", "ro.boot.flash.locked",
		  "ro.boot.veritymode", "ro.boot.warranty_bit", "ro.warranty_bit", "ro.debuggable",
		  "ro.secure", "ro.build.type", "ro.build.tags", "ro.build.selinux", nullptr };

static const char *prop_value[] =
		{ "locked", "green", "1",
		  "enforcing", "0", "0", "0",
		  "1", "user", "release-keys", "0", nullptr };

void hide_sensitive_props() {
	LOGI("hide_policy: Hiding sensitive props\n");

	// Hide all sensitive props
	for (int i = 0; prop_key[i]; ++i) {
		auto value = getprop(prop_key[i]);
		if (!value.empty() && value != prop_value[i])
			setprop(prop_key[i], prop_value[i], false);
	}
}

static inline void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		LOGD("hide_policy: Unmounted (%s)\n", mountpoint);
}

void hide_daemon(int pid) {
	RunFinally fin([=]() -> void {
		// Send resume signal
		tgkill(pid, pid, SIGCONT);
		_exit(0);
	});
	hide_unmount(pid);
}

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

	getprop([](const char *name, auto, auto) -> void {
		if (strstr(name, "magisk"))
			deleteprop(name);
	}, nullptr, false);

	vector<string> targets;

	// Unmount dummy skeletons and /sbin links
	file_readline("/proc/self/mounts", [&](string_view s) -> bool {
		if (str_contains(s, "tmpfs /system/") || str_contains(s, "tmpfs /vendor/") ||
			str_contains(s, "tmpfs /sbin")) {
			char *path = (char *) s.data();
			// Skip first token
			strtok_r(nullptr, " ", &path);
			targets.emplace_back(strtok_r(nullptr, " ", &path));
		}
		return true;
	});

	for (auto &s : targets)
		lazy_unmount(s.data());
	targets.clear();

	// Unmount all Magisk created mounts
	file_readline("/proc/self/mounts", [&](string_view s) -> bool {
		if (str_contains(s, BLOCKDIR)) {
			char *path = (char *) s.data();
			// Skip first token
			strtok_r(nullptr, " ", &path);
			targets.emplace_back(strtok_r(nullptr, " ", &path));
		}
		return true;
	});

	for (auto &s : targets)
		lazy_unmount(s.data());
}

