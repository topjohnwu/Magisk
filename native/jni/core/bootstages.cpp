#include <sys/mount.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <string>

#include <magisk.h>
#include <db.h>
#include <utils.h>
#include <daemon.h>
#include <resetprop.h>
#include <selinux.h>
#include <flags.h>

using namespace std;

static char buf[PATH_MAX], buf2[PATH_MAX];
static vector<string> module_list;
static bool no_secure_dir = false;
static bool pfs_done = false;

static int bind_mount(const char *from, const char *to, bool log = true);
extern void auto_start_magiskhide();

/***************
 * Magic Mount *
 ***************/

#ifdef MAGISK_DEBUG
#define VLOGI(tag, from, to) LOGI("%s: %s <- %s\n", tag, to, from)
#else
#define VLOGI(tag, from, to) LOGI("%s: %s\n", tag, to)
#endif

// Precedence: MODULE > SKEL > INTER > DUMMY
#define IS_DUMMY   0x01    /* mount from mirror */
#define IS_INTER   0x02    /* intermediate node */
#define IS_SKEL    0x04    /* replace with tmpfs */
#define IS_MODULE  0x08    /* mount from module */

#define IS_DIR(n)  ((n)->type == DT_DIR)
#define IS_LNK(n)  ((n)->type == DT_LNK)
#define IS_REG(n)  ((n)->type == DT_REG)

class node_entry {
public:
	explicit node_entry(const char *name, uint8_t type = DT_DIR, uint8_t status = IS_INTER)
			: module(nullptr), name(name), type(type), status(status), parent(nullptr) {}
	~node_entry();
	void create_module_tree(const char *module);
	void magic_mount();
	node_entry *extract(const char *name);

	static bool vendor_root;
	static bool product_root;

private:
	const char *module;    /* Only used when IS_MODULE */
	const string name;
	uint8_t type;
	uint8_t status;
	node_entry *parent;
	vector<node_entry *> children;

	node_entry(node_entry *parent, const char *module, const char *name, uint8_t type)
			: module(module), name(name), type(type), status(0), parent(parent) {}
	bool is_special();
	bool is_root();
	string get_path();
	void insert(node_entry *&);
	void clone_skeleton();
	int get_path(char *path);
};

bool node_entry::vendor_root = false;
bool node_entry::product_root = false;

node_entry::~node_entry() {
	for (auto &node : children)
		delete node;
}

#define SPECIAL_NODE (parent->parent ? false : \
((vendor_root && name == "vendor") || (product_root && name == "product")))

bool node_entry::is_special() {
	return parent ? SPECIAL_NODE : false;
}

bool node_entry::is_root() {
	return parent ? SPECIAL_NODE : true;
}

string node_entry::get_path() {
	get_path(buf);
	return buf;
}

int node_entry::get_path(char *path) {
	int len = 0;
	if (parent)
		len = parent->get_path(path);
	len += sprintf(path + len, "/%s", name.c_str());
	return len;
}

void node_entry::insert(node_entry *&node) {
	node->parent = this;
	for (auto &child : children) {
		if (child->name == node->name) {
			if (node->status > child->status) {
				// The new node has higher precedence
				delete child;
				child = node;
			} else {
				delete node;
				node = child;
			}
			return;
		}
	}
	children.push_back(node);
}

