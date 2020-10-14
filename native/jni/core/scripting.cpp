#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <magisk.hpp>
#include <utils.hpp>
#include <selinux.hpp>

using namespace std;

#define BBEXEC_CMD DATABIN "/busybox", "sh"

static void set_standalone() {
	setenv("ASH_STANDALONE", "1", 1);
};

void exec_script(const char *script) {
	exec_t exec {
		.pre_exec = set_standalone,
		.fork = fork_no_zombie
	};
	exec_command_sync(exec, BBEXEC_CMD, script);
}

void exec_common_scripts(const char *stage) {
	LOGI("* Running %s.d scripts\n", stage);
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
				.pre_exec = set_standalone,
				.fork = pfs ? fork_no_zombie : fork_dont_care
			};
			if (pfs)
				exec_command_sync(exec, BBEXEC_CMD, path);
			else
				exec_command(exec, BBEXEC_CMD, path);
		}
	}
}

void exec_module_scripts(const char *stage, const vector<string> &module_list) {
	LOGI("* Running module %s scripts\n", stage);
	char path[4096];
	bool pfs = stage == "post-fs-data"sv;
	for (auto &m : module_list) {
		const char* module = m.c_str();
		sprintf(path, MODULEROOT "/%s/%s.sh", module, stage);
		if (access(path, F_OK) == -1)
			continue;
		LOGI("%s: exec [%s.sh]\n", module, stage);
		exec_t exec {
			.pre_exec = set_standalone,
			.fork = pfs ? fork_no_zombie : fork_dont_care
		};
		if (pfs)
			exec_command_sync(exec, BBEXEC_CMD, path);
		else
			exec_command(exec, BBEXEC_CMD, path);
	}
}

constexpr char install_script[] = R"EOF(
APK=%s
log -t Magisk "apk_install: $APK"
log -t Magisk "apk_install: `pm install -r $APK 2>&1`"
rm -f $APK
)EOF";

void install_apk(const char *apk) {
	setfilecon(apk, "u:object_r:" SEPOL_FILE_TYPE ":s0");
	exec_t exec {
		.pre_exec = set_standalone,
		.fork = fork_no_zombie
	};
	char cmds[sizeof(install_script) + 4096];
	sprintf(cmds, install_script, apk);
	if (exec_command_sync(exec, BBEXEC_CMD, "-c", cmds) != 0) {
		exec_command_sync(exec, "/system/bin/sh", "-c", cmds);
	}
}

[[noreturn]] static void abort(const char *msg) {
	fprintf(stderr, "%s\n\n", msg);
	exit(1);
}

constexpr char install_module_script[] = R"EOF(
. /data/adb/magisk/util_functions.sh
install_module
exit 0
)EOF";

void install_module(const char *file) {
	if (getuid() != 0)
		abort("Run this command with root");
	if (access(DATABIN, F_OK) ||
		access(DATABIN "/busybox", X_OK) ||
		access(DATABIN "/util_functions.sh", F_OK))
		abort("Incomplete Magisk install");
	if (access(file, F_OK)) {
		char msg[4096];
		sprintf(msg, "'%s' does not exist", file);
		abort(msg);
	}

	setenv("OUTFD", "1", true);
	setenv("ZIPFILE", file, true);
	setenv("ASH_STANDALONE", "1", 1);

	int fd = xopen("/dev/null", O_RDONLY);
	xdup2(fd, STDERR_FILENO);
	close(fd);

	const char *argv[] = { BBEXEC_CMD, "-c", install_module_script };
	execve(argv[0], (char **) argv, environ);
	abort("Failed to execute BusyBox shell");
}
