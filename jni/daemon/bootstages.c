/* post_fs_data.c - post-fs-data actions
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

#ifdef DEBUG
static int debug_log_pid, debug_log_fd;
#endif

/******************
 * Node structure *
 ******************/

// Precedence: MODULE > SKEL > DUMMY
#define DO_NOTHING 0x0    /* intermediate node */
#define IS_DUMMY   0x1    /* mount from mirror */
#define IS_SKEL    0x2    /* mount from skeleton */
#define IS_MODULE  0x4    /* mount from module */

struct node_entry {
	const char *module;    /* Only used when status & IS_MODULE */
	char *name;
	uint8_t type;
	uint8_t status;
	struct node_entry *parent;
	struct vector *children;
};

#define IS_DIR(n)  (n->type == DT_DIR)
#define IS_LNK(n)  (n->type == DT_LNK)
#define IS_REG(n)  (n->type == DT_REG)

/******************
 * Image handling *
 ******************/

static char *loopsetup(const char *img) {
	char device[20];
	struct loop_info64 info;
	int i, lfd, ffd;
	memset(&info, 0, sizeof(info));
	// First get an empty loop device
	for (i = 0; i <= 7; ++i) {
		sprintf(device, "/dev/block/loop%d", i);
		lfd = xopen(device, O_RDWR);
		if (ioctl(lfd, LOOP_GET_STATUS64, &info) == -1)
			break;
		close(lfd);
	}
	if (i == 8) return NULL;
	ffd = xopen(img, O_RDWR);
	if (ioctl(lfd, LOOP_SET_FD, ffd) == -1)
		return NULL;
	strcpy((char *) info.lo_file_name, img);
	ioctl(lfd, LOOP_SET_STATUS64, &info);
	close(lfd);
	close(ffd);
	return strdup(device);
}

static char *mount_image(const char *img, const char *target) {
	char *device = loopsetup(img);
	if (device)
		xmount(device, target, "ext4", 0, NULL);
	return device;
}

static void umount_image(const char *target, const char *device) {
	xumount(target);
	int fd = xopen(device, O_RDWR);
	ioctl(fd, LOOP_CLR_FD);
	close(fd);
}

#define round_size(a) ((((a) / 32) + 2) * 32)
#define SOURCE_TMP "/dev/source"
#define TARGET_TMP "/dev/target"

static int merge_img(const char *source, const char *target) {
	if (access(source, F_OK) == -1)
		return 0;
	if (access(target, F_OK) == -1) {
		rename(source, target);
		return 0;
	}
	
	// resize target to worst case
	int s_used, s_total, t_used, t_total, n_total;
	if (get_img_size(source, &s_used, &s_total)) return 1;
	if (get_img_size(target, &t_used, &t_total)) return 1;
	n_total = round_size(s_used + t_used);
	if (n_total != t_total)
		resize_img(target, n_total);

	mkdir(SOURCE_TMP, 0755);
	mkdir(TARGET_TMP, 0755);
	char *s_loop, *t_loop;
	s_loop = mount_image(source, SOURCE_TMP);
	if (s_loop == NULL) return 1;
	t_loop = mount_image(target, TARGET_TMP);
	if (t_loop == NULL) return 1;

	DIR *dir;
	struct dirent *entry;
	if (!(dir = opendir(SOURCE_TMP)))
		return 1;
	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				strcmp(entry->d_name, ".core") == 0 ||
				strcmp(entry->d_name, "lost+found") == 0)
				continue;
			// Cleanup old module
			snprintf(buf, PATH_MAX, "/dev/target/%s", entry->d_name);
			if (access(buf, F_OK) == 0) {
				LOGI("Upgrade module: %s\n", entry->d_name);
				rm_rf(buf);
			} else {
				LOGI("New module: %s\n", entry->d_name);
			}
		}
	}
	closedir(dir);
	clone_dir(SOURCE_TMP, TARGET_TMP);

	// Unmount all loop devices
	umount_image(SOURCE_TMP, s_loop);
	umount_image(TARGET_TMP, t_loop);
	rmdir(SOURCE_TMP);
	rmdir(TARGET_TMP);
	free(s_loop);
	free(t_loop);
	unlink(source);
	return 0;
}

