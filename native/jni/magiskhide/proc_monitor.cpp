/* proc_monitor.cpp - Monitor am_proc_start events and unmount
 *
 * We monitor the listed APK files from /data/app until they get opened
 * via inotify to detect a new app launch.
 *
 * If it's a target we pause it ASAP, and fork a new process to join
 * its mount namespace and do all the unmounting/mocking.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <vector>
#include <string>
#include <map>

#include <magisk.h>
#include <utils.h>

#include "magiskhide.h"

using namespace std;

extern char *system_block, *vendor_block, *data_block;

static int inotify_fd = -1;

// Workaround for the lack of pthread_cancel
static void term_thread(int) {
	LOGD("proc_monitor: running cleanup\n");
	hide_list.clear();
	hide_uid.clear();
	hide_enabled = false;
	pthread_mutex_destroy(&list_lock);
	close(inotify_fd);
	inotify_fd = -1;
	LOGD("proc_monitor: terminating\n");
	pthread_exit(nullptr);
}

static inline int read_ns(const int pid, struct stat *st) {
	char path[32];
	sprintf(path, "/proc/%d/ns/mnt", pid);
	return stat(path, st);
}

static inline void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		LOGD("hide_daemon: Unmounted (%s)\n", mountpoint);
}

/* APK monitoring doesn't seem to require checking namespace
 * separation from PPID. Preserve this function just in case */
#if 0
static inline int parse_ppid(const int pid) {
	char path[32];
	int ppid;

	sprintf(path, "/proc/%d/stat", pid);
	FILE *stat = fopen(path, "re");
	if (stat == nullptr)
		return -1;

	/* PID COMM STATE PPID ..... */
	fscanf(stat, "%*d %*s %*c %d", &ppid);
	fclose(stat);

	return ppid;
}
#endif

static bool is_pid_safetynet_process(const int pid) {
	char path[32];
	char buf[64];
	int fd;
	ssize_t len;

	sprintf(path, "/proc/%d/cmdline", pid);
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return false;

	len = read(fd, buf, sizeof(buf));
	close(fd);
	if (len == -1)
		return false;

	return !strcmp(buf, SAFETYNET_PROCESS);
}

