#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>
#include <vector>

#include <xz.h>
#include <magisk.h>
#include <cpio.h>
#include <utils.h>
#include <flags.h>

#include "binaries.h"
#ifdef USE_64BIT
#include "binaries_arch64.h"
#else
#include "binaries_arch.h"
#endif

#include "init.h"

using namespace std;

constexpr const char *init_applet[] =
		{ "magiskpolicy", "supolicy", "magisk", nullptr };
constexpr int (*init_applet_main[])(int, char *[]) =
		{ magiskpolicy_main, magiskpolicy_main, magisk_proxy_main, nullptr };

#ifdef MAGISK_DEBUG
static FILE *kmsg;
static int vprintk(const char *fmt, va_list ap) {
	fprintf(kmsg, "magiskinit: ");
	return vfprintf(kmsg, fmt, ap);
}
static void setup_klog() {
	mknod("/kmsg", S_IFCHR | 0666, makedev(1, 11));
	int fd = xopen("/kmsg", O_WRONLY | O_CLOEXEC);
	kmsg = fdopen(fd, "w");
	setbuf(kmsg, nullptr);
	unlink("/kmsg");
	log_cb.d = log_cb.i = log_cb.w = log_cb.e = vprintk;
	log_cb.ex = nop_ex;

	// Prevent file descriptor confusion
	mknod("/null", S_IFCHR | 0666, makedev(1, 3));
	int null = xopen("/null", O_RDWR | O_CLOEXEC);
	unlink("/null");
	xdup3(null, STDIN_FILENO, O_CLOEXEC);
	xdup3(null, STDOUT_FILENO, O_CLOEXEC);
	xdup3(null, STDERR_FILENO, O_CLOEXEC);
	if (null > STDERR_FILENO)
		close(null);
}
#else
#define setup_klog(...)
#endif

static bool unxz(int fd, const uint8_t *buf, size_t size) {
	uint8_t out[8192];
	xz_crc32_init();
	struct xz_dec *dec = xz_dec_init(XZ_DYNALLOC, 1 << 26);
	struct xz_buf b = {
			.in = buf,
			.in_pos = 0,
			.in_size = size,
			.out = out,
			.out_pos = 0,
			.out_size = sizeof(out)
	};
	enum xz_ret ret;
	do {
		ret = xz_dec_run(dec, &b);
		if (ret != XZ_OK && ret != XZ_STREAM_END)
			return false;
		write(fd, out, b.out_pos);
		b.out_pos = 0;
	} while (b.in_pos != size);
	return true;
}

static void decompress_ramdisk() {
	constexpr char tmp[] = "tmp.cpio";
	constexpr char ramdisk_xz[] = "ramdisk.cpio.xz";
	if (access(ramdisk_xz, F_OK))
		return;
	LOGD("Decompressing ramdisk from %s\n", ramdisk_xz);
	uint8_t *buf;
	size_t sz;
	mmap_ro(ramdisk_xz, buf, sz);
	int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC);
	unxz(fd, buf, sz);
	munmap(buf, sz);
	close(fd);
	cpio_mmap cpio(tmp);
	cpio.extract();
	unlink(tmp);
	unlink(ramdisk_xz);
}

int dump_magisk(const char *path, mode_t mode) {
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
	if (fd < 0)
		return 1;
	if (!unxz(fd, magisk_xz, sizeof(magisk_xz)))
		return 1;
	close(fd);
	return 0;
}

static int dump_manager(const char *path, mode_t mode) {
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
	if (fd < 0)
		return 1;
	if (!unxz(fd, manager_xz, sizeof(manager_xz)))
		return 1;
	close(fd);
	return 0;
}

class RecoveryInit : public BaseInit {
public:
	RecoveryInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
	void start() override {
		LOGD("Ramdisk is recovery, abort\n");
		rename("/.backup/init", "/init");
		rm_rf("/.backup");
		exec_init();
	}
};

class TestInit : public SARInit {
public:
	TestInit(char *argv[], cmdline *cmd) : SARInit(argv, cmd) {};
	void start() override {
		early_mount();
		patch_rootdir();
		cleanup();
	}
};

static void setup_test(const char *dir) {
	// Log to console
	cmdline_logging();
	log_cb.ex = nop_ex;

	// Switch to isolate namespace
	xunshare(CLONE_NEWNS);
	xmount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr);

	// Unmount everything in reverse
	vector<string> mounts;
	parse_mnt("/proc/mounts", [&](mntent *me) {
		if (me->mnt_dir != "/"sv)
			mounts.emplace_back(me->mnt_dir);
		return true;
	});
	for (auto m = mounts.rbegin(); m != mounts.rend(); ++m)
		xumount(m->data());

	// chroot jail
	chdir(dir);
	chroot(".");
	chdir("/");
}

int main(int argc, char *argv[]) {
	umask(0);

	for (int i = 0; init_applet[i]; ++i) {
		if (strcmp(basename(argv[0]), init_applet[i]) == 0)
			return (*init_applet_main[i])(argc, argv);
	}

	if (argc > 1 && strcmp(argv[1], "-x") == 0) {
		if (strcmp(argv[2], "magisk") == 0)
			return dump_magisk(argv[3], 0755);
		else if (strcmp(argv[2], "manager") == 0)
			return dump_manager(argv[3], 0644);
	}

	if (argc > 1 && argv[1] == "selinux_setup"sv) {
		auto init = make_unique<SecondStageInit>(argv);
		init->start();
	}

#ifdef MAGISK_DEBUG
	bool run_test = getenv("INIT_TEST") != nullptr;
#else
	constexpr bool run_test = false;
#endif

	if (run_test) {
		setup_test(dirname(argv[0]));
	} else {
		if (getpid() != 1)
			return 1;
		setup_klog();
	}

	cmdline cmd{};
	load_kernel_info(&cmd);

	unique_ptr<BaseInit> init;
	if (run_test) {
		init = make_unique<TestInit>(argv, &cmd);
	} else if (cmd.force_normal_boot) {
		init = make_unique<FirstStageInit>(argv, &cmd);
	} else if (cmd.system_as_root) {
		if (access("/overlay", F_OK) == 0)  /* Compatible mode */
			init = make_unique<SARCompatInit>(argv, &cmd);
		else
			init = make_unique<SARInit>(argv, &cmd);
	} else {
		decompress_ramdisk();
		if (access("/sbin/recovery", F_OK) == 0)
			init = make_unique<RecoveryInit>(argv, &cmd);
		else
			init = make_unique<LegacyInit>(argv, &cmd);
	}

	// Run the main routine
	init->start();
	exit(1);
}
