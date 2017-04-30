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

#include "magisk.h"
#include "utils.h"
#include "daemon.h"
#include "resetprop.h"

static char *buf, *buf2;
static struct vector module_list;

/******************
 * Node structure *
 ******************/

#define DO_NOTHING 0x0    /* intermediate node */

#define IS_MODULE  0x1    /* mount from module */
#define IS_DUMMY   0x2    /* mount from mirror */
#define IS_SKEL    0x4    /* mount from skeleton */

struct node_entry {
	const char *module;
	char *name;
	uint8_t type;
	uint8_t status;
	struct node_entry *parent;
	struct vector *children;
};

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
	return strdup(device);
}

static char *mount_image(const char *img, const char *target) {
	char *device = loopsetup(img);
	if (device)
		xmount(device, target, "ext4", 0, NULL);
	return device;
}

/***********
 * Scripts *
 ***********/

void exec_common_script(const char* stage) {
	DIR *dir;
	struct dirent *entry;
	snprintf(buf, PATH_MAX, "/magisk/.core/%s.d", stage);

	if (!(dir = opendir(buf)))
		return;

	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_REG) {
			snprintf(buf, PATH_MAX, "%s/%s", buf, entry->d_name);
			if (access(buf, X_OK) == -1)
				continue;
			LOGI("%s.d: exec [%s]\n", stage, entry->d_name);
			char *const command[] = { "sh", buf, NULL };
			int pid = run_command(NULL, "/system/bin/sh", command);
			if (pid != -1)
				waitpid(pid, NULL, 0);
		}
	}

	closedir(dir);
}

void exec_module_script(const char* stage) {
	char *module;
	vec_for_each(&module_list, module) {
		snprintf(buf, PATH_MAX, "/magisk/%s/%s.sh", module, stage);
		if (access(buf, F_OK) == -1)
			continue;
		LOGI("%s: exec [%s.sh]\n", module, stage);
		char *const command[] = { "sh", buf, NULL };
		int pid = run_command(NULL, "/system/bin/sh", command);
		if (pid != -1)
			waitpid(pid, NULL, 0);
	}

}

/***************
 * Magic Mount *
 ***************/

static int hasChild(struct node_entry *node, const char *name) {
	struct node_entry *child;
	vec_for_each(node->children, child) {
		if (strcmp(child->name, name) == 0)
			return 1;
	}
	return 0;
}

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

static void destroy_subtree(struct node_entry *node) {
	// Never free parent, since it shall be freed by themselves
	free(node->name);
	struct node_entry *e;
	vec_for_each(node->children, e) {
		destroy_subtree(e);
	}
	vec_destroy(node->children);
	free(node->children);
	free(node);
}

static void insert_child(struct node_entry *p, struct node_entry *c) {
	c->parent = p;
	if (p->children == NULL) {
		p->children = xmalloc(sizeof(struct vector));
		vec_init(p->children);
	}
	struct node_entry *e;
	vec_for_each(p->children, e) {
		if (strcmp(e->name, c->name) == 0) {
			// Exist duplicate, replace
			destroy_subtree(e);
			vec_entry(p->children)[_] = c;
			return;
		}
	}
	// New entry, push back
	vec_push_back(p->children, c);
}

static void construct_tree(const char *module, const char *path, struct node_entry *parent) {
	DIR *dir;
	struct dirent *entry;
	struct node_entry *node;

	snprintf(buf, PATH_MAX, "/magisk/%s/%s", module, path);

	if (!(dir = xopendir(buf)))
		return;

	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		// Create new node
		node = xcalloc(sizeof(*node), 1);
		node->module = module;
		node->name = strdup(entry->d_name);
		node->type = entry->d_type;
		insert_child(parent, node);
		char *real_path = get_full_path(node);
		// Check if the entry has a correspond target
		if (entry->d_type == DT_LNK || access(real_path, F_OK) == -1) {
			// Mark the parent folder as a skeleton
			parent->status = IS_SKEL;
			node->status = IS_MODULE;
		} else {
			if (entry->d_type == DT_DIR) {
				// Check if marked as replace
				snprintf(buf, PATH_MAX, "/magisk/%s/%s/%s/.replace", module, path, entry->d_name);
				if (access(buf, F_OK) == 0) {
					// Replace everything, mark as leaf
					node->status = IS_MODULE;
				} else {
					// Travel deeper
					snprintf(buf, PATH_MAX, "%s/%s", path, entry->d_name);
					char *new_path = strdup(buf);
					construct_tree(module, new_path, node);
					free(new_path);
				}
			} else if (entry->d_type == DT_REG) {
				// This is a leaf, mark as target
				node->status = IS_MODULE;
			}
		}
		free(real_path);
	}
	
	closedir(dir);
}

