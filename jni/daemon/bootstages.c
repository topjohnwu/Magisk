/* bootstages.c - Core bootstage operations
 *
 * All bootstage operations, including simple mount in post-fs,
 * magisk mount in post-fs-data, various image handling, script
 * execution, load modules, install Magisk Manager etc.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <linux/loop.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"
#include "resetprop.h"

static char *buf, *buf2;
static struct vector module_list;
static int seperate_vendor = 0;

extern char **environ;

/******************
 * Node structure *
 ******************/

// Precedence: MODULE > SKEL > INTER > DUMMY
#define IS_DUMMY   0x01    /* mount from mirror */
#define IS_INTER   0x02    /* intermediate node */
#define IS_SKEL    0x04    /* mount from skeleton */
#define IS_MODULE  0x08    /* mount from module */

#define IS_DIR(n)  (n->type == DT_DIR)
#define IS_LNK(n)  (n->type == DT_LNK)
#define IS_REG(n)  (n->type == DT_REG)

struct node_entry {
	const char *module;    /* Only used when status & IS_MODULE */
	char *name;
	uint8_t type;
	uint8_t status;
	struct node_entry *parent;
	struct vector *children;
};

static void concat_path(struct node_entry *node) {
	if (node->parent)
		concat_path(node->parent);
	int len = strlen(buf);
	buf[len] = '/';
	strcpy(buf + len + 1, node->name);
}

static char *get_full_path(struct node_entry *node) {
	buf[0] = '\0';
	concat_path(node);
	return strdup(buf);
}

// Free the node
static void destroy_node(struct node_entry *node) {
	free(node->name);
	vec_destroy(node->children);
	free(node->children);
	free(node);
}

// Free the node and all children recursively
static void destroy_subtree(struct node_entry *node) {
	// Never free parent, since it shall be freed by themselves
	struct node_entry *e;
	vec_for_each(node->children, e) {
		destroy_subtree(e);
	}
	destroy_node(node);
}

// Return the child
static struct node_entry *insert_child(struct node_entry *p, struct node_entry *c) {
	c->parent = p;
	if (p->children == NULL) {
		p->children = xmalloc(sizeof(struct vector));
		vec_init(p->children);
	}
	struct node_entry *e;
	vec_for_each(p->children, e) {
		if (strcmp(e->name, c->name) == 0) {
			// Exist duplicate
			if (c->status > e->status) {
				// Precedence is higher, replace with new node
				destroy_subtree(e);
				vec_cur(p->children) = c;
				return c;
			} else {
				// Free the new entry, return old
				destroy_node(c);
				return e;
			}
		}
	}
	// New entry, push back
	vec_push_back(p->children, c);
	return c;
}

/***********
 * setenvs *
 ***********/

static void bb_setenv(struct vector *v) {
	for (int i = 0; environ[i]; ++i) {
		if (strncmp(environ[i], "PATH=", 5) == 0) {
			snprintf(buf, PATH_MAX, "PATH=%s:%s", BBPATH, strchr(environ[i], '=') + 1);
			vec_push_back(v, strdup(buf));
		} else {
			vec_push_back(v, strdup(environ[i]));
		}
	}
	vec_push_back(v, NULL);
}

static void pm_setenv(struct vector *v) {
	for (int i = 0; environ[i]; ++i) {
		if (strncmp(environ[i], "CLASSPATH=", 10) != 0)
			vec_push_back(v, strdup(environ[i]));
	}
	vec_push_back(v, strdup("CLASSPATH=/system/framework/pm.jar"));
	vec_push_back(v, NULL);
}

/***********
 * Scripts *
 ***********/

static void exec_common_script(const char* stage) {
	DIR *dir;
	struct dirent *entry;
	snprintf(buf, PATH_MAX, "%s/%s.d", COREDIR, stage);

	if (!(dir = xopendir(buf)))
		return;

	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_REG) {
			snprintf(buf2, PATH_MAX, "%s/%s", buf, entry->d_name);
			if (access(buf2, X_OK) == -1)
				continue;
			LOGI("%s.d: exec [%s]\n", stage, entry->d_name);
			int pid = exec_command(0, NULL, bb_setenv, "sh", buf2, NULL);
			if (pid != -1)
				waitpid(pid, NULL, 0);
		}
	}

	closedir(dir);
}

