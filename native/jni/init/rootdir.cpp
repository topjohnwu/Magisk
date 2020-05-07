#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <libgen.h>

#include <magisk.hpp>
#include <magiskpolicy.hpp>
#include <utils.hpp>
#include <socket.hpp>

#include "init.hpp"
#include "magiskrc.inc"

#ifdef USE_64BIT
#define LIBNAME "lib64"
#else
#define LIBNAME "lib"
#endif

using namespace std;

static vector<string> rc_list;

static void patch_init_rc(const char *src, const char *dest, const char *tmp_dir) {
	FILE *rc = xfopen(dest, "we");
	file_readline(src, [=](string_view line) -> bool {
		// Do not start vaultkeeper
		if (str_contains(line, "start vaultkeeper")) {
			LOGD("Remove vaultkeeper\n");
			return true;
		}
		// Do not run flash_recovery
		if (str_starts(line, "service flash_recovery")) {
			LOGD("Remove flash_recovery\n");
			fprintf(rc, "service flash_recovery /system/bin/xxxxx\n");
			return true;
		}
		// Else just write the line
		fprintf(rc, "%s", line.data());
		return true;
	});

	fprintf(rc, "\n");

	// Inject custom rc scripts
	for (auto &script : rc_list) {
		// Replace template arguments of rc scripts with dynamic paths
		replace_all(script, "${MAGISKTMP}", tmp_dir);
		fprintf(rc, "\n%s\n", script.data());
	}
	rc_list.clear();

	// Inject Magisk rc scripts
	char pfd_svc[16], ls_svc[16], bc_svc[16];
	gen_rand_str(pfd_svc, sizeof(pfd_svc));
	gen_rand_str(ls_svc, sizeof(ls_svc));
	gen_rand_str(bc_svc, sizeof(bc_svc));
	LOGD("Inject magisk services: [%s] [%s] [%s]\n", pfd_svc, ls_svc, bc_svc);
	fprintf(rc, MAGISK_RC, tmp_dir, pfd_svc, ls_svc, bc_svc);

	fclose(rc);
	clone_attr(src, dest);
}

static void load_overlay_rc(const char *overlay) {
	auto dir = open_dir(overlay);
	if (!dir) return;

	int dfd = dirfd(dir.get());
	// Do not allow overwrite init.rc
	unlinkat(dfd, "init.rc", 0);
	for (dirent *entry; (entry = xreaddir(dir.get()));) {
		if (strend(entry->d_name, ".rc") == 0) {
			LOGD("Found rc script [%s]\n", entry->d_name);
			int rc = xopenat(dfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			rc_list.push_back(fd_full_read(rc));
			close(rc);
			unlinkat(dfd, entry->d_name, 0);
		}
	}
}

int raw_data_patch(void *addr, size_t sz, str_pairs list) {
	int count = 0;
	for (uint8_t *p = (uint8_t *)addr, *eof = (uint8_t *)addr + sz; p < eof; ++p) {
		for (auto &[from, to] : list) {
			if (memcmp(p, from.data(), from.length() + 1) == 0) {
				LOGD("Replace [%s] -> [%s]\n", from.data(), to.data());
				memset(p, 0, from.length());
				memcpy(p, to.data(), to.length());
				++count;
				p += from.length();
			}
		}
	}
	return count;
}

void RootFSInit::setup_rootfs() {
	if (patch_sepolicy("/sepolicy")) {
		char *addr;
		size_t size;
		mmap_rw("/init", addr, size);
		raw_data_patch(addr, size, {make_pair(SPLIT_PLAT_CIL, "xxx")});
		munmap(addr, size);
	}

	// Handle overlays
	if (access("/overlay.d", F_OK) == 0) {
		LOGD("Merge overlay.d\n");
		load_overlay_rc("/overlay.d");
		mv_path("/overlay.d", "/");
	}

	patch_init_rc("/init.rc", "/init.p.rc", "/sbin");
	rename("/init.p.rc", "/init.rc");

	// Create hardlink mirror of /sbin to /root
	mkdir("/root", 0750);
	clone_attr("/sbin", "/root");
	link_path("/sbin", "/root");

	// Dump magiskinit as magisk
	int fd = xopen("/sbin/magisk", O_WRONLY | O_CREAT, 0755);
	write(fd, self.buf, self.sz);
	close(fd);
}

bool MagiskInit::patch_sepolicy(const char *file) {
	bool patch_init = false;

	if (access(SPLIT_PLAT_CIL, R_OK) == 0) {
		LOGD("sepol: split policy\n");
		patch_init = true;
	} else if (access("/sepolicy", R_OK) == 0) {
		LOGD("sepol: monolithic policy\n");
		load_policydb("/sepolicy");
	} else {
		LOGD("sepol: no selinux\n");
		return false;
	}

	if (access(SELINUX_VERSION, F_OK) != 0) {
		// Mount selinuxfs to communicate with kernel
		xmount("selinuxfs", SELINUX_MNT, "selinuxfs", 0, nullptr);
		mount_list.emplace_back(SELINUX_MNT);
	}

	if (patch_init)
		load_split_cil();

	sepol_magisk_rules();
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, ALL, ALL);

	// Custom rules
	if (auto dir = open_dir(persist_dir.data()); dir) {
		for (dirent *entry; (entry = xreaddir(dir.get()));) {
			auto rule = persist_dir + "/" + entry->d_name + "/sepolicy.rule";
			if (access(rule.data(), R_OK) == 0) {
				LOGD("Loading custom sepolicy patch: %s\n", rule.data());
				load_rule_file(rule.data());
			}
		}
	}

	dump_policydb(file);
	destroy_policydb();

	// Remove OnePlus stupid debug sepolicy and use our own
	if (access("/sepolicy_debug", F_OK) == 0) {
		unlink("/sepolicy_debug");
		link("/sepolicy", "/sepolicy_debug");
	}

	return patch_init;
}