static void clone_skeleton(struct node_entry *node, const char *real_path) {
	DIR *dir;
	struct dirent *entry;
	struct node_entry *dummy, *child;

	// Clone the structure
	dir = xopendir(real_path);
	while ((entry = xreaddir(dir))) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (!hasChild(node, entry->d_name)) {
			// Create dummy node
			dummy = xcalloc(sizeof(*dummy), 1);
			dummy->name = strdup(entry->d_name);
			dummy->type = entry->d_type;
			dummy->status = IS_DUMMY;
			insert_child(node, dummy);
		}
	}

	snprintf(buf, PATH_MAX, "/dev/magisk/dummy%s", real_path);
	xmkdir_p(buf, 0755);
	bind_mount(buf, real_path);

	vec_for_each(node->children, child) {
		snprintf(buf, PATH_MAX, "/dev/magisk/dummy%s/%s", real_path, child->name);
		if (child->type == DT_DIR) {
			xmkdir(buf, 0755);
		} else if (child->type == DT_REG) {
			close(open_new(buf));
		}
		if (child->status == IS_MODULE) {
			// Mount from module file to dummy file
			snprintf(buf2, PATH_MAX, "/magisk/%s%s/%s", child->module, real_path, child->name);
		} else if (child->status == IS_DUMMY) {
			// Mount from mirror to dummy file
			snprintf(buf2, PATH_MAX, "/dev/magisk/mirror%s/%s", real_path, child->name);
		} else if (child->status == IS_SKEL) {
			// It's another skeleton, recursive call and end
			char *s = get_full_path(child);
			clone_skeleton(child, s);
			free(s);
			continue;
		}
		if (child->type == DT_LNK) {
			// Symlink special treatments
			char *temp = xmalloc(PATH_MAX);
			xreadlink(buf2, temp, PATH_MAX);
			symlink(temp, buf);
			free(temp);
			LOGD("cplink: %s -> %s\n", buf2, buf);
		} else {
			snprintf(buf, PATH_MAX, "%s/%s", real_path, child->name);
			bind_mount(buf2, buf);
		}
	}
}

static void magic_mount(struct node_entry *node) {
	char *real_path;
	struct node_entry *child;

	if (strcmp(node->name, "vendor") == 0 && strcmp(node->parent->name, "/system") == 0)
		return;

	if (node->status == DO_NOTHING) {
		vec_for_each(node->children, child)
			magic_mount(child);
	} else {
		real_path = get_full_path(node);
		if (node->status == IS_MODULE) {
			snprintf(buf, PATH_MAX, "/magisk/%s%s", node->module, real_path);
			bind_mount(buf, real_path);
		} else if (node->status == IS_SKEL) {
			clone_skeleton(node, real_path);
		}
		free(real_path);
	}
}

/****************
 * Entry points *
 ****************/

static void *start_magisk_hide(void *args) {
	launch_magiskhide(-1);
	return NULL;
}

void post_fs(int client) {
	LOGI("** post-fs mode running\n");
	// ack
	write_int(client, 0);
	close(client);

	// TODO: Simple bind mounts

	// Allocate buffer
	buf = xmalloc(PATH_MAX);
	buf2 = xmalloc(PATH_MAX);

unblock:
	unblock_boot_process();
}

