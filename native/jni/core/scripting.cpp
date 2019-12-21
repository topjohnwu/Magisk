#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <magisk.h>
#include <utils.h>
#include <selinux.h>

using namespace std;

static void set_path() {
	char buf[4096];
	sprintf(buf, BBPATH ":%s", getenv("PATH"));
	setenv("PATH", buf, 1);
}

void exec_script(const char *script) {
	exec_t exec {
		.pre_exec = set_path,
		.fork = fork_no_zombie
	};
	exec_command_sync(exec, "/system/bin/sh", script);
}

void exec_common_script(const char *stage) {
	char path[4096];
	char *name = path + sprintf(path, SECURE_DIR "/%s.d", stage);
	auto dir = xopen_dir(path);
	if (!dir)
		return;

	int dfd = dirfd(dir.get());
	bool pfs = stage == "post-fs-data"sv;
	*(name++) = '/';

	for (dirent *entry; (entry = xreaddir(dir.get()));) {
		if (entry->d_type == DT_REG) {
			if (faccessat(dfd, entry->d_name, X_OK, 0) != 0)
				continue;
			LOGI("%s.d: exec [%s]\n", stage, entry->d_name);
			strcpy(name, entry->d_name);
			exec_t exec {
				.pre_exec = set_path,
				.fork = pfs ? fork_no_zombie : fork_dont_care
			};
			if (pfs)
				exec_command_sync(exec, "/system/bin/sh", path);
			else
				exec_command(exec, "/system/bin/sh", path);
		}
	}
}

void exec_module_script(const char *stage, const vector<string> &module_list) {
	char path[4096];
	bool pfs = stage == "post-fs-data"sv;
	for (auto &m : module_list) {
		const char* module = m.c_str();
		sprintf(path, MODULEROOT "/%s/%s.sh", module, stage);
		if (access(path, F_OK) == -1)
			continue;
		LOGI("%s: exec [%s.sh]\n", module, stage);
		exec_t exec {
			.pre_exec = set_path,
			.fork = pfs ? fork_no_zombie : fork_dont_care
		};
		if (pfs)
			exec_command_sync(exec, "/system/bin/sh", path);
		else
			exec_command(exec, "/system/bin/sh", path);
	}
}

constexpr char install_script[] = R"EOF(
APK=%s
log -t Magisk "apk_install: $APK"
log -t Magisk "apk_install: `pm install -r $APK 2>&1`"
rm -f $APK
)EOF";

void install_apk(const char *apk) {
	setfilecon(apk, "u:object_r:" SEPOL_FILE_DOMAIN ":s0");
	exec_t exec {
		.pre_exec = set_path,
		.fork = fork_no_zombie
	};
	char cmds[sizeof(install_script) + 4096];
	sprintf(cmds, install_script, apk);
	exec_command_sync(exec, "/system/bin/sh", "-c", cmds);
}