static void exec_module_script(const char* stage) {
	char *module;
	vec_for_each(&module_list, module) {
		snprintf(buf2, PATH_MAX, "%s/%s/%s.sh", MOUNTPOINT, module, stage);
		snprintf(buf, PATH_MAX, "%s/%s/disable", MOUNTPOINT, module);
		if (access(buf2, F_OK) == -1 || access(buf, F_OK) == 0)
			continue;
		LOGI("%s: exec [%s.sh]\n", module, stage);
		int pid = exec_command(0, NULL, bb_setenv, "sh", buf2, NULL);
		if (pid != -1)
			waitpid(pid, NULL, 0);
	}

}

/***************
 * Magic Mount *
 ***************/

static void construct_tree(const char *module, struct node_entry *parent) {
	DIR *dir;
	struct dirent *entry;
	struct node_entry *node;

	char *parent_path = get_full_path(parent);
	snprintf(buf, PATH_MAX, "%s/%s%s", MOUNTPOINT, module, parent_path);

	if (!(dir = xopendir(buf)))
		goto cleanup;

	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		// Create new node
		node = xcalloc(sizeof(*node), 1);
		node->module = module;
		node->name = strdup(entry->d_name);
		node->type = entry->d_type;
		snprintf(buf, PATH_MAX, "%s/%s", parent_path, node->name);

		/*
		 * Clone the parent in the following condition:
		 * 1. File in module is a symlink
		 * 2. Target file do not exist
		 * 3. Target file is a symlink, but not /system/vendor
		 */
		int clone = 0;
		if (IS_LNK(node) || access(buf, F_OK) == -1) {
			clone = 1;
		} else if (parent->parent != NULL || strcmp(node->name, "vendor") != 0) {
			struct stat s;
			xstat(buf, &s);
			if (S_ISLNK(s.st_mode))
				clone = 1;
		}

		if (clone) {
			// Mark the parent folder as a skeleton
			parent->status |= IS_SKEL;  /* This will not overwrite if parent is module */
			node->status = IS_MODULE;
		} else if (IS_DIR(node)) {
			// Check if marked as replace
			snprintf(buf2, PATH_MAX, "%s/%s%s/.replace", MOUNTPOINT, module, buf);
			if (access(buf2, F_OK) == 0) {
				// Replace everything, mark as leaf
				node->status = IS_MODULE;
			} else {
				// This will be an intermediate node
				node->status = IS_INTER;
			}
		} else if (IS_REG(node)) {
			// This is a leaf, mark as target
			node->status = IS_MODULE;
		}
		node = insert_child(parent, node);
		if (node->status & (IS_SKEL | IS_INTER)) {
			// Intermediate folder, travel deeper
			construct_tree(module, node);
		}
	}

	closedir(dir);

cleanup:
	free(parent_path);
}

