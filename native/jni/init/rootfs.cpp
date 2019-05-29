#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <magisk.h>
#include <utils.h>

#include "init.h"
#include "magiskrc.h"

#ifdef USE_64BIT
#define LIBNAME "lib64"
#else
#define LIBNAME "lib"
#endif

using namespace std;

static void patch_socket_name(const char *path) {
	uint8_t *buf;
	char name[sizeof(MAIN_SOCKET)];
	size_t size;
	mmap_rw(path, buf, size);
	for (int i = 0; i < size; ++i) {
		if (memcmp(buf + i, MAIN_SOCKET, sizeof(MAIN_SOCKET)) == 0) {
			gen_rand_str(name, sizeof(name));
			memcpy(buf + i, name, sizeof(name));
			i += sizeof(name);
		}
	}
	munmap(buf, size);
}

constexpr const char wrapper[] =
"#!/system/bin/sh\n"
"export LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH:/apex/com.android.runtime/" LIBNAME "\"\n"
"exec /sbin/magisk.bin \"$0\" \"$@\"\n"
;

void MagiskInit::setup_rootfs() {
	bool patch_init = patch_sepolicy();

	if (cmd.system_as_root) {
		// Clone rootfs
		LOGD("Clone root dir from system to rootfs\n");
		int system_root = xopen("/system_root", O_RDONLY | O_CLOEXEC);
		clone_dir(system_root, root, false);
		close(system_root);
	}

	if (patch_init) {
		constexpr char SYSTEM_INIT[] = "/system/bin/init";
		// If init is symlink, copy it to rootfs so we can patch
		if (is_lnk("/init"))
			cp_afc(SYSTEM_INIT, "/init");

		char *addr;
		size_t size;
		mmap_rw("/init", addr, size);
		for (char *p = addr; p < addr + size; ++p) {
			if (memcmp(p, SPLIT_PLAT_CIL, sizeof(SPLIT_PLAT_CIL)) == 0) {
				// Force init to load /sepolicy
				LOGD("Remove from init: " SPLIT_PLAT_CIL "\n");
				memset(p, 'x', sizeof(SPLIT_PLAT_CIL) - 1);
				p += sizeof(SPLIT_PLAT_CIL) - 1;
			} else if (memcmp(p, SYSTEM_INIT, sizeof(SYSTEM_INIT)) == 0) {
				// Force execute /init instead of /system/bin/init
				LOGD("Patch init: [/system/bin/init] -> [/init]\n");
				strcpy(p, "/init");
				p += sizeof(SYSTEM_INIT) - 1;
			}
		}
		munmap(addr, size);
	}

	// Handle ramdisk overlays
	int fd = open("/overlay", O_RDONLY | O_CLOEXEC);
	if (fd >= 0) {
		LOGD("Merge overlay folder\n");
		mv_dir(fd, root);
		close(fd);
		rmdir("/overlay");
	}

	// Patch init.rc
	FILE *rc = xfopen("/init.p.rc", "we");
	file_readline("/init.rc", [&](auto line) -> bool {
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
	char pfd_svc[8], ls_svc[8], bc_svc[8];
	// Make sure to be unique
	pfd_svc[0] = 'a';
	ls_svc[0] = '0';
	bc_svc[0] = 'A';
	gen_rand_str(pfd_svc + 1, sizeof(pfd_svc) - 1);
	gen_rand_str(ls_svc + 1, sizeof(ls_svc) - 1);
	gen_rand_str(bc_svc + 1, sizeof(bc_svc) - 1);
	LOGD("Inject magisk services: [%s] [%s] [%s]\n", pfd_svc, ls_svc, bc_svc);
	fprintf(rc, magiskrc, pfd_svc, pfd_svc, ls_svc, bc_svc, bc_svc);
	fclose(rc);
	clone_attr("/init.rc", "/init.p.rc");
	rename("/init.p.rc", "/init.rc");

	// Don't let init run in init yet
	lsetfilecon("/init", "u:object_r:rootfs:s0");

	// Create hardlink mirror of /sbin to /root
	mkdir("/root", 0750);
	clone_attr("/sbin", "/root");
	int rootdir = xopen("/root", O_RDONLY | O_CLOEXEC);
	int sbin = xopen("/sbin", O_RDONLY | O_CLOEXEC);
	link_dir(sbin, rootdir);
	close(sbin);

	LOGD("Mount /sbin tmpfs overlay\n");
	xmount("tmpfs", "/sbin", "tmpfs", 0, "mode=755");
	sbin = xopen("/sbin", O_RDONLY | O_CLOEXEC);

	char path[64];

	// Create symlinks pointing back to /root
	DIR *dir = xfdopendir(rootdir);
	struct dirent *entry;
	while((entry = xreaddir(dir))) {
		if (entry->d_name == "."sv || entry->d_name == ".."sv)
			continue;
		sprintf(path, "/root/%s", entry->d_name);
		xsymlinkat(path, sbin, entry->d_name);
	}

	// Dump binaries
	mkdir(MAGISKTMP, 0755);
	fd = xopen(MAGISKTMP "/config", O_WRONLY | O_CREAT, 0000);
	write(fd, config.buf, config.sz);
	close(fd);
	fd = xopen("/sbin/magiskinit", O_WRONLY | O_CREAT, 0755);
	write(fd, self.buf, self.sz);
	close(fd);
	if (access("/system/apex", F_OK) == 0) {
		LOGD("APEX detected, use wrapper\n");
		dump_magisk("/sbin/magisk.bin", 0755);
		patch_socket_name("/sbin/magisk.bin");
		fd = xopen("/sbin/magisk", O_WRONLY | O_CREAT, 0755);
		write(fd, wrapper, sizeof(wrapper) - 1);
		close(fd);
	} else {
		dump_magisk("/sbin/magisk", 0755);
		patch_socket_name("/sbin/magisk");
	}

	// Create applet symlinks
	for (int i = 0; applet_names[i]; ++i) {
		sprintf(path, "/sbin/%s", applet_names[i]);
		xsymlink("/sbin/magisk", path);
	}
	xsymlink("/sbin/magiskinit", "/sbin/magiskpolicy");
	xsymlink("/sbin/magiskinit", "/sbin/supolicy");

	close(rootdir);
	close(sbin);
}

bool MagiskInit::patch_sepolicy() {
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
	dump_policydb("/sepolicy");

	// Load policy to kernel so we can label rootfs
	if (load_sepol) {
		LOGD("sepol: preload sepolicy\n");
		dump_policydb(SELINUX_LOAD);
	}

	// Remove OnePlus stupid debug sepolicy and use our own
	if (access("/sepolicy_debug", F_OK) == 0) {
		unlink("/sepolicy_debug");
		link("/sepolicy", "/sepolicy_debug");
	}

	// Enable selinux functions
	selinux_builtin_impl();

	return patch_init;
}