void node_entry::create_module_tree(const char *module) {
	auto full_path = get_path();
	snprintf(buf, PATH_MAX, "%s/%s%s", MODULEROOT, module, full_path.c_str());

	auto dir = xopen_dir(buf);
	if (!dir)
		return;

	// Check directory replace
	if (faccessat(dirfd(dir.get()), ".replace", F_OK, 0) == 0) {
		if (is_root()) {
			// Root nodes should not be replaced
			rm_rf(buf);
		} else if (status < IS_MODULE) {
			// Upgrade current node to current module
			this->module = module;
			status = IS_MODULE;
		}
		return;
	}

	for (dirent *entry; (entry = xreaddir(dir.get()));) {
		if (entry->d_name == "."sv || entry->d_name == ".."sv)
			continue;
		// Create new node
		auto node = new node_entry(this, module, entry->d_name, entry->d_type);

		// buf = real path, buf2 = module path
		snprintf(buf, PATH_MAX, "%s/%s", full_path.c_str(), entry->d_name);
		snprintf(buf2, PATH_MAX, MODULEROOT "/%s%s/%s", module, full_path.c_str(), entry->d_name);

		/*
		 * Clone current directory in one of the following conditions:
		 * - Target does not exist
		 * - Module file is a symlink
		 * - Target file is a symlink (exclude special nodes)
		 */
		bool clone = false;
		if (IS_LNK(node) || access(buf, F_OK) != 0) {
			clone = true;
		} else if (!node->is_special()) {
			struct stat s;
			xlstat(buf, &s);
			if (S_ISLNK(s.st_mode))
				clone = true;
		}
		if (clone && is_root()) {
			// Root nodes should not be cloned
			rm_rf(buf2);
			delete node;
			continue;
		}

		if (clone) {
			// Mark self as a skeleton
			status |= IS_SKEL;  /* This will not overwrite if parent is module */
			node->status = IS_MODULE;
		} else if (node->is_special()) {
			// Special nodes will be pulled out as root nodes later
			node->status = IS_INTER;
		} else {
			// Clone attributes from real path
			clone_attr(buf, buf2);
			if (IS_DIR(node)) {
				// First mark as an intermediate node
				node->status = IS_INTER;
			} else if (IS_REG(node)) {
				// This is a file, mark as leaf
				node->status = IS_MODULE;
			}
		}
		insert(node);
		if (IS_DIR(node)) {
			// Recursive traverse through everything
			node->create_module_tree(module);
		}
	}
}

void node_entry::clone_skeleton() {
	// Clone the structure
	auto full_path = get_path();
	snprintf(buf, PATH_MAX, "%s%s", MIRRDIR, full_path.data());
	if (auto dir = xopen_dir(buf); dir) {
		for (dirent *entry; (entry = xreaddir(dir.get()));) {
			if (entry->d_name == "."sv || entry->d_name == ".."sv)
				continue;
			// Create dummy node
			auto dummy = new node_entry(entry->d_name, entry->d_type, IS_DUMMY);
			insert(dummy);
		}
	} else { return; }

	if (status & IS_SKEL) {
		file_attr attr;
		getattr(full_path.c_str(), &attr);
		LOGI("mnt_tmpfs : %s\n", full_path.c_str());
		xmount("tmpfs", full_path.c_str(), "tmpfs", 0, nullptr);
		setattr(full_path.c_str(), &attr);
	}

	for (auto &child : children) {
		snprintf(buf, PATH_MAX, "%s/%s", full_path.c_str(), child->name.c_str());

		// Create the dummy file/directory
		if (IS_DIR(child))
			xmkdir(buf, 0755);
		else if (IS_REG(child))
			close(creat(buf, 0644));
		// Links will be handled later

		if (child->status & IS_MODULE) {
			// Mount from module file to dummy file
			snprintf(buf2, PATH_MAX, "%s/%s%s/%s", MODULEMNT,
					 child->module, full_path.c_str(), child->name.c_str());
		} else if (child->status & (IS_SKEL | IS_INTER)) {
			// It's an intermediate folder, recursive clone
			child->clone_skeleton();
			continue;
		} else if (child->status & IS_DUMMY) {
			// Mount from mirror to dummy file
			snprintf(buf2, PATH_MAX, "%s%s/%s", MIRRDIR, full_path.c_str(), child->name.c_str());
		}

		if (IS_LNK(child)) {
			// Copy symlinks directly
			cp_afc(buf2, buf);
			VLOGI("copy_link ", buf2, buf);
		} else {
			snprintf(buf, PATH_MAX, "%s/%s", full_path.c_str(), child->name.c_str());
			bind_mount(buf2, buf);
		}
	}
}

void node_entry::magic_mount() {
	if (status & IS_MODULE) {
		// Mount module item
		auto real_path = get_path();
		snprintf(buf, PATH_MAX, "%s/%s%s", MODULEMNT, module, real_path.c_str());
		bind_mount(buf, real_path.c_str());
	} else if (status & IS_SKEL) {
		// The node is labeled to be cloned with skeleton, lets do it
		clone_skeleton();
	} else if (status & IS_INTER) {
		// It's an intermediate node, travel deeper
		for (auto &child : children)
			child->magic_mount();
	}
}