static void clone_skeleton(struct node_entry *node) {
	DIR *dir;
	struct dirent *entry;
	struct node_entry *dummy, *child;

	// Clone the structure
	char *full_path = get_full_path(node);
	snprintf(buf, PATH_MAX, "%s%s", MIRRDIR, full_path);
	if (!(dir = xopendir(buf)))
		goto cleanup;
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		// Create dummy node
		dummy = xcalloc(sizeof(*dummy), 1);
		dummy->name = strdup(entry->d_name);
		dummy->type = entry->d_type;
		dummy->status = IS_DUMMY;
		insert_child(node, dummy);
	}
	closedir(dir);

	if (node->status & IS_SKEL) {
		struct stat s;
		char *con;
		xstat(full_path, &s);
		getfilecon(full_path, &con);
		LOGI("tmpfs: %s\n", full_path);
		xmount("tmpfs", full_path, "tmpfs", 0, NULL);
		chmod(full_path, s.st_mode & 0777);
		chown(full_path, s.st_uid, s.st_gid);
		setfilecon(full_path, con);
		free(con);
	}

	vec_for_each(node->children, child) {
		snprintf(buf, PATH_MAX, "%s/%s", full_path, child->name);

		// Create the dummy file/directory
		if (IS_DIR(child))
			xmkdir(buf, 0755);
		else if (IS_REG(child))
			close(open_new(buf));
		// Links will be handled later

		if (child->parent->parent == NULL && strcmp(child->name, "vendor") == 0) {
			if (IS_LNK(child)) {
				cp_afc(MIRRDIR "/system/vendor", "/system/vendor");
				LOGI("cplink: %s -> %s\n", MIRRDIR "/system/vendor", "/system/vendor");
			}
			// Skip
			continue;
		} else if (child->status & IS_MODULE) {
			// Mount from module file to dummy file
			snprintf(buf2, PATH_MAX, "%s/%s%s/%s", MOUNTPOINT, child->module, full_path, child->name);
		} else if (child->status & (IS_SKEL | IS_INTER)) {
			// It's an intermediate folder, recursive clone
			clone_skeleton(child);
			continue;
		} else if (child->status & IS_DUMMY) {
			// Mount from mirror to dummy file
			snprintf(buf2, PATH_MAX, "%s%s/%s", MIRRDIR, full_path, child->name);
		}

		if (IS_LNK(child)) {
			// Copy symlinks directly
			cp_afc(buf2, buf);
			#ifdef MAGISK_DEBUG
				LOGI("cplink: %s -> %s\n",buf2, buf);
			#else
				LOGI("cplink: %s\n", buf);
			#endif
		} else {
			snprintf(buf, PATH_MAX, "%s/%s", full_path, child->name);
			bind_mount(buf2, buf);
		}
	}

cleanup:
	free(full_path);
}

static void magic_mount(struct node_entry *node) {
	char *real_path;
	struct node_entry *child;

	if (node->status & IS_MODULE) {
		// The real deal, mount module item
		real_path = get_full_path(node);
		snprintf(buf, PATH_MAX, "%s/%s%s", MOUNTPOINT, node->module, real_path);
		bind_mount(buf, real_path);
		free(real_path);
	} else if (node->status & IS_SKEL) {
		// The node is labeled to be cloned with skeleton, lets do it
		clone_skeleton(node);
	} else if (node->status & IS_INTER) {
		// It's an intermediate node, travel deeper
		vec_for_each(node->children, child)
			magic_mount(child);
	}
	// The only thing goes here should be vendor placeholder
	// There should be no dummies, so don't need to handle it here
}

/****************
 * Simple Mount *
 ****************/

