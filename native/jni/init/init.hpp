#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include <logging.hpp>

struct cmdline {
	bool skip_initramfs;
	bool force_normal_boot;
	char slot[3];
	char dt_dir[64];
	char hardware[32];
	char hardware_plat[32];
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

struct fstab_entry {
	std::string dev;
	std::string mnt_point;
	std::string type;
	std::string mnt_flags;
	std::string fsmgr_flags;

	fstab_entry() = default;
	fstab_entry(const fstab_entry &o) = delete;
	fstab_entry(fstab_entry &&o) = default;
	void to_file(FILE *fp);
};

#define INIT_SOCKET "MAGISKINIT"
#define DEFAULT_DT_DIR "/proc/device-tree/firmware/android"

void load_kernel_info(cmdline *cmd);
int dump_magisk(const char *path, mode_t mode);
int magisk_proxy_main(int argc, char *argv[]);
void setup_klog();
void setup_tmp(const char *path, const raw_data &self, const raw_data &config);

using str_pairs = std::initializer_list<std::pair<std::string_view, std::string_view>>;
int raw_data_patch(void *addr, size_t sz, str_pairs list);

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
public:
	BaseInit(char *argv[], cmdline *cmd) :
	cmd(cmd), argv(argv), mount_list{"/sys", "/proc"} {}
	virtual ~BaseInit() = default;
	virtual void start() = 0;
	void read_dt_fstab(std::vector<fstab_entry> &fstab);
	void dt_early_mount();
};

class MagiskInit : public BaseInit {
protected:
	raw_data self;
	std::string persist_dir;

	virtual void early_mount() = 0;
	bool patch_sepolicy(const char *file);
public:
	MagiskInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {}
};

class SARBase : public MagiskInit {
protected:
	raw_data config;
	std::vector<raw_file> overlays;

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

/************
 * Initramfs
 ************/

class RootFSInit : public MagiskInit {
private:
	void setup_rootfs();
protected:
	void early_mount() override;
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
