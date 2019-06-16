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
	raw_data self{};
	raw_data config{};
	char **argv;
	int root = -1;
	bool load_sepol = false;
	bool mnt_system = false;
	bool mnt_vendor = false;
	bool mnt_product = false;
	bool mnt_odm = false;

	virtual void preset() {};
	virtual void early_mount() {}
	void setup_rootfs();
	bool read_dt_fstab(const char *name, char *partname, char *fstype);
	bool patch_sepolicy();
	void cleanup();
	void re_exec_init();

public:
	BaseInit(char *argv[], cmdline *cmd) : cmd(cmd), argv(argv) {}
	virtual ~BaseInit() = default;
	virtual void start();
};

class LegacyInit : public BaseInit {
protected:
	void preset() override;
	void early_mount() override;
public:
	LegacyInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
};

class SARInit : public BaseInit {
protected:
	void preset() override;
	void early_mount() override;
public:
	SARInit(char *argv[], cmdline *cmd) : BaseInit(argv, cmd) {};
};

static inline bool is_lnk(const char *name) {
	struct stat st;
	if (lstat(name, &st))
		return false;
	return S_ISLNK(st.st_mode);
}

void load_kernel_info(cmdline *cmd);
int dump_magisk(const char *path, mode_t mode);
