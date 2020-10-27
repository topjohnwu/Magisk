#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include <utils.hpp>

#include "raw_data.hpp"

struct cmdline {
	bool skip_initramfs;
	bool force_normal_boot;
	char slot[3];
	char dt_dir[64];
	char fstab_suffix[32];
	char hardware[32];
	char hardware_plat[32];
};

struct fstab_entry {
	std::string dev;
	std::string mnt_point;
	std::string type;
	std::string mnt_flags;
	std::string fsmgr_flags;

	fstab_entry() = default;
	fstab_entry(const fstab_entry &) = delete;
	fstab_entry(fstab_entry &&) = default;
	void to_file(FILE *fp);
};

#define INIT_SOCKET "MAGISKINIT"
#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"

void load_kernel_info(cmdline *cmd);
bool check_two_stage();
int dump_magisk(const char *path, mode_t mode);
void setup_klog();

/***************
 * Base classes
 ***************/

class BaseInit {
protected:
	cmdline *cmd;
	char **argv;
	std::vector<std::string> mount_list;

	void exec_init() {
		cleanup();
		execv("/init", argv);
		exit(1);
	}
	virtual void cleanup();
	void read_dt_fstab(std::vector<fstab_entry> &fstab);
public:
	BaseInit(char *argv[], cmdline *cmd) :
	cmd(cmd), argv(argv), mount_list{"/sys", "/proc"} {}
	virtual ~BaseInit() = default;
	virtual void start() = 0;
};

class MagiskInit : public BaseInit {
protected:
	auto_data<HEAP> self;
	auto_data<HEAP> config;
	std::string persist_dir;

	void mount_with_dt();
	bool patch_sepolicy(const char *file);
	void setup_tmp(const char *path);
public:
	MagiskInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {}
};

class SARBase : public MagiskInit {
protected:
	std::vector<raw_file> overlays;

	virtual void early_mount() = 0;
	void backup_files();
	void patch_rootdir();
	void mount_system_root();
public:
	SARBase(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {}
	void start() override {
		early_mount();
		patch_rootdir();
		exec_init();
	}
};

/***************
 * 2 Stage Init
 ***************/

class FirstStageInit : public BaseInit {
private:
	void prepare();
public:
	FirstStageInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {
		LOGD("%s\n", __FUNCTION__);
	};
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
	SecondStageInit(char *argv[]) : SARBase(argv, nullptr) {
		LOGD("%s\n", __FUNCTION__);
	};
};

/*************
 * Legacy SAR
 *************/

class SARInit : public SARBase {
protected:
	void early_mount() override;
public:
	SARInit(char *argv[], cmdline *cmd) : SARBase(argv, cmd) {
		LOGD("%s\n", __FUNCTION__);
	};
};

// Special case for legacy SAR on Android 10+
// Should be followed by normal 2SI SecondStageInit
class SARFirstStageInit : public SARBase {
private:
	void prepare();
protected:
	void early_mount() override;
public:
	SARFirstStageInit(char *argv[], cmdline *cmd) : SARBase(argv, cmd) {
		LOGD("%s\n", __FUNCTION__);
	};
	void start() override {
		early_mount();
		prepare();
		exec_init();
	}
};

/************
 * Initramfs
 ************/

class RootFSInit : public MagiskInit {
private:
	void setup_rootfs();
	void early_mount();
public:
	RootFSInit(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {
		LOGD("%s\n", __FUNCTION__);
	}

	void start() override {
		early_mount();
		setup_rootfs();
		exec_init();
	}
};

class MagiskProxy : public MagiskInit {
public:
	explicit MagiskProxy(char *argv[]) : MagiskInit(argv, nullptr) {
		LOGD("%s\n", __FUNCTION__);
	}
	void start() override;
};
