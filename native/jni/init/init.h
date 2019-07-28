#include <sys/mount.h>
#include <unistd.h>
#include <stdlib.h>

struct cmdline {
	bool system_as_root;
	bool force_normal_boot;
	char slot[3];
	char dt_dir[128];
};

struct raw_data {
	uint8_t *buf = nullptr;
	size_t sz = 0;

	raw_data() = default;
	raw_data(const raw_data&) = delete;
	raw_data(raw_data &&d) {
		buf = d.buf;
		sz = d.sz;
		d.buf = nullptr;
		d.sz = 0;
	}
	~raw_data() {
		free(buf);
	}
};

/* *************
 * Base classes
 * *************/

class BaseInit {
protected:
	cmdline *cmd;
	char **argv;

	void exec_init(const char *init = "/init") {
		cleanup();
		execv(init, argv);
		exit(1);
	}
	virtual void cleanup() {
		umount("/sys");
		umount("/proc");
		umount("/dev");
	}
public:
	BaseInit(char *argv[], cmdline *cmd) : cmd(cmd), argv(argv) {}
	virtual ~BaseInit() = default;
	virtual void start() = 0;
};

class MagiskInit : public BaseInit {
protected:
	raw_data self;
	bool mnt_system = false;
	bool mnt_vendor = false;
	bool mnt_product = false;
	bool mnt_odm = false;

	virtual void early_mount() = 0;
	bool read_dt_fstab(const char *name, char *partname, char *fstype);
	bool patch_sepolicy(const char *file = "/sepolicy");
	void cleanup() override;
public:
	MagiskInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
};

class RootFSInit : public MagiskInit {
protected:
	int root = -1;

	virtual void setup_rootfs();
public:
	RootFSInit(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {};
	void start() override {
		early_mount();
		setup_rootfs();
		exec_init();
	}
};

class SARCommon : public MagiskInit {
protected:
	raw_data config;
	dev_t system_dev;

	void backup_files();
	void patch_rootdir();
public:
	SARCommon(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {};
	void start() override {
		early_mount();
		patch_rootdir();
		exec_init();
	}
};

/* *******************
 * Logical Partitions
 * *******************/

class FirstStageInit : public BaseInit {
protected:
	void prepare();
public:
	FirstStageInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
	void start() override {
		prepare();
		exec_init("/system/bin/init");
	}
};

class SecondStageInit : public SARCommon {
protected:
	void early_mount() override;
	void cleanup() override { /* Do not do any cleanup */ }
public:
	SecondStageInit(char *argv[]) : SARCommon(argv, nullptr) {};
};

/* ***********
 * Normal SAR
 * ***********/

class SARInit : public SARCommon {
protected:
	void early_mount() override;
public:
	SARInit(char *argv[], cmdline *cmd) : SARCommon(argv, cmd) {};
};

/* **********
 * Initramfs
 * **********/

class LegacyInit : public RootFSInit {
protected:
	void early_mount() override;
public:
	LegacyInit(char *argv[], cmdline *cmd) : RootFSInit(argv, cmd) {};
};

/* ****************
 * Compat-mode SAR
 * ****************/

class SARCompatInit : public RootFSInit {
protected:
	void early_mount() override;
	void setup_rootfs() override;
public:
	SARCompatInit(char *argv[], cmdline *cmd) : RootFSInit(argv, cmd) {};
};

void load_kernel_info(cmdline *cmd);
int dump_magisk(const char *path, mode_t mode);
int magisk_proxy_main(int argc, char *argv[]);
