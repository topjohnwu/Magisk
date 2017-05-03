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

static int get_img_size(const char *img, int *used, int *total) {
	char buffer[PATH_MAX];
	snprintf(buffer, sizeof(buffer), "e2fsck -n %s 2>/dev/null | tail -n 1", img);
	char *const command[] = { "sh", "-c", buffer, NULL };
	int pid, fd;
	pid = run_command(&fd, "/system/bin/sh", command);
	fdgets(buffer, sizeof(buffer), fd);
	close(fd);
	if (pid == -1)
		return 1;
	waitpid(pid, NULL, 0);
	char *tok;
	tok = strtok(buffer, ",");
	while(tok != NULL) {
		if (strstr(tok, "blocks"))
			break;
		tok = strtok(NULL, ",");
	}
	sscanf(tok, "%d/%d", used, total);
	*used = *used / 256 + 1;
	*total /= 256;
	return 0;
}

#define round_size(a) ((((a) / 32) + 1) * 32)

static int resize_img(const char *img, int size) {
	char buffer[ARG_MAX];
	LOGI("resize %s to %dM\n", img, size);
	snprintf(buffer, sizeof(buffer), "e2fsck -yf %s && resize2fs %s %dM;", img, img, size);
	return system(buffer);
}

static int merge_img(const char *source, const char *target) {
	if (access(source, F_OK) == -1)
		return 0;
	if (access(target, F_OK) == -1) {
		rename(source, target);
		return 0;
	}
	
	// resize target to worst case
	int s_used, s_total, t_used, t_total, n_total;
	get_img_size(source, &s_used, &s_total);
	get_img_size(target, &t_used, &t_total);
	n_total = round_size(s_used + t_used);
	if (n_total != t_total && resize_img(target, n_total))
		return 1;

	xmkdir("/cache/source", 0755);
	xmkdir("/cache/target", 0755);
	char *s_loop, *t_loop;
	s_loop = mount_image(source, "/cache/source");
	if (s_loop == NULL) return 1;
	t_loop = mount_image(target, "/cache/target");
	if (t_loop == NULL) return 1;

	DIR *dir;
	struct dirent *entry;
	if (!(dir = opendir("/cache/source")))
		return 1;
	while ((entry = xreaddir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0 ||
				strcmp(entry->d_name, ".core") == 0 ||
				strcmp(entry->d_name, "lost+found") == 0)
				continue;
			// Cleanup old module
			snprintf(buf, PATH_MAX, "/cache/target/%s", entry->d_name);
			if (access(buf, F_OK) == 0) {
				LOGI("merge module %s\n", entry->d_name);
				rm_rf(buf);
			}
		}
	}
	closedir(dir);
	clone_dir("/cache/source", "/cache/target");

	// Unmount all loop devices
	umount_image("/cache/source", s_loop);
	umount_image("/cache/target", t_loop);
	rmdir("/cache/source");
	rmdir("/cache/target");
	free(s_loop);
	free(t_loop);
	unlink(source);
	return 0;
}

static void trim_img(const char *img) {
	int used, total, new_size;
	get_img_size(img, &used, &total);
	new_size = round_size(used);
	if (new_size < total)
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
		snprintf(buf, PATH_MAX, "%s/%s/%s.sh", MOUNTPOINT, module, stage);
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

	snprintf(buf, PATH_MAX, "%s/%s/%s", MOUNTPOINT, module, path);

	if (!(dir = opendir(buf)))
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
				snprintf(buf2, PATH_MAX, "%s/%s/%s/%s/.replace", MOUNTPOINT, module, path, entry->d_name);
				if (access(buf2, F_OK) == 0) {
					// Replace everything, mark as leaf
					node->status = IS_MODULE;
				} else {
					// Travel deeper
					snprintf(buf2, PATH_MAX, "%s/%s", path, entry->d_name);
					char *new_path = strdup(buf2);
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
	if (!(dir = opendir(real_path)))
		return;
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
	closedir(dir);

	snprintf(buf, PATH_MAX, "%s%s", DUMMDIR, real_path);
	xmkdir_p(buf, 0755);
	bind_mount(buf, real_path);

	vec_for_each(node->children, child) {
		snprintf(buf, PATH_MAX, "%s%s/%s", DUMMDIR, real_path, child->name);
		if (child->type == DT_DIR) {
			xmkdir(buf, 0755);
		} else if (child->type == DT_REG) {
			close(open_new(buf));
		}
		if (child->status == IS_MODULE) {
			// Mount from module file to dummy file
			snprintf(buf2, PATH_MAX, "%s/%s%s/%s", MOUNTPOINT, child->module, real_path, child->name);
		} else if (child->status == IS_DUMMY) {
			// Mount from mirror to dummy file
			snprintf(buf2, PATH_MAX, "%s%s/%s", MIRRDIR, real_path, child->name);
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
			LOGI("cplink: %s -> %s\n", buf2, buf);
		} else {
			snprintf(buf, PATH_MAX, "%s/%s", real_path, child->name);
			bind_mount(buf2, buf);
		}
	}
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

	if (node->status == DO_NOTHING) {
		vec_for_each(node->children, child)
			magic_mount(child);
	} else {
		real_path = get_full_path(node);
		if (node->status == IS_MODULE) {
			snprintf(buf, PATH_MAX, "%s/%s%s", MOUNTPOINT, node->module, real_path);
			bind_mount(buf, real_path);
		} else if (node->status == IS_SKEL) {
			clone_skeleton(node, real_path);
		}
		free(real_path);
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
			struct stat s;
			xstat(buf2, &s);
			chmod(buf, s.st_mode & 0777);
			chown(buf, s.st_uid, s.st_gid);
			char *con;
			getfilecon(buf2, &con);
			setfilecon(buf, con);
			free(con);
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

	LOGI("** post-fs-data mode running\n");

	// uninstaller
	if (access(UNINSTALLER, F_OK) == 0) {
		close(open(UNBLOCKFILE, O_RDONLY | O_CREAT));
		char *const command[] = { "sh", UNBLOCKFILE, NULL };
		run_command(NULL, "/system/bin/sh", command);
		return;
	}

	// Allocate buffer
	if (buf == NULL) buf = xmalloc(PATH_MAX);
	if (buf2 == NULL) buf2 = xmalloc(PATH_MAX);

	// Cache support
	if (access("/cache/data_bin", F_OK) == 0) {
		rm_rf(DATABIN);
		rename("/cache/data_bin", DATABIN);
		system("mv /cache/stock_boot* /data");   // Lazy.... use bash glob....
	}

	// Merge images
	if (merge_img("/cache/magisk.img", MAINIMG))
		goto unblock;
	if (merge_img("/data/magisk_merge.img", MAINIMG))
		goto unblock;

	LOGI("* Mounting " MAINIMG "\n");
	// Mounting magisk image
	char *magiskloop = mount_image(MAINIMG, MOUNTPOINT);
	if (magiskloop == NULL)
		goto unblock;

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
			construct_tree(module, "system", sys_root);
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