static void hide_daemon(int pid) {
	LOGD("hide_daemon: handling PID=[%d]\n", pid);

	char buffer[4096];
	vector<string> mounts;

	manage_selinux();
	clean_magisk_props();

	if (switch_mnt_ns(pid))
		goto exit;

	snprintf(buffer, sizeof(buffer), "/proc/%d", pid);
	chdir(buffer);

	mounts = file_to_vector("mounts");
	// Unmount dummy skeletons and /sbin links
	for (auto &s : mounts) {
		if (str_contains(s, "tmpfs /system/") || str_contains(s, "tmpfs /vendor/") ||
			str_contains(s, "tmpfs /sbin")) {
			sscanf(s.c_str(), "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
	}

	// Re-read mount infos
	mounts = file_to_vector("mounts");

	// Unmount everything under /system, /vendor, and data mounts
	for (auto &s : mounts) {
		if ((str_contains(s, " /system/") || str_contains(s, " /vendor/")) &&
			(str_contains(s, system_block) || str_contains(s, vendor_block) || str_contains(s, data_block))) {
			sscanf(s.c_str(), "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
	}

exit:
	// Send resume signal
	kill(pid, SIGCONT);
	_exit(0);
}

// A mapping from pid to namespace inode to avoid time-consuming GC
static map<int, uint64_t> pid_ns_map;

static bool process_pid(int pid) {
	// We're only interested in PIDs > 1000
	if (pid <= 1000)
		return true;

	struct stat ns;
	int uid = get_uid(pid);
	if (hide_uid.count(uid)) {
		// Make sure we can read mount namespace
		if (read_ns(pid, &ns))
			return true;

		// Check if it's a process we haven't already hijacked
		auto pos = pid_ns_map.find(pid);
		if (pos != pid_ns_map.end() && pos->second == ns.st_ino)
			return true;

		if (uid == gms_uid) {
			// Check /proc/uid/cmdline to see if it's SAFETYNET_PROCESS
			if (!is_pid_safetynet_process(pid))
				return true;

			LOGD("proc_monitor: " SAFETYNET_PROCESS "\n");
		}

		// Send pause signal ASAP
		if (kill(pid, SIGSTOP) == -1)
			return true;

		pid_ns_map[pid] = ns.st_ino;
		LOGI("proc_monitor: UID=[%d] PID=[%d] ns=[%llu]\n", uid, pid, ns.st_ino);

		/*
		 * The setns system call do not support multithread processes
		 * We have to fork a new process, setns, then do the unmounts
		 */
		if (fork_dont_care() == 0)
			hide_daemon(pid);
	}
	return true;
}

static int xinotify_add_watch(int fd, const char* path, uint32_t mask) {
	int ret = inotify_add_watch(fd, path, mask);
	if (ret >= 0) {
		LOGD("proc_monitor: Monitoring %s\n", path);
	} else {
		PLOGE("proc_monitor: Monitor %s", path);
	}
	return ret;
}

static char *append_path(char *eof, const char *name) {
	*(eof++) = '/';
	char c;
	while ((c = *(name++)))
		*(eof++) = c;
	*eof = '\0';
	return eof;
}

#define DATA_APP "/data/app"
static int new_inotify;
static int data_app_wd;
static vector<bool> app_in_data;
static void find_apks(char *path, char *eof) {
	DIR *dir = opendir(path);
	if (dir == nullptr)
		return;

	struct dirent *entry;
	char *dash;
	for (; (entry = xreaddir(dir)); *eof = '\0') {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (entry->d_type == DT_DIR) {
			find_apks(path, append_path(eof, entry->d_name));
		} else if (strend(entry->d_name, ".apk") == 0) {
			append_path(eof, entry->d_name);
			/* Supported path will be in either format:
			 * /data/app/[pkg]-[hash or 1 or 2]/base.apk
			 * /data/app/[pkg]-[1 or 2].apk */
			if ((dash = strchr(path, '-')) == nullptr)
				continue;
			*dash = '\0';
			for (int i = 0; i < hide_list.size(); ++i) {
				if (hide_list[i] == path + sizeof(DATA_APP)) {
					*dash = '-';
					append_path(eof, entry->d_name);
					xinotify_add_watch(new_inotify, path, IN_OPEN | IN_DELETE);
					app_in_data[i] = true;
					break;
				}
			}
			*dash = '-';
			break;
		}
	}
	closedir(dir);
}

// Iterate through /data/app and search all .apk files
void update_inotify_mask(bool refresh) {
	char buf[4096];

	new_inotify = inotify_init();
	if (new_inotify < 0) {
		LOGE("proc_monitor: Cannot initialize inotify: %s\n", strerror(errno));
		term_thread(TERM_THREAD);
	}

	LOGD("proc_monitor: Updating inotify list\n");
	strcpy(buf, DATA_APP);
	app_in_data.clear();
	bool reinstall = false;
	{
		MutexGuard lock(list_lock);
		app_in_data.resize(hide_list.size(), false);
		find_apks(buf, buf + sizeof(DATA_APP) - 1);
		// Stop monitoring /data/app
		if (inotify_fd >= 0)
			inotify_rm_watch(inotify_fd, data_app_wd);
		// All apps on the hide list should be installed in data
		auto it = hide_list.begin();
		for (bool in_data : app_in_data) {
			if (!in_data) {
				if (reinstall_apk(it->c_str()) != 0) {
					// Reinstallation failed, remove from hide list
					hide_list.erase(it);
					refresh = true;
					continue;
				}
				reinstall = true;
			}
			it++;
		}
		if (refresh && !reinstall)
			refresh_uid();
	}
	if (reinstall) {
		// Rerun detection
		close(new_inotify);
		update_inotify_mask(refresh);
		return;
	}

	// Add /data/app itself to the watch list to detect app (un)installations/updates
	data_app_wd = xinotify_add_watch(new_inotify, DATA_APP, IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE);

	int tmp = inotify_fd;
	inotify_fd = new_inotify;
	if (tmp >= 0)
		close(tmp);
}

void proc_monitor() {
	// Unblock user signals
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, TERM_THREAD);
	pthread_sigmask(SIG_UNBLOCK, &block_set, nullptr);

	// Register the cancel signal
	struct sigaction act{};
	act.sa_handler = term_thread;
	sigaction(TERM_THREAD, &act, nullptr);

	// Read inotify events
	struct inotify_event *event;
	ssize_t len;
	char *p;
	char buf[4096];
	while ((len = read(inotify_fd, buf, sizeof(buf))) >= 0) {
		for (p = buf; p < buf + len; ) {
			event = (struct inotify_event *)p;

			if (event->mask & IN_OPEN) {
				// Since we're just watching files,
				// extracting file name is not possible from querying event
				MutexGuard lock(list_lock);
				crawl_procfs(process_pid);
			} else if (!(event->mask & IN_IGNORED)) {
				LOGD("proc_monitor: inotify: /data/app change detected\n");
				update_inotify_mask(true);
				break;
			}

			p += sizeof(*event) + event->len;
		}
	}
	PLOGE("proc_monitor: read inotify");
}