static void simple_mount(const char *path) {
	DIR *dir;
	struct dirent *entry;

	snprintf(buf, PATH_MAX, "%s%s", CACHEMOUNT, path);
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
			char *new_path = strdup(buf2);
			simple_mount(new_path);
			free(new_path);
		} else if (entry->d_type == DT_REG) {
			// Actual file path
			snprintf(buf, PATH_MAX, "%s%s", CACHEMOUNT, buf2);
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

// A one time setup
static void daemon_init() {
	LOGI("* Creating /sbin overlay");
	DIR *dir;
	struct dirent *entry;
	int root, sbin;
	// Setup links under /sbin
	xmount(NULL, "/", NULL, MS_REMOUNT, NULL);
	xmkdir("/root", 0755);
	chmod("/root", 0755);
	root = xopen("/root", O_RDONLY | O_CLOEXEC);
	sbin = xopen("/sbin", O_RDONLY | O_CLOEXEC);
	dir = xfdopendir(sbin);
	while((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		linkat(sbin, entry->d_name, root, entry->d_name, 0);
		if (strcmp(entry->d_name, "magisk") == 0)
			unlinkat(sbin, entry->d_name, 0);
	}
	close(sbin);
	mount("tmpfs", "/sbin", "tmpfs", 0, NULL);
	sbin = xopen("/sbin", O_RDONLY | O_CLOEXEC);
	fchmod(sbin, 0755);
	fsetfilecon(sbin, "u:object_r:rootfs:s0");
	dir = xfdopendir(root);
	while((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		snprintf(buf, PATH_MAX, "/root/%s", entry->d_name);
		snprintf(buf2, PATH_MAX, "/sbin/%s", entry->d_name);
		xsymlink(buf, buf2);
	}
	for (int i = 0; applet[i]; ++i) {
		snprintf(buf2, PATH_MAX, "/sbin/%s", applet[i]);
		xsymlink("/root/magisk", buf2);
	}
	xmkdir("/magisk", 0755);
	xmount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL);

	LOGI("* Mounting mirrors");
	struct vector mounts;
	vec_init(&mounts);
	file_to_vector("/proc/mounts", &mounts);
	char *line;
	int skip_initramfs = 0;
	// Check whether skip_initramfs device
	vec_for_each(&mounts, line) {
		if (strstr(line, " /system_root ")) {
			xmkdir_p(MIRRDIR "/system", 0755);
			bind_mount("/system_root/system", MIRRDIR "/system");
			skip_initramfs = 1;
			break;
		}
	}
	vec_for_each(&mounts, line) {
		if (!skip_initramfs && strstr(line, " /system ")) {
			sscanf(line, "%s", buf);
			xmkdir_p(MIRRDIR "/system", 0755);
			xmount(buf, MIRRDIR "/system", "ext4", MS_RDONLY, NULL);
			#ifdef MAGISK_DEBUG
				LOGI("mount: %s -> %s\n", buf, MIRRDIR "/system");
			#else
				LOGI("mount: %s\n", MIRRDIR "/system");
			#endif
		} else if (strstr(line, " /vendor ")) {
			seperate_vendor = 1;
			sscanf(line, "%s", buf);
			xmkdir_p(MIRRDIR "/vendor", 0755);
			xmount(buf, MIRRDIR "/vendor", "ext4", MS_RDONLY, NULL);
			#ifdef MAGISK_DEBUG
				LOGI("mount: %s -> %s\n", buf, MIRRDIR "/vendor");
			#else
				LOGI("mount: %s\n", MIRRDIR "/vendor");
			#endif
		}
		free(line);
	}
	vec_destroy(&mounts);
	if (!seperate_vendor) {
		xsymlink(MIRRDIR "/system/vendor", MIRRDIR "/vendor");
		#ifdef MAGISK_DEBUG
			LOGI("link: %s -> %s\n", MIRRDIR "/system/vendor", MIRRDIR "/vendor");
		#else
			LOGI("link: %s\n", MIRRDIR "/vendor");
		#endif
	}
	xmkdir_p(MIRRDIR "/bin", 0755);
	bind_mount(DATABIN, MIRRDIR "/bin");

	LOGI("* Setting up internal busybox");
	xmkdir_p(BBPATH, 0755);
	exec_command_sync(MIRRDIR "/bin/busybox", "--install", "-s", BBPATH, NULL);
	xsymlink(MIRRDIR "/bin/busybox", BBPATH "/busybox");
}

static int prepare_img() {
	// First merge images
	if (merge_img("/data/magisk_merge.img", MAINIMG)) {
		LOGE("Image merge /data/magisk_merge.img -> " MAINIMG " failed!\n");
		return 1;
	}

	if (access(MAINIMG, F_OK) == -1) {
		if (create_img(MAINIMG, 64))
			return 1;
	}

	LOGI("* Mounting " MAINIMG "\n");
	// Mounting magisk image
	char *magiskloop = mount_image(MAINIMG, MOUNTPOINT);
	if (magiskloop == NULL)
		return 1;

	vec_init(&module_list);

	xmkdir(COREDIR, 0755);
	xmkdir(COREDIR "/post-fs-data.d", 0755);
	xmkdir(COREDIR "/service.d", 0755);
	xmkdir(COREDIR "/props", 0755);

	DIR *dir = xopendir(MOUNTPOINT);
	struct dirent *entry;
	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				strcmp(entry->d_name, ".core") == 0 ||
				strcmp(entry->d_name, "lost+found") == 0)
				continue;
			snprintf(buf, PATH_MAX, "%s/%s/remove", MOUNTPOINT, entry->d_name);
			if (access(buf, F_OK) == 0) {
				snprintf(buf, PATH_MAX, "%s/%s", MOUNTPOINT, entry->d_name);
				rm_rf(buf);
				continue;
			}
			snprintf(buf, PATH_MAX, "%s/%s/disable", MOUNTPOINT, entry->d_name);
			if (access(buf, F_OK) == 0)
				continue;
			vec_push_back(&module_list, strdup(entry->d_name));
		}
	}

	closedir(dir);

	// Trim image
	umount_image(MOUNTPOINT, magiskloop);
	free(magiskloop);
	trim_img(MAINIMG);

	// Remount them back :)
	magiskloop = mount_image(MAINIMG, MOUNTPOINT);
	free(magiskloop);

	// Fix file selinux contexts
	fix_filecon();
	return 0;
}