node_entry *node_entry::extract(const char *name) {
	node_entry *node = nullptr;
	// Extract the node out of the tree
	for (auto it = children.begin(); it != children.end(); ++it) {
		if ((*it)->name == name) {
			node = *it;
			node->parent = nullptr;
			children.erase(it);
			break;
		}
	}
	return node;
}

/*****************
 * Miscellaneous *
 *****************/

static int bind_mount(const char *from, const char *to, bool log) {
	int ret = xmount(from, to, nullptr, MS_BIND, nullptr);
	if (log) VLOGI("bind_mount", from, to);
	return ret;
}

#define MIRRMNT(part)   MIRRDIR "/" #part
#define PARTBLK(part)   BLOCKDIR "/" #part
#define DIR_IS(part)    (me->mnt_dir == "/" #part ""sv)

#define mount_mirror(part, flag) { \
	xstat(me->mnt_fsname, &st); \
	mknod(PARTBLK(part), (st.st_mode & S_IFMT) | 0600, st.st_rdev); \
	xmkdir(MIRRMNT(part), 0755); \
	xmount(PARTBLK(part), MIRRMNT(part), me->mnt_type, flag, nullptr); \
	VLOGI("mount", PARTBLK(part), MIRRMNT(part)); \
}

static bool magisk_env() {
	LOGI("* Initializing Magisk environment\n");

	string pkg;
	check_manager(&pkg);

	char install_dir[128];
	sprintf(install_dir, "%s/0/%s/install", APP_DATA_DIR, pkg.data());

	// Alternative binaries paths
	const char *alt_bin[] = { "/cache/data_adb/magisk", "/data/magisk", install_dir };
	for (auto alt : alt_bin) {
		struct stat st;
		if (lstat(alt, &st) != -1) {
			if (S_ISLNK(st.st_mode)) {
				unlink(alt);
				continue;
			}
			rm_rf(DATABIN);;
			cp_afc(alt, DATABIN);
			rm_rf(alt);
			break;
		}
	}

	// Remove stuffs
	rm_rf("/cache/data_adb");
	rm_rf("/data/adb/modules/.core");
	unlink("/data/magisk.img");
	unlink("/data/magisk_debug.log");

	// Directories in tmpfs overlay
	xmkdir(MIRRDIR, 0);
	xmkdir(BLOCKDIR, 0);
	xmkdir(BBPATH, 0755);
	xmkdir(MODULEMNT, 0755);

	// Directories in /data/adb
	xmkdir(DATABIN, 0755);
	xmkdir(MODULEROOT, 0755);
	xmkdir(SECURE_DIR "/post-fs-data.d", 0755);
	xmkdir(SECURE_DIR "/service.d", 0755);

	LOGI("* Mounting mirrors");
	bool system_as_root = false;
	struct stat st;
	parse_mnt("/proc/mounts", [&](mntent *me) {
		if (DIR_IS(system_root)) {
			mount_mirror(system_root, MS_RDONLY);
			xsymlink("./system_root/system", MIRRMNT(system));
			VLOGI("link", MIRRMNT(system_root) "/system", MIRRMNT(system));
			system_as_root = true;
		} else if (!system_as_root && DIR_IS(system)) {
			mount_mirror(system, MS_RDONLY);
		} else if (DIR_IS(vendor)) {
			mount_mirror(vendor, MS_RDONLY);
		} else if (DIR_IS(product)) {
			mount_mirror(product, MS_RDONLY);
		} else if (DIR_IS(data) && me->mnt_type != "tmpfs"sv) {
			mount_mirror(data, 0);
		} else if (SDK_INT >= 24 && DIR_IS(proc) && !strstr(me->mnt_opts, "hidepid=2")) {
			xmount(nullptr, "/proc", nullptr, MS_REMOUNT, "hidepid=2,gid=3009");
		}
		return true;
	});
	if (access(MIRRMNT(system), F_OK) != 0 && access(MIRRMNT(system_root), F_OK) == 0) {
		// Pre-init mirrors
		xsymlink("./system_root/system", MIRRMNT(system));
		VLOGI("link", MIRRMNT(system_root) "/system", MIRRMNT(system));
	}
	if (access(MIRRMNT(vendor), F_OK) != 0) {
		xsymlink("./system/vendor", MIRRMNT(vendor));
		VLOGI("link", MIRRMNT(system) "/vendor", MIRRMNT(vendor));
	}
	if (access("/system/product", F_OK) == 0 && access(MIRRMNT(product), F_OK) != 0) {
		xsymlink("./system/product", MIRRMNT(product));
		VLOGI("link", MIRRMNT(system) "/product", MIRRMNT(product));
	}

	// Disable/remove magiskhide, resetprop
	if (SDK_INT < 19) {
		unlink("/sbin/resetprop");
		unlink("/sbin/magiskhide");
	}

	if (access(DATABIN "/busybox", X_OK) == -1)
		return false;
	LOGI("* Setting up internal busybox");
	cp_afc(DATABIN "/busybox", BBPATH "/busybox");
	exec_command_sync(BBPATH "/busybox", "--install", "-s", BBPATH);

	return true;
}

