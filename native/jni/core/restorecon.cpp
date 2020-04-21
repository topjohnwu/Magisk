#include <string_view>

#include <magisk.hpp>
#include <selinux.hpp>
#include <utils.hpp>

using namespace std;

#define UNLABEL_CON "u:object_r:unlabeled:s0"
#define SYSTEM_CON  "u:object_r:system_file:s0"
#define ADB_CON     "u:object_r:adb_data_file:s0"
#define ROOT_CON    "u:object_r:rootfs:s0"
#define MAGISK_CON  "u:object_r:" SEPOL_FILE_DOMAIN ":s0"

static void restore_syscon(int dirfd) {
	struct dirent *entry;
	DIR *dir;
	char *con;

	fgetfilecon(dirfd, &con);
	if (strlen(con) == 0 || strcmp(con, UNLABEL_CON) == 0)
		fsetfilecon(dirfd, SYSTEM_CON);
	freecon(con);

	dir = xfdopendir(dirfd);
	while ((entry = xreaddir(dir))) {
		int fd = openat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
		if (entry->d_type == DT_DIR) {
			restore_syscon(fd);
		} else if (entry->d_type == DT_REG) {
			fgetfilecon(fd, &con);
			if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
				fsetfilecon(fd, SYSTEM_CON);
			freecon(con);
		} else if (entry->d_type == DT_LNK) {
			getfilecon_at(dirfd, entry->d_name, &con);
			if (con[0] == '\0' || strcmp(con, UNLABEL_CON) == 0)
				setfilecon_at(dirfd, entry->d_name, con);
			freecon(con);
		}
		close(fd);
	}
}

static void restore_magiskcon(int dirfd) {
	struct dirent *entry;
	DIR *dir;

	fsetfilecon(dirfd, MAGISK_CON);
	fchown(dirfd, 0, 0);

	dir = xfdopendir(dirfd);
	while ((entry = xreaddir(dir))) {
		int fd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
		if (entry->d_type == DT_DIR) {
			restore_magiskcon(fd);
		} else if (entry->d_type) {
			fsetfilecon(fd, MAGISK_CON);
			fchown(fd, 0, 0);
		}
		close(fd);
	}
}

void restorecon() {
	int fd = xopen(SELINUX_CONTEXT, O_WRONLY | O_CLOEXEC);
	if (write(fd, ADB_CON, sizeof(ADB_CON)) >= 0)
		lsetfilecon(SECURE_DIR, ADB_CON);
	close(fd);
	lsetfilecon(MODULEROOT, SYSTEM_CON);
	fd = xopen(MODULEROOT, O_RDONLY | O_CLOEXEC);
	restore_syscon(fd);
	close(fd);
	fd = xopen(DATABIN, O_RDONLY | O_CLOEXEC);
	restore_magiskcon(fd);
	close(fd);
}

void restore_tmpcon() {
	if (MAGISKTMP == "/sbin")
		setfilecon(MAGISKTMP.data(), ROOT_CON);
	else
		chmod(MAGISKTMP.data(), 0700);

	auto dir = xopen_dir(MAGISKTMP.data());
	int dfd = dirfd(dir.get());

	for (dirent *entry; (entry = xreaddir(dir.get()));) {
		if (entry->d_name == "magisk"sv || entry->d_name == "magiskinit"sv)
			setfilecon_at(dfd, entry->d_name, MAGISK_CON);
		else
			setfilecon_at(dfd, entry->d_name, ROOT_CON);
	}
}