void fix_filecon() {
	int dirfd = xopen(MOUNTPOINT, O_RDONLY | O_CLOEXEC);
	restorecon(dirfd, 0);
	close(dirfd);
	dirfd = xopen(DATABIN, O_RDONLY | O_CLOEXEC);
	restorecon(dirfd, 1);
	close(dirfd);
}

/****************
 * Entry points *
 ****************/

static void unblock_boot_process() {
	close(xopen(UNBLOCKFILE, O_RDONLY | O_CREAT));
	pthread_exit(NULL);
}

void post_fs(int client) {
	LOGI("** post-fs mode running\n");
	// ack
	write_int(client, 0);
	close(client);

	// Uninstall or core only mode
	if (access(UNINSTALLER, F_OK) == 0 || access(DISABLEFILE, F_OK) == 0)
		goto unblock;

	// Allocate buffer
	buf = xmalloc(PATH_MAX);
	buf2 = xmalloc(PATH_MAX);

	simple_mount("/system");
	simple_mount("/vendor");

unblock:
	unblock_boot_process();
}

void post_fs_data(int client) {
	// ack
	write_int(client, 0);
	close(client);
	if (!check_data())
		goto unblock;

	// Start the debug log
	start_debug_full_log();

	LOGI("** post-fs-data mode running\n");

	// Allocate buffer
	if (buf == NULL) buf = xmalloc(PATH_MAX);
	if (buf2 == NULL) buf2 = xmalloc(PATH_MAX);

	// Magisk binaries
	char *bin_path = NULL;
	if (access("/cache/data_bin", F_OK) == 0)
		bin_path = "/cache/data_bin";
	else if (access("/data/data/com.topjohnwu.magisk/install", F_OK) == 0)
		bin_path = "/data/data/com.topjohnwu.magisk/install";
	else if (access("/data/user_de/0/com.topjohnwu.magisk/install", F_OK) == 0)
		bin_path = "/data/user_de/0/com.topjohnwu.magisk/install";
	if (bin_path) {
		rm_rf(DATABIN);
		cp_afc(bin_path, DATABIN);
		rm_rf(bin_path);
		// Lazy.... use shell blob to match files
		exec_command_sync("sh", "-c", "mv /data/magisk/stock_boot* /data", NULL);
	}

	// Initialize
	daemon_init();

	// uninstaller
	if (access(UNINSTALLER, F_OK) == 0) {
		close(open(UNBLOCKFILE, O_RDONLY | O_CREAT));
		setenv("BOOTMODE", "true", 1);
		exec_command(0, NULL, bb_setenv, "sh", UNINSTALLER, NULL);
		return;
	}

	// Merge, trim, mount magisk.img, which will also travel through the modules
	// After this, it will create the module list
	if (prepare_img())
		goto core_only; // Mounting fails, we can only do core only stuffs

	// Run common scripts
	LOGI("* Running post-fs-data.d scripts\n");
	exec_common_script("post-fs-data");

	// Core only mode
	if (access(DISABLEFILE, F_OK) == 0)
		goto core_only;

	// Execute module scripts
	LOGI("* Running module post-fs-data scripts\n");
	exec_module_script("post-fs-data");

	char *module;
	struct node_entry *sys_root, *ven_root = NULL, *child;

	// Create the system root entry
	sys_root = xcalloc(sizeof(*sys_root), 1);
	sys_root->name = strdup("system");
	sys_root->status = IS_INTER;

	int has_modules = 0;

	LOGI("* Loading modules\n");
	vec_for_each(&module_list, module) {
		// Read props
		snprintf(buf, PATH_MAX, "%s/%s/system.prop", MOUNTPOINT, module);
		if (access(buf, F_OK) == 0) {
			LOGI("%s: loading [system.prop]\n", module);
			read_prop_file(buf, 0);
		}
		// Check whether enable auto_mount
		snprintf(buf, PATH_MAX, "%s/%s/auto_mount", MOUNTPOINT, module);
		if (access(buf, F_OK) == -1)
			continue;
		// Double check whether the system folder exists
		snprintf(buf, PATH_MAX, "%s/%s/system", MOUNTPOINT, module);
		if (access(buf, F_OK) == -1)
			continue;

		// Construct structure
		has_modules = 1;
		LOGI("%s: constructing magic mount structure\n", module);
		// If /system/vendor exists in module, create a link outside
		snprintf(buf, PATH_MAX, "%s/%s/system/vendor", MOUNTPOINT, module);
		if (access(buf, F_OK) == 0) {
			snprintf(buf2, PATH_MAX, "%s/%s/vendor", MOUNTPOINT, module);
			unlink(buf2);
			xsymlink(buf, buf2);
		}
		construct_tree(module, sys_root);
	}

	if (has_modules) {
		// Extract the vendor node out of system tree and swap with placeholder
		vec_for_each(sys_root->children, child) {
			if (strcmp(child->name, "vendor") == 0) {
				ven_root = child;
				child = xcalloc(sizeof(*child), 1);
				child->type = seperate_vendor ? DT_LNK : DT_DIR;
				child->parent = ven_root->parent;
				child->name = strdup("vendor");
				child->status = 0;
				// Swap!
				vec_cur(sys_root->children) = child;
				ven_root->parent = NULL;
				break;
			}
		}

		// Magic!!
		magic_mount(sys_root);
		if (ven_root) magic_mount(ven_root);
	}

	// Cleanup memory
	destroy_subtree(sys_root);
	if (ven_root) destroy_subtree(ven_root);

core_only:
	// Systemless hosts
	if (access(HOSTSFILE, F_OK) == 0) {
		LOGI("* Enabling systemless hosts file support");
		bind_mount(HOSTSFILE, "/system/etc/hosts");
	}

	auto_start_magiskhide();

unblock:
	unblock_boot_process();
}