static void prepare_modules() {
	// Upgrade modules
	if (auto dir = open_dir(MODULEUPGRADE); dir) {
		for (dirent *entry; (entry = xreaddir(dir.get()));) {
			if (entry->d_type == DT_DIR) {
				if (entry->d_name == "."sv || entry->d_name == ".."sv)
					continue;
				// Cleanup old module if exists
				snprintf(buf, sizeof(buf), "%s/%s", MODULEROOT, entry->d_name);
				if (access(buf, F_OK) == 0)
					rm_rf(buf);
				LOGI("Upgrade / New module: %s\n", entry->d_name);
				snprintf(buf2, sizeof(buf2), "%s/%s", MODULEUPGRADE, entry->d_name);
				rename(buf2, buf);
			}
		}
		rm_rf(MODULEUPGRADE);
	}
	bind_mount(MIRRDIR MODULEROOT, MODULEMNT, false);

	restorecon();
	chmod(SECURE_DIR, 0700);
}

static void reboot() {
	if (RECOVERY_MODE)
		exec_command_sync("/system/bin/reboot", "recovery");
	else
		exec_command_sync("/system/bin/reboot");
}

void remove_modules() {
	LOGI("* Remove all modules and reboot");
	auto dir = xopen_dir(MODULEROOT);
	int dfd = dirfd(dir.get());
	for (dirent *entry; (entry = xreaddir(dir.get()));) {
		if (entry->d_type == DT_DIR) {
			if (entry->d_name == "."sv || entry->d_name == ".."sv || entry->d_name == ".core"sv)
				continue;

			int modfd = xopenat(dfd, entry->d_name, O_RDONLY | O_CLOEXEC);
			close(xopenat(modfd, "remove", O_RDONLY | O_CREAT | O_CLOEXEC));
			close(modfd);
		}
	}
	reboot();
}

static void collect_modules() {
	auto dir = xopen_dir(MODULEROOT);
	int dfd = dirfd(dir.get());
	for (dirent *entry; (entry = xreaddir(dir.get()));) {
		if (entry->d_type == DT_DIR) {
			if (entry->d_name == "."sv || entry->d_name == ".."sv || entry->d_name == ".core"sv)
				continue;

			int modfd = xopenat(dfd, entry->d_name, O_RDONLY);
			run_finally f([=]{ close(modfd); });

			if (faccessat(modfd, "remove", F_OK, 0) == 0) {
				LOGI("%s: remove\n", entry->d_name);
				fd_pathat(modfd, "uninstall.sh", buf, sizeof(buf));
				if (access(buf, F_OK) == 0)
					exec_script(buf);
				frm_rf(modfd);
				unlinkat(dfd, entry->d_name, AT_REMOVEDIR);
				continue;
			}

			unlinkat(modfd, "update", 0);

			if (faccessat(modfd, "disable", F_OK, 0) != 0)
				module_list.emplace_back(entry->d_name);
		}
	}
}

