/* bootstages.c - Core bootstage operations
 *
 * All bootstage operations, including simple mount in post-fs,
 * magisk mount in post-fs-data, various image handling, script
 * execution, load modules, install Magisk Manager etc.
 */

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
static bool seperate_vendor;

char *system_block, *vendor_block, *data_block;

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
	explicit node_entry(const char *name, uint8_t status = 0, uint8_t type = 0)
			: name(name), type(type), status(status), parent(nullptr) {}
	~node_entry();
	void create_module_tree(const char *module);
	void magic_mount();
	node_entry *extract(const char *name);

private:
	const char *module;    /* Only used when status & IS_MODULE */
	const string name;
	uint8_t type;
	uint8_t status;
	node_entry *parent;
	vector<node_entry *> children;

	node_entry(node_entry *parent, const char *module, const char *name, uint8_t type)
			: node_entry(name, 0, type) {
		this->parent = parent;
		this->module = module;
	}
	bool is_vendor();
	string get_path();
	void insert(node_entry *&);
	void clone_skeleton();
	int get_path(char *path);
};

node_entry::~node_entry() {
	for (auto &node : children)
		delete node;
}

bool node_entry::is_vendor() {
	return parent ? (parent->parent ? false : name == "vendor") : false;
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
	DIR *dir;
	struct dirent *entry;

	auto full_path = get_path();
	snprintf(buf, PATH_MAX, "%s/%s%s", MODULEROOT, module, full_path.c_str());
	if (!(dir = xopendir(buf)))
		return;

	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		// Create new node
		auto node = new node_entry(this, module, entry->d_name, entry->d_type);

		// buf = real path, buf2 = module path
		snprintf(buf, PATH_MAX, "%s/%s", full_path.c_str(), entry->d_name);
		int eo2 = snprintf(buf2, PATH_MAX, MODULEROOT "/%s%s/%s",
				module, full_path.c_str(), entry->d_name);

		/*
		 * Clone the parent in the following condition:
		 * 1. File in module is a symlink
		 * 2. Target file do not exist
		 * 3. Target file is a symlink (exclude /system/vendor)
		 */
		bool clone = false;
		if (IS_LNK(node) || access(buf, F_OK) == -1) {
			clone = true;
		} else if (!node->is_vendor()) {
			struct stat s;
			xstat(buf, &s);
			if (S_ISLNK(s.st_mode))
				clone = true;
		}

		if (clone) {
			// Mark self as a skeleton
			status |= IS_SKEL;  /* This will not overwrite if parent is module */
			node->status = IS_MODULE;
		} else {
			// Clone attributes from real path
			clone_attr(buf, buf2);
			if (IS_DIR(node)) {
				// Check if marked as replace
				strcpy(buf2 + eo2, "/.replace");
				if (access(buf2, F_OK) == 0) {
					// Replace everything, mark as leaf
					node->status = IS_MODULE;
				} else {
					// This will be an intermediate node
					node->status = IS_INTER;
				}
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
	closedir(dir);
}

void node_entry::clone_skeleton() {
	DIR *dir;
	struct dirent *entry;
	struct node_entry *dummy;

	// Clone the structure
	auto full_path = get_path();
	snprintf(buf, PATH_MAX, "%s%s", MIRRDIR, full_path.c_str());
	if (!(dir = xopendir(buf)))
		return;
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		// Create dummy node
		dummy = new node_entry(entry->d_name, IS_DUMMY, entry->d_type);
		insert(dummy);
	}
	closedir(dir);

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

		if (child->is_vendor()) {
			if (seperate_vendor) {
				cp_afc(MIRRDIR "/system/vendor", "/system/vendor");
				VLOGI("copy_link ", "/system/vendor", MIRRDIR "/system/vendor");
			}
			// Skip
			continue;
		} else if (child->status & IS_MODULE) {
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
	// The only thing goes here should be placeholder nodes
	// There should be no dummies, so don't need to handle it here
}

node_entry *node_entry::extract(const char *name) {
	node_entry *node = nullptr;
	// Extract the node out and swap with placeholder
	for (auto &child : children) {
		if (child->name == name) {
			node = child;
			child = new node_entry(name);
			child->parent = node->parent;
			node->parent = nullptr;
			break;
		}
	}
	return node;
}

/****************
 * Simple Mount *
 ****************/

static void simple_mount(const char *path) {
	DIR *dir;
	struct dirent *entry;

	snprintf(buf, PATH_MAX, "%s%s", SIMPLEMOUNT, path);
	if (!(dir = opendir(buf)))
		return;

	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		// Target file path
		snprintf(buf2, PATH_MAX, "%s/%s", path, entry->d_name);
		// Only mount existing file
		if (access(buf2, F_OK) == -1)
			continue;
		if (entry->d_type == DT_DIR) {
			simple_mount(string(buf2).c_str());
		} else if (entry->d_type == DT_REG) {
			// Actual file path
			snprintf(buf, PATH_MAX, "%s%s", SIMPLEMOUNT, buf2);
			// Clone all attributes
			clone_attr(buf2, buf);
			// Finally, mount the file
			bind_mount(buf, buf2);
		}
	}

	closedir(dir);
}

/*****************
 * Miscellaneous *
 *****************/

static int bind_mount(const char *from, const char *to, bool log) {
	int ret = xmount(from, to, nullptr, MS_BIND, nullptr);
	if (log) VLOGI("bind_mount", from, to);
	return ret;
}

static bool magisk_env() {
	LOGI("* Initializing Magisk environment\n");

	// Alternative binaries paths
	const char *alt_bin[] = { "/cache/data_adb/magisk", "/data/magisk",
							  "/data/data/com.topjohnwu.magisk/install",
							  "/data/user_de/0/com.topjohnwu.magisk/install" };
	for (auto &alt : alt_bin) {
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
	unlink("/data/magisk.img");
	unlink("/data/magisk_debug.log");

	// Legacy support
	symlink(MAGISKTMP, "/sbin/.core");
	symlink(MODULEMNT, MAGISKTMP "/img");

	// Create directories in tmpfs overlay
	xmkdirs(MIRRDIR "/system", 0755);
	xmkdir(MIRRDIR "/data", 0755);
	xmkdir(BBPATH, 0755);
	xmkdir(MODULEMNT, 0755);

	// /data/adb directories
	xmkdir(MODULEROOT, 0755);
	xmkdir(SECURE_DIR "/post-fs-data.d", 0755);
	xmkdir(SECURE_DIR "/service.d", 0755);

	LOGI("* Mounting mirrors");
	bool system_as_root = false;
	file_readline("/proc/mounts", [&](string_view &line) -> bool {
		if (str_contains(line, " /system_root ")) {
			bind_mount("/system_root/system", MIRRDIR "/system");
			sscanf(line.data(), "%s", buf);
			system_block = strdup(buf);
			system_as_root = true;
		} else if (!system_as_root && str_contains(line, " /system ")) {
			sscanf(line.data(), "%s %*s %s", buf, buf2);
			system_block = strdup(buf);
			xmount(system_block, MIRRDIR "/system", buf2, MS_RDONLY, nullptr);
			VLOGI("mount", system_block, MIRRDIR "/system");
		} else if (str_contains(line, " /vendor ")) {
			seperate_vendor = true;
			sscanf(line.data(), "%s %*s %s", buf, buf2);
			vendor_block = strdup(buf);
			xmkdir(MIRRDIR "/vendor", 0755);
			xmount(vendor_block, MIRRDIR "/vendor", buf2, MS_RDONLY, nullptr);
			VLOGI("mount", vendor_block, MIRRDIR "/vendor");
		} else if (str_contains(line, " /data ")) {
			sscanf(line.data(), "%s %*s %s", buf, buf2);
			data_block = strdup(buf);
			xmount(data_block, MIRRDIR "/data", buf2, 0, nullptr);
			VLOGI("mount", data_block, MIRRDIR "/data");
		} else if (SDK_INT >= 24 &&
		str_contains(line, " /proc ") && !str_contains(line, "hidepid=2")) {
			// Enforce hidepid
			xmount(nullptr, "/proc", nullptr, MS_REMOUNT, "hidepid=2,gid=3009");
		}
		return true;
	});
	if (!seperate_vendor) {
		xsymlink(MIRRDIR "/system/vendor", MIRRDIR "/vendor");
		VLOGI("link", MIRRDIR "/system/vendor", MIRRDIR "/vendor");
	}

	if (access(DATABIN "/busybox", X_OK) == -1)
		return false;
	LOGI("* Setting up internal busybox");
	close(xopen(BBPATH "/busybox", O_RDONLY | O_CREAT | O_CLOEXEC));
	bind_mount(DATABIN "/busybox", BBPATH "/busybox", false);
	exec_command_sync(BBPATH "/busybox", "--install", "-s", BBPATH);

	// Disable/remove magiskhide, resetprop, and modules
	if (SDK_INT < 19) {
		close(xopen(DISABLEFILE, O_RDONLY | O_CREAT | O_CLOEXEC, 0));
		unlink("/sbin/resetprop");
		unlink("/sbin/magiskhide");
	}
	return true;
}

static void prepare_modules() {
	const char *legacy_imgs[] = {SECURE_DIR "/magisk.img", SECURE_DIR "/magisk_merge.img"};
	for (auto img : legacy_imgs) {
		if (access(img, F_OK) == 0)
			migrate_img(img);
	}
	DIR *dir;
	struct dirent *entry;
	if ((dir = opendir(MODULEUPGRADE))) {
		while ((entry = xreaddir(dir))) {
			if (entry->d_type == DT_DIR) {
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
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
		closedir(dir);
		rm_rf(MODULEUPGRADE);
	}
	bind_mount(MIRRDIR MODULEROOT, MODULEMNT, false);
	// Legacy support
	xmkdir(LEGACYCORE, 0755);
	symlink(SECURE_DIR "/post-fs-data.d", LEGACYCORE "/post-fs-data.d");
	symlink(SECURE_DIR "/service.d", LEGACYCORE "/service.d");
}

static void collect_modules() {
	chdir(MODULEROOT);
	rm_rf("lost+found");
	DIR *dir = xopendir(".");
	struct dirent *entry;
	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				strcmp(entry->d_name, ".core") == 0)
				continue;
			chdir(entry->d_name);
			if (access("remove", F_OK) == 0) {
				chdir("..");
				LOGI("%s: remove\n", entry->d_name);
				rm_rf(entry->d_name);
				continue;
			}
			unlink("update");
			if (access("disable", F_OK))
				module_list.emplace_back(entry->d_name);
			chdir("..");
		}
	}
	closedir(dir);
	chdir("/");
}

static bool check_data() {
	bool mnt = false;
	bool data = false;
	file_readline("/proc/mounts", [&](string_view &s) -> bool {
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
	DIR *dir;
	struct dirent *entry;
	int fd, dev, OFF = 0;

	if ((dev = xopen("/dev/block", O_RDONLY | O_CLOEXEC)) < 0)
		return;
	dir = xfdopendir(dev);

	while((entry = readdir(dir))) {
		if (entry->d_type == DT_BLK) {
			if ((fd = openat(dev, entry->d_name, O_RDONLY)) < 0)
				continue;
			if (ioctl(fd, BLKROSET, &OFF) == -1)
				PLOGE("unlock %s", entry->d_name);
			close(fd);
		}
	}
	close(dev);
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
	new_daemon_thread([](auto) -> void* {
		int fd = xopen(LOGFILE, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644);
		exec_t exec {
			.fd = fd,
			.fork = fork_no_zombie
		};
		int pid = exec_command(exec, "/system/bin/logcat", "-s", "Magisk");
		close(fd);
		if (pid < 0) {
			log_dump = false;
			return nullptr;
		}
		waitpid(pid, nullptr, 0);
		return nullptr;
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
	auto_start_magiskhide();
	cp_afc("/sbin/magisk", MAGISKTMP "/app_process");
	struct stat st;
	for (const char *app : { "app_process", "app_process32", "app_process64" }) {
		sprintf(buf, "/system/bin/%s", app);
		if (lstat(buf, &st) == 0 && S_ISREG(st.st_mode)) {
			clone_attr(buf, MAGISKTMP "/app_process");
			bind_mount(MAGISKTMP "/app_process", buf);
		}
	}
	unblock_boot_process();
}

void post_fs_data(int client) {
	// ack
	write_int(client, 0);
	close(client);

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
		unblock_boot_process();
	}

#if 0
	// Increment boot count
	int boot_count = 0;
	FILE *cf = fopen(BOOTCOUNT, "r");
	if (cf) {
		fscanf(cf, "%d", &boot_count);
		fclose(cf);
	}
	boot_count++;
	if (boot_count > 2)
		creat(DISABLEFILE, 0644);
	cf = xfopen(BOOTCOUNT, "w");
	fprintf(cf, "%d", boot_count);
	fclose(cf);
#endif

	// No uninstaller or core-only mode
	if (access(DISABLEFILE, F_OK) != 0) {
		simple_mount("/system");
		simple_mount("/vendor");
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

	restorecon();
	chmod(SECURE_DIR, 0700);

	collect_modules();

	// Execute module scripts
	LOGI("* Running module post-fs-data scripts\n");
	exec_module_script("post-fs-data", module_list);

	// Recollect modules
	module_list.clear();
	collect_modules();

	// Create the system root entry
	auto sys_root = new node_entry("system", IS_INTER);

	// Vendor root entry
	node_entry *ven_root = nullptr;

	bool has_modules = false;

	LOGI("* Loading modules\n");
	for (const auto &m : module_list) {
		const auto module = m.c_str();
		// Read props
		snprintf(buf, PATH_MAX, "%s/%s/system.prop", MODULEROOT, module);
		if (access(buf, F_OK) == 0) {
			LOGI("%s: loading [system.prop]\n", module);
			load_prop_file(buf, false);
		}
		// Check whether enable auto_mount
		snprintf(buf, PATH_MAX, "%s/%s/auto_mount", MODULEROOT, module);
		if (access(buf, F_OK) == -1)
			continue;
		// Double check whether the system folder exists
		snprintf(buf, PATH_MAX, "%s/%s/system", MODULEROOT, module);
		if (access(buf, F_OK) == -1)
			continue;

		// Construct structure
		has_modules = true;
		LOGI("%s: constructing magic mount structure\n", module);
		// If /system/vendor exists in module, create a link outside
		snprintf(buf, PATH_MAX, "%s/%s/system/vendor", MODULEROOT, module);
		if (access(buf, F_OK) == 0) {
			snprintf(buf2, PATH_MAX, "%s/%s/vendor", MODULEROOT, module);
			unlink(buf2);
			xsymlink(buf, buf2);
		}
		sys_root->create_module_tree(module);
	}

	if (has_modules) {
		// Pull out /system/vendor node if exist
		ven_root = sys_root->extract("vendor");

		// Magic!!
		sys_root->magic_mount();
		if (ven_root) ven_root->magic_mount();
	}

	// Cleanup memory
	delete sys_root;
	delete ven_root;

	core_only();
}

void late_start(int client) {
	LOGI("** late_start service mode running\n");
	// ack
	write_int(client, 0);
	close(client);

	dump_logs();

	if (access(SECURE_DIR, F_OK) != 0) {
		// It's safe to create the folder at this point if the system didn't create it
		xmkdir(SECURE_DIR, 0700);
		// And reboot to make proper setup possible
		exec_command_sync("/system/bin/reboot");
	}

	auto_start_magiskhide();

	// Run scripts after full patch, most reliable way to run scripts
	LOGI("* Running service.d scripts\n");
	exec_common_script("service");

	// Core only mode
	if (access(DISABLEFILE, F_OK) == 0)
		goto core_only;

	LOGI("* Running module service scripts\n");
	exec_module_script("service", module_list);

core_only:
	if (access(MANAGERAPK, F_OK) == 0) {
		// Install Magisk Manager if exists
		rename(MANAGERAPK, "/data/magisk.apk");
		install_apk("/data/magisk.apk");
	} else {
		// Check whether we have a valid manager installed
		db_strings str;
		get_db_strings(&str, SU_MANAGER);
		if (validate_manager(str[SU_MANAGER], 0, nullptr)) {
			// There is no manager installed, install the stub
			exec_command_sync("/sbin/magiskinit", "-x", "manager", "/data/magisk.apk");
			install_apk("/data/magisk.apk");
		}
	}

	// All boot stage done, cleanup
	module_list.clear();
}

void boot_complete(int client) {
	LOGI("** boot_complete triggered\n");
	// ack
	write_int(client, 0);
	close(client);

	unlink(BOOTCOUNT);
}
