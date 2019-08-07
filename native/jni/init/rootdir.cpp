#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>

#include <magisk.h>
#include <magiskpolicy.h>
#include <utils.h>

#include "init.h"
#include "flags.h"
#include "magiskrc.h"

#ifdef USE_64BIT
#define LIBNAME "lib64"
#else
#define LIBNAME "lib"
#endif

using namespace std;

static void patch_socket_name(const char *path) {
	char *buf;
	size_t size;
	mmap_rw(path, buf, size);
	for (int i = 0; i < size; ++i) {
		if (memcmp(buf + i, MAIN_SOCKET, sizeof(MAIN_SOCKET)) == 0) {
			gen_rand_str(buf + i, sizeof(MAIN_SOCKET));
			i += sizeof(MAIN_SOCKET);
		}
	}
	munmap(buf, size);
}

static vector<raw_data> rc_list;

static void patch_init_rc(FILE *rc) {
	file_readline("/init.rc", [=](string_view line) -> bool {
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
	for (auto &d : rc_list)
		fprintf(rc, "\n%s\n", d.buf);
	rc_list.clear();

	// Inject Magisk rc scripts
	char pfd_svc[16], ls_svc[16], bc_svc[16];
	gen_rand_str(pfd_svc, sizeof(pfd_svc));
	gen_rand_str(ls_svc, sizeof(ls_svc));
	gen_rand_str(bc_svc, sizeof(bc_svc));
	LOGD("Inject magisk services: [%s] [%s] [%s]\n", pfd_svc, ls_svc, bc_svc);
	fprintf(rc, magiskrc, pfd_svc, pfd_svc, ls_svc, bc_svc, bc_svc);
}

static void load_overlay_rc(int dirfd) {
	// Do not allow overwrite init.rc
	unlinkat(dirfd, "init.rc", 0);
	DIR *dir = fdopendir(dirfd);
	for (dirent *entry; (entry = readdir(dir));) {
		if (strend(entry->d_name, ".rc") == 0) {
			LOGD("Found rc script [%s]\n", entry->d_name);
			int rc = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			raw_data data;
			fd_full_read(rc, data.buf, data.sz);
			close(rc);
			rc_list.push_back(std::move(data));
			unlinkat(dirfd, entry->d_name, 0);
		}
	}
	rewinddir(dir);
}

void RootFSInit::setup_rootfs() {
	if (patch_sepolicy()) {
		char *addr;
		size_t size;
		mmap_rw("/init", addr, size);
		for (char *p = addr; p < addr + size; ++p) {
			if (memcmp(p, SPLIT_PLAT_CIL, sizeof(SPLIT_PLAT_CIL)) == 0) {
				// Force init to load /sepolicy
				LOGD("Remove from init: " SPLIT_PLAT_CIL "\n");
				memset(p, 'x', sizeof(SPLIT_PLAT_CIL) - 1);
				break;
			}
		}
		munmap(addr, size);
	}

	// Handle legacy overlays
	int fd = open("/overlay", O_RDONLY | O_CLOEXEC);
	if (fd >= 0) {
		LOGD("Merge overlay folder\n");
		mv_dir(fd, root);
		close(fd);
		rmdir("/overlay");
	}

	// Handle overlays
	fd = open("/overlay.d", O_RDONLY | O_CLOEXEC);
	if (fd >= 0) {
		LOGD("Merge overlay.d\n");
		load_overlay_rc(fd);
		mv_dir(fd, root);
		close(fd);
		rmdir("/overlay.d");
	}

	// Patch init.rc
	FILE *rc = xfopen("/init.p.rc", "we");
	patch_init_rc(rc);
	fclose(rc);
	clone_attr("/init.rc", "/init.p.rc");
	rename("/init.p.rc", "/init.rc");

	// Create hardlink mirror of /sbin to /root
	mkdir("/root", 0750);
	clone_attr("/sbin", "/root");
	int rootdir = xopen("/root", O_RDONLY | O_CLOEXEC);
	int sbin = xopen("/sbin", O_RDONLY | O_CLOEXEC);
	link_dir(sbin, rootdir);
	close(sbin);

	// Dump magiskinit as magisk
	fd = xopen("/sbin/magisk", O_WRONLY | O_CREAT, 0755);
	write(fd, self.buf, self.sz);
	close(fd);
}

void SARCompatInit::setup_rootfs() {
	// Clone rootfs
	LOGD("Clone root dir from system to rootfs\n");
	int system_root = xopen("/system_root", O_RDONLY | O_CLOEXEC);
	clone_dir(system_root, root, false);
	close(system_root);

	RootFSInit::setup_rootfs();
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

	// Mount selinuxfs to communicate with kernel
	xmount("selinuxfs", SELINUX_MNT, "selinuxfs", 0, nullptr);

	if (patch_init)
		load_split_cil();

	sepol_magisk_rules();
	sepol_allow(SEPOL_PROC_DOMAIN, ALL, ALL, ALL);
	dump_policydb(file);

	// Remove OnePlus stupid debug sepolicy and use our own
	if (access("/sepolicy_debug", F_OK) == 0) {
		unlink("/sepolicy_debug");
		link("/sepolicy", "/sepolicy_debug");
	}

	return patch_init;
}

constexpr const char wrapper[] =
"#!/system/bin/sh\n"
"export LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH:/apex/com.android.runtime/" LIBNAME "\"\n"
"exec /sbin/magisk.bin \"$0\" \"$@\"\n"
;

static void sbin_overlay(const raw_data &self, const raw_data &config) {
	LOGD("Mount /sbin tmpfs overlay\n");
	xmount("tmpfs", "/sbin", "tmpfs", 0, "mode=755");

	// Dump binaries
	xmkdir(MAGISKTMP, 0755);
	int fd = xopen(MAGISKTMP "/config", O_WRONLY | O_CREAT, 0000);
	xwrite(fd, config.buf, config.sz);
	close(fd);
	fd = xopen("/sbin/magiskinit", O_WRONLY | O_CREAT, 0755);
	xwrite(fd, self.buf, self.sz);
	close(fd);
	if (access("/system/apex", F_OK) == 0) {
		LOGD("APEX detected, use wrapper\n");
		dump_magisk("/sbin/magisk.bin", 0755);
		patch_socket_name("/sbin/magisk.bin");
		fd = xopen("/sbin/magisk", O_WRONLY | O_CREAT, 0755);
		xwrite(fd, wrapper, sizeof(wrapper) - 1);
		close(fd);
	} else {
		dump_magisk("/sbin/magisk", 0755);
		patch_socket_name("/sbin/magisk");
	}

	// Create applet symlinks
	char path[64];
	for (int i = 0; applet_names[i]; ++i) {
		sprintf(path, "/sbin/%s", applet_names[i]);
		xsymlink("./magisk", path);
	}
	xsymlink("./magiskinit", "/sbin/magiskpolicy");
	xsymlink("./magiskinit", "/sbin/supolicy");
}

#define ROOTMIR MIRRDIR "/system_root"
#define ROOTBLK BLOCKDIR "/system_root"
#define MONOPOLICY  "/sepolicy"
#define PATCHPOLICY "/sbin/.se"
#define LIBSELINUX  "/system/" LIBNAME "/libselinux.so"

static string mount_list;

static void magic_mount(int dirfd, const string &path) {
	DIR *dir = xfdopendir(dirfd);
	for (dirent *entry; (entry = readdir(dir));) {
		if (entry->d_name == "."sv || entry->d_name == ".."sv)
			continue;
		string dest = path + "/" + entry->d_name;
		if (access(dest.data(), F_OK) == 0) {
			if (entry->d_type == DT_DIR) {
				// Recursive
				int fd = xopenat(dirfd, entry->d_name, O_RDONLY | O_CLOEXEC);
				magic_mount(fd, dest);
				close(fd);
			} else {
				string src = ROOTOVL + dest;
				LOGD("Mount [%s] -> [%s]\n", src.data(), dest.data());
				xmount(src.data(), dest.data(), nullptr, MS_BIND, nullptr);
				mount_list += dest;
				mount_list += '\n';
			}
		}
	}
}

void SARCommon::patch_rootdir() {
	sbin_overlay(self, config);

	// Mount system_root mirror
	xmkdir(MIRRDIR, 0);
	xmkdir(ROOTMIR, 0);
	xmkdir(BLOCKDIR, 0);
	mknod(ROOTBLK, S_IFBLK | 0600, system_dev);
	if (xmount(ROOTBLK, ROOTMIR, "ext4", MS_RDONLY, nullptr))
		xmount(ROOTBLK, ROOTMIR, "erofs", MS_RDONLY, nullptr);

	// Recreate original sbin structure
	int src = xopen(ROOTMIR "/sbin", O_RDONLY | O_CLOEXEC);
	int dest = xopen("/sbin", O_RDONLY | O_CLOEXEC);
	DIR *fp = fdopendir(src);
	struct dirent *entry;
	struct stat st;
	char buf[256];
	while ((entry = xreaddir(fp))) {
		if (entry->d_name == "."sv || entry->d_name == ".."sv)
			continue;
		fstatat(src, entry->d_name, &st, AT_SYMLINK_NOFOLLOW);
		if (S_ISLNK(st.st_mode)) {
			xreadlinkat(src, entry->d_name, buf, sizeof(buf));
			xsymlinkat(buf, dest, entry->d_name);
		} else {
			char spath[256];
			sprintf(buf, "/sbin/%s", entry->d_name);
			sprintf(spath, ROOTMIR "/sbin/%s", entry->d_name);
			// Create dummy
			if (S_ISDIR(st.st_mode))
				xmkdir(buf, st.st_mode & 0777);
			else
				close(xopen(buf, O_CREAT | O_WRONLY | O_CLOEXEC, st.st_mode & 0777));
			xmount(spath, buf, nullptr, MS_BIND, nullptr);
		}
	}
	close(src);
	close(dest);

	// Patch init
	raw_data init;
	file_attr attr;
	bool redirect = false;
	src = xopen("/init", O_RDONLY | O_CLOEXEC);
	fd_full_read(src, init.buf, init.sz);
	fgetattr(src, &attr);
	close(src);
	uint8_t *eof = init.buf + init.sz;
	for (uint8_t *p = init.buf; p < eof; ++p) {
		if (memcmp(p, SPLIT_PLAT_CIL, sizeof(SPLIT_PLAT_CIL)) == 0) {
			// Force init to load monolithic policy
			LOGD("Remove from init: " SPLIT_PLAT_CIL "\n");
			memset(p, 'x', sizeof(SPLIT_PLAT_CIL) - 1);
			p += sizeof(SPLIT_PLAT_CIL) - 1;
		} else if (memcmp(p, MONOPOLICY, sizeof(MONOPOLICY)) == 0) {
			// Redirect /sepolicy to tmpfs
			LOGD("Patch init [" MONOPOLICY "] -> [" PATCHPOLICY "]\n");
			memcpy(p, PATCHPOLICY, sizeof(PATCHPOLICY));
			redirect = true;
			p += sizeof(MONOPOLICY) - 1;
		}
	}
	xmkdir(ROOTOVL, 0);
	dest = xopen(ROOTOVL "/init", O_CREAT | O_WRONLY | O_CLOEXEC);
	xwrite(dest, init.buf, init.sz);
	fsetattr(dest, &attr);
	close(dest);

	// Patch libselinux
	if (!redirect) {
		raw_data lib;
		// init is dynamically linked, need to patch libselinux
		full_read(LIBSELINUX, lib.buf, lib.sz);
		getattr(LIBSELINUX, &attr);
		eof = lib.buf + lib.sz;
		for (uint8_t *p = lib.buf; p < eof; ++p) {
			if (memcmp(p, MONOPOLICY, sizeof(MONOPOLICY)) == 0) {
				// Redirect /sepolicy to tmpfs
				LOGD("Patch libselinux.so [" MONOPOLICY "] -> [" PATCHPOLICY "]\n");
				memcpy(p, PATCHPOLICY, sizeof(PATCHPOLICY));
				break;
			}
		}
		xmkdir(ROOTOVL "/system", 0755);
		xmkdir(ROOTOVL "/system/" LIBNAME, 0755);
		dest = xopen(ROOTOVL LIBSELINUX, O_CREAT | O_WRONLY | O_CLOEXEC);
		xwrite(dest, lib.buf, lib.sz);
		fsetattr(dest, &attr);
		close(dest);
	}

	// sepolicy
	patch_sepolicy(PATCHPOLICY);

	// Handle overlay
	if ((src = xopen("/dev/overlay.d", O_RDONLY | O_CLOEXEC)) >= 0) {
		load_overlay_rc(src);
		if (int fd = xopen("/dev/overlay.d/sbin", O_RDONLY | O_CLOEXEC); fd >= 0) {
			dest = xopen("/sbin", O_RDONLY | O_CLOEXEC);
			clone_dir(fd, dest);
			close(fd);
			close(dest);
			xmkdir(ROOTOVL "/sbin", 0);  // Prevent copying
		}
		dest = xopen(ROOTOVL, O_RDONLY | O_CLOEXEC);
		clone_dir(src, dest, false);
		rmdir(ROOTOVL "/sbin");
		close(src);
		close(dest);
		rm_rf("/dev/overlay.d");
	}

	// Patch init.rc
	FILE *rc = xfopen(ROOTOVL "/init.rc", "we");
	patch_init_rc(rc);
	fclose(rc);
	clone_attr("/init.rc", ROOTOVL "/init.rc");

	// Mount rootdir
	src = xopen(ROOTOVL, O_RDONLY | O_CLOEXEC);
	magic_mount(src, "");
	close(src);
	dest = xopen(ROOTMNT, O_WRONLY | O_CREAT | O_CLOEXEC);
	write(dest, mount_list.data(), mount_list.length());
	close(dest);
}

#define FSR "/first_stage_ramdisk"

void FirstStageInit::prepare() {
	// Find fstab
	DIR *dir = xopendir(FSR);
	if (!dir)
		return;
	dirent *de;
	string fstab(FSR "/");
	while ((de = readdir(dir))) {
		if (strstr(de->d_name, "fstab")) {
			fstab += de->d_name;
			break;
		}
	}
	closedir(dir);
	if (fstab.length() == sizeof(FSR))
		return;

	// Patch fstab
	string patched = fstab + ".p";
	FILE *fp = xfopen(patched.data(), "we");
	file_readline(fstab.data(), [=](string_view l) -> bool {
		if (l[0] == '#' || l.length() == 1)
			return true;
		char *line = (char *) l.data();
		int src0, src1, mnt0, mnt1, type0, type1, opt0, opt1, flag0, flag1;
		sscanf(line, "%n%*s%n %n%*s%n %n%*s%n %n%*s%n %n%*s%n",
			   &src0, &src1, &mnt0, &mnt1, &type0, &type1, &opt0, &opt1, &flag0, &flag1);
		const char *src, *mnt, *type, *opt, *flag;
		src = &line[src0];
		line[src1] = '\0';
		mnt = &line[mnt0];
		line[mnt1] = '\0';
		type = &line[type0];
		line[type1] = '\0';
		opt = &line[opt0];
		line[opt1] = '\0';
		flag = &line[flag0];
		line[flag1] = '\0';

		// Redirect system to system_root
		if (mnt == "/system"sv)
			mnt = "/system_root";

		fprintf(fp, "%s %s %s %s %s\n", src, mnt, type, opt, flag);
		return true;
	});
	fclose(fp);

	// Replace old fstab
	clone_attr(fstab.data(), patched.data());
	rename(patched.data(), fstab.data());

	// Move stuffs for next stage
	xmkdir(FSR "/system", 0755);
	xmkdir(FSR "/system/bin", 0755);
	rename("/init", FSR "/system/bin/init");
	symlink("/system/bin/init", FSR "/init");
	xmkdir(FSR "/.backup", 0);
	rename("/.backup/.magisk", FSR "/.backup/.magisk");
	rename("/overlay.d", FSR "/overlay.d");
}

#ifdef MAGISK_DEBUG
static FILE *kmsg;
static int vprintk(const char *fmt, va_list ap) {
	fprintf(kmsg, "magiskinit: ");
	return vfprintf(kmsg, fmt, ap);
}
static void setup_klog() {
	int fd = xopen("/proc/kmsg", O_WRONLY | O_CLOEXEC);
	kmsg = fdopen(fd, "w");
	setbuf(kmsg, nullptr);
	log_cb.d = log_cb.i = log_cb.w = log_cb.e = vprintk;
	log_cb.ex = nop_ex;
}
#else
#define setup_klog(...)
#endif

int magisk_proxy_main(int argc, char *argv[]) {
	setup_klog();

	raw_data config;
	raw_data self;

	full_read("/sbin/magisk", self.buf, self.sz);
	full_read("/.backup/.magisk", config.buf, config.sz);

	xmount(nullptr, "/", nullptr, MS_REMOUNT, nullptr);

	unlink("/sbin/magisk");
	rm_rf("/.backup");

	sbin_overlay(self, config);

	// Create symlinks pointing back to /root
	char path[256];
	int sbin = xopen("/sbin", O_RDONLY | O_CLOEXEC);
	DIR *dir = xopendir("/root");
	struct dirent *entry;
	while((entry = xreaddir(dir))) {
		if (entry->d_name == "."sv || entry->d_name == ".."sv)
			continue;
		sprintf(path, "/root/%s", entry->d_name);
		xsymlinkat(path, sbin, entry->d_name);
	}
	close(sbin);
	closedir(dir);

	setenv("REMOUNT_ROOT", "1", 1);
	execv("/sbin/magisk", argv);

	return 1;
}