static bool load_modules(node_entry *root) {
	LOGI("* Loading modules\n");

	bool has_modules = false;
	for (const auto &m : module_list) {
		const auto module = m.data();
		char *name = buf + snprintf(buf, sizeof(buf), MODULEROOT "/%s/", module);

		// Read props
		strcpy(name, "system.prop");
		if (access(buf, F_OK) == 0) {
			LOGI("%s: loading [system.prop]\n", module);
			load_prop_file(buf, false);
		}
		// Copy sepolicy rules
		strcpy(name, "sepolicy.rule");
		if (access(MIRRDIR "/persist", F_OK) == 0 && access(buf, F_OK) == 0) {
			char *p = buf2 + snprintf(buf2, sizeof(buf2), MIRRDIR "/persist/magisk/%s", module);
			xmkdirs(buf2, 0755);
			strcpy(p, "/sepolicy.rule");
			cp_afc(buf, buf2);
		}

		// Check whether skip mounting
		strcpy(name, "skip_mount");
		if (access(buf, F_OK) == 0)
			continue;
		// Double check whether the system folder exists
		strcpy(name, "system");
		if (access(buf, F_OK) != 0)
			continue;

		// Construct structure
		has_modules = true;
		LOGI("%s: constructing magic mount structure\n", module);
		// If /system/vendor exists in module, create a link outside
		strcpy(name, "system/vendor");
		if (node_entry::vendor_root && access(buf, F_OK) == 0) {
			snprintf(buf2, sizeof(buf2), "%s/%s/vendor", MODULEROOT, module);
			unlink(buf2);
			xsymlink("./system/vendor", buf2);
		}
		// If /system/product exists in module, create a link outside
		strcpy(name, "system/product");
		if (node_entry::product_root && access(buf, F_OK) == 0) {
			snprintf(buf2, sizeof(buf2), "%s/%s/product", MODULEROOT, module);
			unlink(buf2);
			xsymlink("./system/product", buf2);
		}
		root->create_module_tree(module);
	}
	return has_modules;
}

static bool check_data() {
	bool mnt = false;
	bool data = false;
	file_readline("/proc/mounts", [&](string_view s) -> bool {
		if (str_contains(s, " /data ") && !str_contains(s, "tmpfs"))
			mnt = true;
		return true;
	});
	if (mnt) {
		auto crypto = getprop("ro.crypto.state");
		if (!crypto.empty()) {
			if (crypto == "unencrypted") {
				// Unencrypted, we can directly access data
				data = true;
			} else {
				// Encrypted, check whether vold is started
				data = !getprop("init.svc.vold").empty();
			}
		} else {
			// ro.crypto.state is not set, assume it's unencrypted
			data = true;
		}
	}
	return data;
}

void unlock_blocks() {
	int fd, dev, OFF = 0;

	auto dir = xopen_dir("/dev/block");
	if (!dir)
		return;
	dev = dirfd(dir.get());

	for (dirent *entry; (entry = readdir(dir.get()));) {
		if (entry->d_type == DT_BLK) {
			if ((fd = openat(dev, entry->d_name, O_RDONLY | O_CLOEXEC)) < 0)
				continue;
			if (ioctl(fd, BLKROSET, &OFF) < 0)
				PLOGE("unlock %s", entry->d_name);
			close(fd);
		}
	}
}

static bool log_dump = false;
static void dump_logs() {
	if (log_dump)
		return;
	int test = exec_command_sync("/system/bin/logcat", "-d", "-f", "/dev/null");
	chmod("/dev/null", 0666);
	if (test != 0)
		return;
	rename(LOGFILE, LOGFILE ".bak");
	log_dump = true;
	// Start a daemon thread and wait indefinitely
	new_daemon_thread([]() -> void {
		int fd = xopen(LOGFILE, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644);
		exec_t exec {
			.fd = fd,
			.fork = fork_no_zombie
		};
		int pid = exec_command(exec, "/system/bin/logcat", "-s", "Magisk");
		close(fd);
		if (pid < 0) {
			log_dump = false;
		} else {
			waitpid(pid, nullptr, 0);
		}
	});
}