static void recreate_sbin(const char *mirror, bool use_bind_mount) {
	auto dp = xopen_dir(mirror);
	int src = dirfd(dp.get());
	char buf[4096];
	for (dirent *entry; (entry = xreaddir(dp.get()));) {
		string sbin_path = "/sbin/"s + entry->d_name;
		struct stat st;
		fstatat(src, entry->d_name, &st, AT_SYMLINK_NOFOLLOW);
		if (S_ISLNK(st.st_mode)) {
			xreadlinkat(src, entry->d_name, buf, sizeof(buf));
			xsymlink(buf, sbin_path.data());
		} else {
			sprintf(buf, "%s/%s", mirror, entry->d_name);
			if (use_bind_mount) {
				auto mode = st.st_mode & 0777;
				// Create dummy
				if (S_ISDIR(st.st_mode))
					xmkdir(sbin_path.data(), mode);
				else
					close(xopen(sbin_path.data(), O_CREAT | O_WRONLY | O_CLOEXEC, mode));

				xmount(buf, sbin_path.data(), nullptr, MS_BIND, nullptr);
			} else {
				xsymlink(buf, sbin_path.data());
			}
		}
	}
}

static string magic_mount_list;

static void magic_mount(const string &sdir, const string &ddir = "") {
	auto dir = xopen_dir(sdir.data());
	for (dirent *entry; (entry = xreaddir(dir.get()));) {
		string src = sdir + "/" + entry->d_name;
		string dest = ddir + "/" + entry->d_name;
		if (access(dest.data(), F_OK) == 0) {
			if (entry->d_type == DT_DIR) {
				// Recursive
				magic_mount(src, dest);
			} else {
				LOGD("Mount [%s] -> [%s]\n", src.data(), dest.data());
				xmount(src.data(), dest.data(), nullptr, MS_BIND, nullptr);
				magic_mount_list += dest;
				magic_mount_list += '\n';
			}
		}
	}
}

#define ROOTMIR     MIRRDIR "/system_root"
#define ROOTBLK     BLOCKDIR "/system_root"
#define MONOPOLICY  "/sepolicy"
#define LIBSELINUX  "/system/" LIBNAME "/libselinux.so"
#define NEW_INITRC  "/system/etc/init/hw/init.rc"