static void trim_img(const char *img) {
	int used, total, new_size;
	get_img_size(img, &used, &total);
	new_size = round_size(used);
	if (new_size != total)
		resize_img(img, new_size);
}

/***********
 * Scripts *
 ***********/

void exec_common_script(const char* stage) {
	DIR *dir;
	struct dirent *entry;
	snprintf(buf, PATH_MAX, "%s/%s.d", COREDIR, stage);

	if (!(dir = opendir(buf)))
		return;

	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_REG) {
			snprintf(buf2, PATH_MAX, "%s/%s", buf, entry->d_name);
			if (access(buf2, X_OK) == -1)
				continue;
			LOGI("%s.d: exec [%s]\n", stage, entry->d_name);
			char *const command[] = { "sh", buf2, NULL };
			int pid = run_command(0, NULL, "/system/bin/sh", command);
			if (pid != -1)
				waitpid(pid, NULL, 0);
		}
	}

	closedir(dir);
}

void exec_module_script(const char* stage) {
	char *module;
	vec_for_each(&module_list, module) {
		snprintf(buf, PATH_MAX, "%s/%s/%s.sh", MOUNTPOINT, module, stage);
		if (access(buf, F_OK) == -1)
			continue;
		LOGI("%s: exec [%s.sh]\n", module, stage);
		char *const command[] = { "sh", buf, NULL };
		int pid = run_command(0, NULL, "/system/bin/sh", command);
		if (pid != -1)
			waitpid(pid, NULL, 0);
	}

}

/***************
 * Magic Mount *
 ***************/