void post_fs_data(int client) {
	// ack
	write_int(client, 0);
	close(client);
	if (!check_data())
		goto unblock;

	// Allocate buffer
	if (buf == NULL) buf = xmalloc(PATH_MAX);
	if (buf2 == NULL) buf2 = xmalloc(PATH_MAX);

	LOGI("** post-fs-data mode running\n");
	LOGI("* Mounting magisk.img\n");
	// Mounting magisk image
	char *magiskimg = mount_image("/data/magisk.img", "/magisk");
	free(magiskimg);

	// Run common scripts
	LOGI("* Running post-fs-data.d scripts\n");
	exec_common_script("post-fs-data");

	DIR *dir;
	struct dirent *entry;
	char *module;
	struct node_entry *sys_root, *ven_root = NULL, *child;

	if (!(dir = xopendir("/magisk")))
		goto unblock;

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
			// Check whether disable
			snprintf(buf, PATH_MAX, "/magisk/%s/disable", entry->d_name);
			if (access(buf, F_OK) == 0)
				continue;
			// Add the module to list
			module = strdup(entry->d_name);
			vec_push_back(&module_list, module);
			// Read props
			snprintf(buf, PATH_MAX, "/magisk/%s/system.prop", module);
			if (access(buf, F_OK) == 0) {
				LOGI("%s: loading [system.prop]\n", module);
				read_prop_file(buf, 0);
			}
			// Check whether enable auto_mount
			snprintf(buf, PATH_MAX, "/magisk/%s/auto_mount", module);
			if (access(buf, F_OK) == -1)
				continue;
			// Double check whether the system folder exists
			snprintf(buf, PATH_MAX, "/magisk/%s/system", module);
			if (access(buf, F_OK) == -1)
				continue;
			// Construct structure
			has_modules = 1;
			LOGI("%s: constructing magic mount structure\n", module);
			construct_tree(module, "system", sys_root);
		}
	}

	closedir(dir);

	if (has_modules) {
		// Mount mirrors
		LOGI("* Mounting system/vendor mirrors");
		char block[256];
		int seperate_vendor = 0;
		struct vector mounts;
		vec_init(&mounts);
		file_to_vector("/proc/mounts", &mounts);
		char *line;
		vec_for_each(&mounts, line) {
			if (strstr(line, " /system ")) {
				sscanf(line, "%s", block);
				xmkdir_p("/dev/magisk/mirror/system", 0755);
				xmount(block, "/dev/magisk/mirror/system", "ext4", MS_RDONLY, NULL);
				LOGD("mount: %s -> /dev/magisk/mirror/system\n", block);
				continue;
			}
			if (strstr(line, " /vendor ")) {
				seperate_vendor = 1;
				sscanf(line, "%s", block);
				xmkdir_p("/dev/magisk/mirror/vendor", 0755);
				xmount(block, "/dev/magisk/mirror/vendor", "ext4", MS_RDONLY, NULL);
				LOGD("mount: %s -> /dev/magisk/mirror/vendor\n", block);
				continue;
			}
		}
		vec_deep_destroy(&mounts);
		if (!seperate_vendor) {
			symlink("/dev/magisk/mirror/system/vendor", "/dev/magisk/mirror/vendor");
			LOGD("link: /dev/magisk/mirror/system/vendor -> /dev/magisk/mirror/vendor\n");
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

	// Start magiskhide if enabled
	char *hide_prop = getprop("persist.magisk.hide");
	if (hide_prop) {
		if (strcmp(hide_prop, "1") == 0) {
			pthread_t thread;
			xpthread_create(&thread, NULL, start_magisk_hide, NULL);
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

	// Wait till the full patch is done
	pthread_join(sepol_patch, NULL);

	// Run scripts after full patch, most reliable way to run scripts
	LOGI("* Running service.d scripts\n");
	exec_common_script("service");
	LOGI("* Running module service scripts\n");
	exec_module_script("service");

	// All boot stage done, cleanup everything
	free(buf);
	free(buf2);
	vec_deep_destroy(&module_list);
}