/****************
 * Entry points *
 ****************/

[[noreturn]] static void unblock_boot_process() {
	close(xopen(UNBLOCKFILE, O_RDONLY | O_CREAT, 0));
	pthread_exit(nullptr);
}

[[noreturn]] static void core_only() {
	pfs_done = true;
	auto_start_magiskhide();
	unblock_boot_process();
}

void post_fs_data(int client) {
	// ack
	write_int(client, 0);
	close(client);

	if (getenv("REMOUNT_ROOT"))
		xmount(nullptr, "/", nullptr, MS_REMOUNT | MS_RDONLY, nullptr);

	if (!check_data())
		unblock_boot_process();

	dump_logs();

	LOGI("** post-fs-data mode running\n");

	// Unlock all blocks for rw
	unlock_blocks();

	if (access(SECURE_DIR, F_OK) != 0) {
		/* If the folder is not automatically created by the system,
		 * do NOT proceed further. Manual creation of the folder
		 * will cause bootloops on FBE devices. */
		LOGE(SECURE_DIR " is not present, abort...");
		no_secure_dir = true;
		unblock_boot_process();
	}

	if (!magisk_env()) {
		LOGE("* Magisk environment setup incomplete, abort\n");
		unblock_boot_process();
	}

	LOGI("* Running post-fs-data.d scripts\n");
	exec_common_script("post-fs-data");

	prepare_modules();

	// Core only mode
	if (access(DISABLEFILE, F_OK) == 0)
		core_only();

	collect_modules();

	// Execute module scripts
	LOGI("* Running module post-fs-data scripts\n");
	exec_module_script("post-fs-data", module_list);

	// Recollect modules
	module_list.clear();
	collect_modules();

	node_entry::vendor_root = access("/vendor", F_OK) == 0;
	node_entry::product_root = access("/product", F_OK) == 0;

	// Create the system root entry
	auto sys_root = new node_entry("system");

	if (load_modules(sys_root)) {
		// Pull out special nodes if exist
		node_entry *special;
		if (node_entry::vendor_root && (special = sys_root->extract("vendor"))) {
			special->magic_mount();
			delete special;
		}
		if (node_entry::product_root && (special = sys_root->extract("product"))) {
			special->magic_mount();
			delete special;
		}

		sys_root->magic_mount();
	}

	// Cleanup memory
	delete sys_root;

	core_only();
}

void late_start(int client) {
	LOGI("** late_start service mode running\n");
	// ack
	write_int(client, 0);
	close(client);

	dump_logs();

	if (no_secure_dir) {
		// It's safe to create the folder at this point if the system didn't create it
		if (access(SECURE_DIR, F_OK) != 0)
			xmkdir(SECURE_DIR, 0700);
		// And reboot to make proper setup possible
		reboot();
	}

	if (!pfs_done)
		return;

	if (access(BBPATH, F_OK) != 0){
		LOGE("* post-fs-data mode is not triggered\n");
		unlock_blocks();
		magisk_env();
		prepare_modules();
		close(xopen(DISABLEFILE, O_RDONLY | O_CREAT | O_CLOEXEC, 0));
	}

	auto_start_magiskhide();

	LOGI("* Running service.d scripts\n");
	exec_common_script("service");

	// Core only mode
	if (access(DISABLEFILE, F_OK) != 0) {
		LOGI("* Running module service scripts\n");
		exec_module_script("service", module_list);
	}

	// All boot stage done, cleanup
	module_list.clear();
	module_list.shrink_to_fit();
}

void boot_complete(int client) {
	LOGI("** boot_complete triggered\n");
	// ack
	write_int(client, 0);
	close(client);

	if (!pfs_done)
		return;

	auto_start_magiskhide();

	if (access(MANAGERAPK, F_OK) == 0) {
		// Install Magisk Manager if exists
		rename(MANAGERAPK, "/data/magisk.apk");
		install_apk("/data/magisk.apk");
	} else {
		// Check whether we have manager installed
		if (!check_manager()) {
			// Install stub
			exec_command_sync("/sbin/magiskinit", "-x", "manager", "/data/magisk.apk");
			install_apk("/data/magisk.apk");
		}
	}
}