void late_start(int client) {
	LOGI("** late_start service mode running\n");
	// ack
	write_int(client, 0);
	close(client);

	// Allocate buffer
	if (buf == NULL) buf = xmalloc(PATH_MAX);
	if (buf2 == NULL) buf2 = xmalloc(PATH_MAX);

	// Wait till the full patch is done
	pthread_join(sepol_patch, NULL);

	// Run scripts after full patch, most reliable way to run scripts
	LOGI("* Running service.d scripts\n");
	exec_common_script("service");

	// Core only mode
	if (access(DISABLEFILE, F_OK) == 0)
		goto core_only;

	LOGI("* Running module service scripts\n");
	exec_module_script("service");

core_only:
	// Install Magisk Manager if exists
	if (access(MANAGERAPK, F_OK) == 0) {
		while (1) {
			sleep(5);
			int apk_res = -1, pid;
			pid = exec_command(1, &apk_res, pm_setenv,
				"app_process",
				"/system/bin", "com.android.commands.pm.Pm",
				"install", "-r", MANAGERAPK, NULL);
			if (pid != -1) {
				waitpid(pid, NULL, 0);
				fdgets(buf, PATH_MAX, apk_res);
				close(apk_res);
				// Keep trying until pm is started
				if (strstr(buf, "Error:") == NULL)
					break;
			}
		}
		unlink(MANAGERAPK);
	}

	// All boot stage done, cleanup everything
	free(buf);
	free(buf2);
	buf = buf2 = NULL;
	vec_deep_destroy(&module_list);

	stop_debug_full_log();
}