static char *get_full_path(struct node_entry *node) {
	char buffer[PATH_MAX], temp[PATH_MAX];
	// Concat the paths
	struct node_entry *cur = node;
	strcpy(buffer, node->name);
	while (cur->parent) {
		strcpy(temp, buffer);
		snprintf(buffer, sizeof(buffer), "%s/%s", cur->parent->name, temp);
		cur = cur->parent;
	}
	return strdup(buffer);
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
				c->children = e->children;  // Preserve all children
				free(e->name);
				free(e);
				vec_entry(p->children)[_] = c;
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

static void construct_tree(const char *module, struct node_entry *parent) {
	DIR *dir;
	struct dirent *entry;
	struct node_entry *node;

	char *parent_path = get_full_path(parent);
	snprintf(buf, PATH_MAX, "%s/%s%s", MOUNTPOINT, module, parent_path);

	if (!(dir = opendir(buf)))
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
		// Check if the entry has a correspond target
		if (IS_LNK(node) || access(buf, F_OK) == -1) {
			// Mark the parent folder as a skeleton
			parent->status |= IS_SKEL;
			node->status |= IS_MODULE;
		} else if (IS_DIR(node)) {
			// Check if marked as replace
			snprintf(buf2, PATH_MAX, "%s/%s%s/.replace", MOUNTPOINT, module, buf);
			if (access(buf2, F_OK) == 0) {
				// Replace everything, mark as leaf
				node->status |= IS_MODULE;
			}
		} else if (IS_REG(node)) {
			// This is a leaf, mark as target
			node->status |= IS_MODULE;
		}
		node = insert_child(parent, node);
		if (IS_DIR(node) && !(node->status & IS_MODULE)) {
			// Intermediate node, travel deeper
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
	if (!(dir = opendir(full_path)))
		goto cleanup;
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		// Create dummy node
		dummy = xcalloc(sizeof(*dummy), 1);
		dummy->name = strdup(entry->d_name);
		dummy->type = entry->d_type;
		dummy->status |= IS_DUMMY;
		insert_child(node, dummy);
	}
	closedir(dir);

	snprintf(buf, PATH_MAX, "%s%s", DUMMDIR, full_path);
	mkdir_p(buf, 0755);
	clone_attr(full_path, buf);
	bind_mount(buf, full_path);

	vec_for_each(node->children, child) {
		snprintf(buf, PATH_MAX, "%s%s/%s", DUMMDIR, full_path, child->name);
		// Create the dummy file/directory
		if (IS_DIR(child))
			xmkdir(buf, 0755);
		else if (IS_REG(child))
			close(open_new(buf));

		if (child->status & IS_MODULE) {
			// Mount from module file to dummy file
			snprintf(buf2, PATH_MAX, "%s/%s%s/%s", MOUNTPOINT, child->module, full_path, child->name);
		} else if (child->status & IS_SKEL) {
			// It's another skeleton, recursive call and end
			clone_skeleton(child);
			continue;
		} else if (child->status & IS_DUMMY) {
			// Mount from mirror to dummy file
			snprintf(buf2, PATH_MAX, "%s%s/%s", MIRRDIR, full_path, child->name);
		}

		if (IS_LNK(child)) {
			// Symlink special treatments
			char *temp = xmalloc(PATH_MAX);
			xreadlink(buf2, temp, PATH_MAX);
			symlink(temp, buf);
			free(temp);
			LOGI("cplink: %s -> %s\n", buf2, buf);
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

	if (strcmp(node->name, "vendor") == 0 && strcmp(node->parent->name, "/system") == 0) {
		snprintf(buf, PATH_MAX, "%s/%s/system/vendor", MOUNTPOINT, node->module);
		snprintf(buf2, PATH_MAX, "%s/%s/vendor", MOUNTPOINT, node->module);
		unlink(buf2);
		symlink(buf, buf2);
		return;
	}

	if (node->status) {
		if (node->status & IS_MODULE) {
			// The real deal, mount module item
			real_path = get_full_path(node);
			snprintf(buf, PATH_MAX, "%s/%s%s", MOUNTPOINT, node->module, real_path);
			bind_mount(buf, real_path);
			free(real_path);
		} else if (node->status & IS_SKEL) {
			// The node is labeled to be cloned with skeleton, lets do it
			clone_skeleton(node);
		}
		// We should not see dummies, so no need to handle it here
	} else {
		// It's an intermediate node, travel deeper
		vec_for_each(node->children, child)
			magic_mount(child);
	}
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
			snprintf(buf, PATH_MAX, "%s/%s", buf, entry->d_name);
			// Clone all attributes
			clone_attr(buf2, buf);
			// Finally, mount the file
			bind_mount(buf, buf2);
		}
	}

	closedir(dir);
}

/****************
 * Entry points *
 ****************/

static void *start_magisk_hide(void *args) {
	launch_magiskhide(-1);
	return NULL;
}

static void unblock_boot_process() {
	close(open(UNBLOCKFILE, O_RDONLY | O_CREAT));
	pthread_exit(NULL);
}

void post_fs(int client) {
	// Error handler
	err_handler = unblock_boot_process;

	// Start log monitor
	monitor_logs();

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
	// Error handler
	err_handler = unblock_boot_process;

	// ack
	write_int(client, 0);
	close(client);
	if (!check_data())
		goto unblock;

#ifdef DEBUG
	// Start debug logs in new process
	debug_log_fd = xopen(DEBUG_LOG, O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, 0644);
	char *const command[] = { "logcat", "-v", "brief", NULL };
	debug_log_pid = run_command(0, &debug_log_fd, "/system/bin/logcat", command);
#endif

	LOGI("** post-fs-data mode running\n");

	// uninstaller
	if (access(UNINSTALLER, F_OK) == 0) {
		close(open(UNBLOCKFILE, O_RDONLY | O_CREAT));
		system("(BOOTMODE=true sh " UNINSTALLER ") &");
		return;
	}

	// Allocate buffer
	if (buf == NULL) buf = xmalloc(PATH_MAX);
	if (buf2 == NULL) buf2 = xmalloc(PATH_MAX);

	// Cache support
	if (access("/cache/data_bin", F_OK) == 0) {
		rm_rf(DATABIN);
		rename("/cache/data_bin", DATABIN);
	}

	// Magisk Manual Injector support
	if (access("/data/local/tmp/magisk_inject", F_OK) == 0) {
		rm_rf(DATABIN);
		rename("/data/local/tmp/magisk_inject", DATABIN);
	}

	// Lazy.... use shell blob
	system("mv /data/magisk/stock_boot* /data;");

	// Merge images
	if (merge_img("/cache/magisk.img", MAINIMG)) {
		LOGE("Image merge %s -> %s failed!\n", "/cache/magisk.img", MAINIMG);
		goto unblock;
	}
	if (merge_img("/data/magisk_merge.img", MAINIMG)) {
		LOGE("Image merge %s -> %s failed!\n", "/data/magisk_merge.img", MAINIMG);
		goto unblock;
	}

	int new_img = 0;

	if (access(MAINIMG, F_OK) == -1) {
		if (create_img(MAINIMG, 64))
			goto unblock;
		new_img = 1;
	}

	LOGI("* Mounting " MAINIMG "\n");
	// Mounting magisk image
	char *magiskloop = mount_image(MAINIMG, MOUNTPOINT);
	if (magiskloop == NULL)
		goto unblock;

	if (new_img) {
		mkdir(COREDIR, 0755);
		mkdir(COREDIR "/post-fs-data.d", 0755);
		mkdir(COREDIR "/service.d", 0755);
		mkdir(COREDIR "/props", 0755);
	}

	// Run common scripts
	LOGI("* Running post-fs-data.d scripts\n");
	exec_common_script("post-fs-data");

	// Core only mode
	if (access(DISABLEFILE, F_OK) == 0)
		goto unblock;

	DIR *dir;
	struct dirent *entry;
	char *module;
	struct node_entry *sys_root, *ven_root = NULL, *child;

	dir = xopendir(MOUNTPOINT);

	// Create the system root entry
	sys_root = xcalloc(sizeof(*sys_root), 1);
	sys_root->name = strdup("/system");

	int has_modules = 0;

	// Travel through each modules
	vec_init(&module_list);
	LOGI("* Loading modules\n");
	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				strcmp(entry->d_name, ".core") == 0 ||
				strcmp(entry->d_name, "lost+found") == 0)
				continue;
			snprintf(buf, PATH_MAX, "%s/%s", MOUNTPOINT, entry->d_name);
			// Check whether remove
			snprintf(buf2, PATH_MAX, "%s/remove", buf);
			if (access(buf2, F_OK) == 0) {
				rm_rf(buf);
				continue;
			}
			// Check whether disable
			snprintf(buf2, PATH_MAX, "%s/disable", buf);
			if (access(buf2, F_OK) == 0)
				continue;
			// Add the module to list
			module = strdup(entry->d_name);
			vec_push_back(&module_list, module);
			// Read props
			snprintf(buf2, PATH_MAX, "%s/system.prop", buf);
			if (access(buf2, F_OK) == 0) {
				LOGI("%s: loading [system.prop]\n", module);
				read_prop_file(buf2, 0);
			}
			// Check whether enable auto_mount
			snprintf(buf2, PATH_MAX, "%s/auto_mount", buf);
			if (access(buf2, F_OK) == -1)
				continue;
			// Double check whether the system folder exists
			snprintf(buf2, PATH_MAX, "%s/system", buf);
			if (access(buf2, F_OK) == -1)
				continue;

			// Construct structure
			has_modules = 1;
			LOGI("%s: constructing magic mount structure\n", module);
			construct_tree(module, sys_root);
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

	if (has_modules) {
		// Mount mirrors
		LOGI("* Mounting system/vendor mirrors");
		int seperate_vendor = 0;
		struct vector mounts;
		vec_init(&mounts);
		file_to_vector("/proc/mounts", &mounts);
		char *line;
		vec_for_each(&mounts, line) {
			if (strstr(line, " /system ")) {
				sscanf(line, "%s", buf);
				snprintf(buf2, PATH_MAX, "%s/system", MIRRDIR);
				xmkdir_p(buf2, 0755);
				xmount(buf, buf2, "ext4", MS_RDONLY, NULL);
				LOGI("mount: %s -> %s\n", buf, buf2);
				continue;
			}
			if (strstr(line, " /vendor ")) {
				seperate_vendor = 1;
				sscanf(line, "%s", buf);
				snprintf(buf2, PATH_MAX, "%s/vendor", MIRRDIR);
				xmkdir_p(buf2, 0755);
				xmount(buf, buf2, "ext4", MS_RDONLY, NULL);
				LOGI("mount: %s -> %s\n", buf, buf2);
				continue;
			}
		}
		vec_deep_destroy(&mounts);
		if (!seperate_vendor) {
			snprintf(buf, PATH_MAX, "%s/system/vendor", MIRRDIR);
			snprintf(buf2, PATH_MAX, "%s/vendor", MIRRDIR);
			symlink(buf, buf2);
			LOGI("link: %s -> %s\n", buf, buf2);
		}

		// Magic!!

		magic_mount(sys_root);
		// Get the vendor node if exists
		vec_for_each(sys_root->children, child) {
			if (strcmp(child->name, "vendor") == 0) {
				ven_root = child;
				free(ven_root->name);
				ven_root->name = strdup("/vendor");
				ven_root->parent = NULL;
				break;
			}
		}
		if (ven_root)
			magic_mount(ven_root);
	}

	// Cleanup memory
	destroy_subtree(sys_root);

	// Execute module scripts
	LOGI("* Running module post-fs-data scripts\n");
	exec_module_script("post-fs-data");

	// Systemless hosts
	if (access(HOSTSFILE, F_OK) == 0) {
		LOGI("* Enabling systemless hosts file support");
		bind_mount(HOSTSFILE, "/system/etc/hosts");
	}

	// Start magiskhide if enabled
	char *hide_prop = getprop(MAGISKHIDE_PROP);
	if (hide_prop) {
		if (strcmp(hide_prop, "1") == 0) {
			pthread_t thread;
			xpthread_create(&thread, NULL, start_magisk_hide, NULL);
			pthread_detach(thread);
		}
		free(hide_prop);
	}

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
	if (access(DISABLEFILE, F_OK) == 0) {
		setprop("ro.magisk.disable", "1");
		return;
	}

	LOGI("* Running module service scripts\n");
	exec_module_script("service");

	// Install Magisk Manager if exists
	if (access(MANAGERAPK, F_OK) == 0) {
		while (1) {
			sleep(5);
			char *const command[] = { "sh", "-c", 
				"CLASSPATH=/system/framework/pm.jar "
				"/system/bin/app_process /system/bin "
				"com.android.commands.pm.Pm install -r " MANAGERAPK, NULL };
			int apk_res = 0, pid;
			pid = run_command(1, &apk_res, "/system/bin/sh", command);
			waitpid(pid, NULL, 0);
			fdgets(buf, PATH_MAX, apk_res);
			close(apk_res);
			// Keep trying until pm is started
			if (strstr(buf, "Error:") == NULL)
				break;
		}
		unlink(MANAGERAPK);
	}

	// All boot stage done, cleanup everything
	free(buf);
	free(buf2);
	vec_deep_destroy(&module_list);

#ifdef DEBUG
	// Stop recording the boot logcat after every boot task is done
	kill(debug_log_pid, SIGTERM);
	waitpid(debug_log_pid, NULL, 0);
	close(debug_log_fd);
#endif
}
