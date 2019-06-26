#include <stdlib.h>

struct cmdline {
	bool system_as_root;
	char slot[3];
	char dt_dir[128];
};

struct raw_data {
	void *buf;
	size_t sz;
};

class BaseInit {
protected:
	cmdline *cmd;
	char **argv;
	void re_exec_init();
	virtual void cleanup();
public:
	BaseInit(char *argv[], cmdline *cmd) : cmd(cmd), argv(argv) {}
	virtual ~BaseInit() = default;
	virtual void start() = 0;
};

class MagiskInit : public BaseInit {
protected:
	raw_data self{};
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

class SARInit : public MagiskInit {
protected:
	raw_data config{};
	dev_t system_dev;

	void early_mount() override;
	void patch_rootdir();
public:
	SARInit(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {};
	void start() override;
};

class RootFSInit : public MagiskInit {
protected:
	int root = -1;

	virtual void setup_rootfs();
public:
	RootFSInit(char *argv[], cmdline *cmd) : MagiskInit(argv, cmd) {};
	void start() override;
};

class LegacyInit : public RootFSInit {
protected:
	void early_mount() override;
public:
	LegacyInit(char *argv[], cmdline *cmd) : RootFSInit(argv, cmd) {};
};

class SARCompatInit : public RootFSInit {
protected:
	void early_mount() override;
	void setup_rootfs() override;
public:
	SARCompatInit(char *argv[], cmdline *cmd) : RootFSInit(argv, cmd) {};
};

static inline bool is_lnk(const char *name) {
	struct stat st;
	if (lstat(name, &st))
		return false;
	return S_ISLNK(st.st_mode);
}

void load_kernel_info(cmdline *cmd);
int dump_magisk(const char *path, mode_t mode);
int magisk_proxy_main(int argc, char *argv[]);
