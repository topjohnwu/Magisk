#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>
#include <vector>

#include <xz.h>
#include <magisk.hpp>
#include <cpio.hpp>
#include <utils.hpp>
#include <flags.h>

#include "binaries.h"
#ifdef USE_64BIT
#include "binaries_arch64.h"
#else
#include "binaries_arch.h"
#endif

#include "init.hpp"

using namespace std;

constexpr int (*init_applet_main[])(int, char *[]) =
		{ magiskpolicy_main, magiskpolicy_main, nullptr };

#ifdef MAGISK_DEBUG
static FILE *kmsg;
static char kmsg_buf[4096];
static int vprintk(const char *fmt, va_list ap) {
	vsnprintf(kmsg_buf + 12, sizeof(kmsg_buf) - 12, fmt, ap);
	return fprintf(kmsg, "%s", kmsg_buf);
}
void setup_klog() {
	// Shut down first 3 fds
	int fd;
	if (access("/dev/null", W_OK) == 0) {
		fd = xopen("/dev/null", O_RDWR | O_CLOEXEC);
	} else {
		mknod("/null", S_IFCHR | 0666, makedev(1, 3));
		fd = xopen("/null", O_RDWR | O_CLOEXEC);
		unlink("/null");
	}
	xdup3(fd, STDIN_FILENO, O_CLOEXEC);
	xdup3(fd, STDOUT_FILENO, O_CLOEXEC);
	xdup3(fd, STDERR_FILENO, O_CLOEXEC);
	if (fd > STDERR_FILENO)
		close(fd);

	if (access("/dev/kmsg", W_OK) == 0) {
		fd = xopen("/dev/kmsg", O_WRONLY | O_CLOEXEC);
	} else {
		mknod("/kmsg", S_IFCHR | 0666, makedev(1, 11));
		fd = xopen("/kmsg", O_WRONLY | O_CLOEXEC);
		unlink("/kmsg");
	}

	kmsg = fdopen(fd, "w");
	setbuf(kmsg, nullptr);
	log_cb.d = log_cb.i = log_cb.w = log_cb.e = vprintk;
	log_cb.ex = nop_ex;
	strcpy(kmsg_buf, "magiskinit: ");
}
#else
void setup_klog() {}
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
	int fd = xopen(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	unxz(fd, buf, sz);
	munmap(buf, sz);
	close(fd);
	cpio_mmap cpio(tmp);
	cpio.extract();
	unlink(tmp);
	unlink(ramdisk_xz);
}

int dump_magisk(const char *path, mode_t mode) {
	int fd = xopen(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
	if (fd < 0)
		return 1;
	if (!unxz(fd, magisk_xz, sizeof(magisk_xz)))
		return 1;
	close(fd);
	return 0;
}

static int dump_manager(const char *path, mode_t mode) {
	int fd = xopen(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
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

class TestInit : public BaseInit {
public:
	TestInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
	void start() override {
		// Place init tests here
	}
};

[[maybe_unused]] static int test_main(int argc, char *argv[]) {
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
	for (auto &m : reversed(mounts))
		xumount(m.data());

	// chroot jail
	chdir(dirname(argv[0]));
	chroot(".");
	chdir("/");

	cmdline cmd{};
	load_kernel_info(&cmd);

	auto init = make_unique<TestInit>(argv, &cmd);
	init->start();

	return 1;
}

int main(int argc, char *argv[]) {
	umask(0);

	auto name = basename(argv[0]);
	if (name == "magisk"sv)
		return magisk_proxy_main(argc, argv);
	for (int i = 0; init_applet[i]; ++i) {
		if (strcmp(name, init_applet[i]) == 0)
			return (*init_applet_main[i])(argc, argv);
	}

#ifdef MAGISK_DEBUG
	if (getenv("INIT_TEST") != nullptr)
		return test_main(argc, argv);
#endif

	if (argc > 1 && argv[1] == "-x"sv) {
		if (argv[2] == "magisk"sv)
			return dump_magisk(argv[3], 0755);
		else if (argv[2] == "manager"sv)
			return dump_manager(argv[3], 0644);
	}

	if (getpid() != 1)
		return 1;
	setup_klog();

	BaseInit *init;
	cmdline cmd{};

	if (argc > 1 && argv[1] == "selinux_setup"sv) {
		init = new SecondStageInit(argv);
	} else {
		// This will also mount /sys and /proc
		load_kernel_info(&cmd);

		bool two_stage = access("/apex", F_OK) == 0;
		if (cmd.skip_initramfs) {
			if (two_stage)
				init = new SARFirstStageInit(argv, &cmd);
			else
				init = new SARInit(argv, &cmd);
		} else {
			decompress_ramdisk();
			if (cmd.force_normal_boot)
				init = new FirstStageInit(argv, &cmd);
			else if (access("/sbin/recovery", F_OK) == 0 || access("/system/bin/recovery", F_OK) == 0)
				init = new RecoveryInit(argv, &cmd);
			else if (two_stage)
				init = new FirstStageInit(argv, &cmd);
			else
				init = new RootFSInit(argv, &cmd);
		}
	}

	// Run the main routine
	init->start();
	exit(1);
}