void SARBase::patch_rootdir() {
	char tmp_dir[16];
	const char *sepol;

	char *p;
	if (access("/sbin", F_OK) == 0) {
		p = tmp_dir + sprintf(tmp_dir, "%s", "/sbin");
		sepol = "/sbin/.se";
	} else {
		p = tmp_dir + sprintf(tmp_dir, "%s", "/dev/");
		p += gen_rand_str(p, 8);
		xmkdir(tmp_dir, 0);
		sepol = "/dev/.se";
	}

	setup_tmp(tmp_dir, self, config);
	persist_dir = MIRRDIR "/persist/magisk";

	chdir(tmp_dir);

	// Mount system_root mirror
	struct stat st;
	xstat("/", &st);
	xmkdir(ROOTMIR, 0755);
	mknod(ROOTBLK, S_IFBLK | 0600, st.st_dev);
	strcpy(p, "/" ROOTBLK);
	if (xmount(tmp_dir, ROOTMIR, "ext4", MS_RDONLY, nullptr))
		xmount(tmp_dir, ROOTMIR, "erofs", MS_RDONLY, nullptr);
	*p = '\0';

	// Recreate original sbin structure if necessary
	if (tmp_dir == "/sbin"sv)
		recreate_sbin(ROOTMIR "/sbin", true);

	// Patch init
	raw_data init;
	int src = xopen("/init", O_RDONLY | O_CLOEXEC);
	fd_full_read(src, init.buf, init.sz);
	int patch_count = raw_data_patch(init.buf, init.sz, {
		make_pair(SPLIT_PLAT_CIL, "xxx"), /* Force loading monolithic sepolicy */
		make_pair(MONOPOLICY, sepol)      /* Redirect /sepolicy to custom path */
	});
	xmkdir(ROOTOVL, 0);
	int dest = xopen(ROOTOVL "/init", O_CREAT | O_WRONLY | O_CLOEXEC, 0);
	xwrite(dest, init.buf, init.sz);
	fclone_attr(src, dest);
	close(src);
	close(dest);

	if (patch_count != 2 && access(LIBSELINUX, F_OK) == 0) {
		// init is dynamically linked, need to patch libselinux
		raw_data lib;
		full_read(LIBSELINUX, lib.buf, lib.sz);
		raw_data_patch(lib.buf, lib.sz, {make_pair(MONOPOLICY, sepol)});
		xmkdirs(dirname(ROOTOVL LIBSELINUX), 0755);
		dest = xopen(ROOTOVL LIBSELINUX, O_CREAT | O_WRONLY | O_CLOEXEC, 0);
		xwrite(dest, lib.buf, lib.sz);
		close(dest);
		clone_attr(LIBSELINUX, ROOTOVL LIBSELINUX);
	}

	// sepolicy
	patch_sepolicy(sepol);

	// Restore backup files
	struct sockaddr_un sun;
	int sockfd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (connect(sockfd, (struct sockaddr*) &sun, setup_sockaddr(&sun, INIT_SOCKET)) == 0) {
		LOGD("ACK init daemon to write backup files\n");
		// Let daemon know where tmp_dir is
		write_string(sockfd, tmp_dir);
		// Wait for daemon to finish restoring files
		int ack;
		read(sockfd, &ack, sizeof(ack));
	} else {
		LOGD("Restore backup files locally\n");
		restore_folder(ROOTOVL, overlays);
		overlays.clear();
	}
	close(sockfd);

	// Handle overlay.d
	load_overlay_rc(ROOTOVL);
	if (access(ROOTOVL "/sbin", F_OK) == 0) {
		// Move files in overlay.d/sbin into tmp_dir
		mv_path(ROOTOVL "/sbin", ".");
	}

	// Patch init.rc
	if (access("/init.rc", F_OK) == 0) {
		patch_init_rc("/init.rc", ROOTOVL "/init.rc", tmp_dir);
	} else {
		// Android 11's new init.rc
		xmkdirs(dirname(ROOTOVL NEW_INITRC), 0755);
		patch_init_rc(NEW_INITRC, ROOTOVL NEW_INITRC, tmp_dir);
	}

	// Mount rootdir
	magic_mount(ROOTOVL);
	dest = xopen(ROOTMNT, O_WRONLY | O_CREAT | O_CLOEXEC, 0);
	write(dest, magic_mount_list.data(), magic_mount_list.length());
	close(dest);

	chdir("/");
}

int magisk_proxy_main(int argc, char *argv[]) {
	setup_klog();

	raw_data config;
	raw_data self;

	full_read("/sbin/magisk", self.buf, self.sz);
	full_read("/.backup/.magisk", config.buf, config.sz);

	xmount(nullptr, "/", nullptr, MS_REMOUNT, nullptr);

	unlink("/sbin/magisk");
	rm_rf("/.backup");

	setup_tmp("/sbin", self, config);

	// Create symlinks pointing back to /root
	recreate_sbin("/root", false);

	setenv("REMOUNT_ROOT", "1", 1);
	execv("/sbin/magisk", argv);

	return 1;
}
