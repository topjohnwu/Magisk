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
	DIR *dir;
	struct dirent *entry;
	sprintf(path, SECURE_DIR "/%s.d", stage);
	if (!(dir = xopendir(path)))
		return;
	chdir(path);

	bool pfs = strcmp(stage, "post-fs-data") == 0;
	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_REG) {
			if (access(entry->d_name, X_OK) == -1)
				continue;
			LOGI("%s.d: exec [%s]\n", stage, entry->d_name);
			exec_t exec {
				.pre_exec = set_path,
				.fork = pfs ? fork_no_zombie : fork_dont_care
			};
			if (pfs)
				exec_command_sync(exec, "/system/bin/sh", entry->d_name);
			else
				exec_command(exec, "/system/bin/sh", entry->d_name);
		}
	}

	closedir(dir);
	chdir("/");
}

void exec_module_script(const char *stage, const vector<string> &module_list) {
	char path[4096];
	bool pfs = strcmp(stage, "post-fs-data") == 0;
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

static const char migrate_script[] =
"IMG=%s;"
"MNT=/dev/img_mnt;"
"e2fsck -yf $IMG;"
"mkdir -p $MNT;"
"for num in 0 1 2 3 4 5 6 7; do"
"  losetup /dev/block/loop${num} $IMG || continue;"
"  mount -t ext4 /dev/block/loop${num} $MNT;"
"  rm -rf $MNT/lost+found $MNT/.core;"
"  magisk --clone $MNT " MODULEROOT ";"
"  umount $MNT;"
"  rm -rf $MNT;"
"  losetup -d /dev/block/loop${num};"
"  break;"
"done;"
"rm -rf $IMG;";

void migrate_img(const char *img) {
	LOGI("* Migrating %s\n", img);
	exec_t exec { .pre_exec = set_path };
	char cmds[sizeof(migrate_script) + 128];
	sprintf(cmds, migrate_script, img);
	exec_command_sync(exec, "/system/bin/sh", "-c", cmds);
}

static const char install_script[] =
"APK=%s;"
"log -t Magisk \"apk_install: $APK\";"
"log -t Magisk \"apk_install: `pm install -r $APK 2>&1`\";"
"rm -f $APK;";

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
