#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <libgen.h>
#include <vector>

#include <xz.h>
#include <magisk.hpp>
#include <cpio.hpp>
#include <utils.hpp>

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

int data_holder::patch(str_pairs list) {
	int count = 0;
	for (uint8_t *p = buf, *eof = buf + sz; p < eof; ++p) {
		for (auto [from, to] : list) {
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

bool data_holder::contains(string_view pattern) {
	for (uint8_t *p = buf, *eof = buf + sz; p < eof; ++p) {
		if (memcmp(p, pattern.data(), pattern.length() + 1) == 0)
			return true;
	}
	return false;
}

void data_holder::consume(data_holder &other) {
	buf = other.buf;
	sz = other.sz;
	other.buf = nullptr;
	other.sz = 0;
}

auto_data<HEAP> raw_data::read(int fd) {
	auto_data<HEAP> data;
	fd_full_read(fd, data.buf, data.sz);
	return data;
}

auto_data<HEAP> raw_data::read(const char *name) {
	auto_data<HEAP> data;
	full_read(name, data.buf, data.sz);
	return data;
}

auto_data<MMAP> raw_data::mmap_rw(const char *name) {
	auto_data<MMAP> data;
	::mmap_rw(name, data.buf, data.sz);
	return data;
}

auto_data<MMAP> raw_data::mmap_ro(const char *name) {
	auto_data<MMAP> data;
	::mmap_ro(name, data.buf, data.sz);
	return data;
}

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

#if 0
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

	BaseInit *init;
	cmdline cmd{};

	if (argc > 1 && argv[1] == "selinux_setup"sv) {
		setup_klog();
		init = new SecondStageInit(argv);
	} else {
		// This will also mount /sys and /proc
		load_kernel_info(&cmd);

		bool two_stage = check_two_stage();
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
