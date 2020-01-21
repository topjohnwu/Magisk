#include <sys/mount.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include <magisk.h>

struct cmdline {
	bool skip_initramfs;
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
	std::vector<std::string> mount_list;

	void exec_init(const char *init = "/init") {
		cleanup();
		execv(init, argv);
		exit(1);
	}
	virtual void cleanup();
public:
	BaseInit(char *argv[], cmdline *cmd) :
	cmd(cmd), argv(argv), mount_list{"/sys", "/proc"} {}
	virtual ~BaseInit() = default;
	virtual void start() = 0;
};

class MagiskInit : public BaseInit {
protected:
	raw_data self;
	const char *persist_dir;

	virtual void early_mount() = 0;
	bool patch_sepolicy(const char *file = "/sepolicy");
public:
	MagiskInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
};

class SARBase : public MagiskInit {
protected:
	raw_data config;
	dev_t system_dev;

	void backup_files(const char *self_path);
	void patch_rootdir();
public:
	SARBase(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {
		persist_dir = MIRRDIR "/persist/magisk";
	}
	void start() override {
		early_mount();
		patch_rootdir();
		exec_init();
	}
};

/* *************
 * 2 Stage Init
 * *************/

class ABFirstStageInit : public BaseInit {
private:
	void prepare();
public:
	ABFirstStageInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
	void start() override {
		prepare();
		exec_init("/system/bin/init");
	}
};

class AFirstStageInit : public BaseInit {
private:
	void prepare();
public:
	AFirstStageInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
	void start() override {
		prepare();
		exec_init();
	}
};

class SecondStageInit : public SARBase {
protected:
	void early_mount() override;
	void cleanup() override { /* Do not do any cleanup */ }
public:
	SecondStageInit(char *argv[]) : SARBase(argv, nullptr) {};
};

/* ***********
 * Legacy SAR
 * ***********/

class SARInit : public SARBase {
protected:
	void early_mount() override;
public:
	SARInit(char *argv[], cmdline *cmd) : SARBase(argv, cmd) {};
};

/* **********
 * Initramfs
 * **********/

class RootFSInit : public MagiskInit {
private:
	int root = -1;
	void setup_rootfs();
protected:
	void early_mount() override;
public:
	RootFSInit(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {
		persist_dir = "/dev/mnt/persist/magisk";
	}

	void start() override {
		early_mount();
		setup_rootfs();
		exec_init();
	}
};

void load_kernel_info(cmdline *cmd);
int dump_magisk(const char *path, mode_t mode);
int magisk_proxy_main(int argc, char *argv[]);
void setup_klog();
void mount_sbin();
