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

class MagiskInit {
private:
	cmdline cmd{};
	raw_data self{};
	raw_data config{};
	int root = -1;
	char **argv;
	bool load_sepol = false;
	bool mnt_system = false;
	bool mnt_vendor = false;
	bool mnt_product = false;
	bool mnt_odm = false;

	void load_kernel_info();
	void preset();
	void early_mount();
	void setup_rootfs();
	bool read_dt_fstab(const char *name, char *partname, char *fstype);
	bool patch_sepolicy();
	void cleanup();
	void re_exec_init();

public:
	explicit MagiskInit(char *argv[]) : argv(argv) {}
	void start();
	void test();
};

static inline bool is_lnk(const char *name) {
	struct stat st;
	if (lstat(name, &st))
		return false;
	return S_ISLNK(st.st_mode);
}

int dump_magisk(const char *path, mode_t mode);
