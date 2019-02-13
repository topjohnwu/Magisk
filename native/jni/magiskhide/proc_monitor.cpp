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
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <vector>
#include <string>
#include <unordered_map>

#include <magisk.h>
#include <utils.h>

#include "magiskhide.h"

using namespace std;

extern char *system_block, *vendor_block, *data_block;

static int inotify_fd = -1;

#define EVENT_SIZE	sizeof(struct inotify_event)
#define EVENT_BUF_LEN	(1024 * (EVENT_SIZE + 16))
#define __ALIGN_EVENT __attribute__ ((aligned(__alignof__(struct inotify_event))))

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

static inline uid_t get_uid(const int pid) {
	char path[16];
	struct stat st;

	sprintf(path, "/proc/%d", pid);
	if (stat(path, &st) == -1)
		return -1;

	return st.st_uid;
}

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
	LOGD("hide_daemon: handling pid=[%d]\n", pid);

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

/*
 * Bionic's atoi runs through strtol() and fault-tolerence checkings.
 * Since we don't need it, use our own implementation of atoi()
 * for faster conversion.
 */
static inline int fast_atoi(const char *str) {
	int val = 0;

	while (*str)
		val = val * 10 + (*str++ - '0');

	return val;
}

// Leave /proc fd opened as we're going to read from it repeatedly
static DIR *dfd;
// Use unordered map with pid and namespace inode number to avoid time-consuming GC
static unordered_map<int, uint64_t> pid_ns_map;

static void detect_new_processes() {
	struct dirent *dp;
	struct stat ns, pns;
	int pid, ppid;
	uid_t uid;

	// Iterate through /proc and get a process that reads the target APK
	rewinddir(dfd);
	pthread_mutex_lock(&list_lock);
	while ((dp = readdir(dfd))) {
		if (!isdigit(dp->d_name[0]))
			continue;

		// dp->d_name is now the pid
		pid = fast_atoi(dp->d_name);

		// We're only interested in PIDs > 1000
		if (pid <= 1000)
			continue;

		uid = get_uid(pid) % 100000; // Handle multiuser
		bool is_target = hide_uid.count(uid) != 0;
		if (is_target) {
			// Make sure our target is alive
			if ((ppid = parse_ppid(pid)) < 0 || read_ns(ppid, &pns) || read_ns(pid, &ns))
				continue;

			// Check if it's a process we haven't already hijacked
			auto pos = pid_ns_map.find(pid);
			if (pos == pid_ns_map.end() || pos->second != ns.st_ino) {
				pid_ns_map[pid] = ns.st_ino;
				if (uid == gms_uid) {
					// Check /proc/uid/cmdline to see if it's SAFETYNET_PROCESS
					if (!is_pid_safetynet_process(pid))
						continue;

					LOGI("proc_monitor: found %s\n", SAFETYNET_PROCESS);
				}

				// Send pause signal ASAP
				if (kill(pid, SIGSTOP) == -1)
					continue;

				/*
				 * The setns system call do not support multithread processes
				 * We have to fork a new process, setns, then do the unmounts
				 */
				LOGI("proc_monitor: UID=[%ju] PID=[%d] ns=[%llu]\n",
					 (uintmax_t)uid, pid, ns.st_ino);
				if (fork_dont_care() == 0)
					hide_daemon(pid);
			}
		}
	}
	pthread_mutex_unlock(&list_lock);
}

static void listdir_apk(const char *name) {
	DIR *dir;
	struct dirent *entry;
	const char *ext;
	char path[4096];

	if (!(dir = opendir(name)))
		return;

	while ((entry = readdir(dir)) != NULL) {
		snprintf(path, sizeof(path), "%s/%s", name,
			 entry->d_name);

		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0
			    || strcmp(entry->d_name, "..") == 0)
				continue;
			listdir_apk(path);
		} else {
			ext = &path[strlen(path) - 4];
			if (!strncmp(".apk", ext, 4)) {
				pthread_mutex_lock(&list_lock);
				for (auto &s : hide_list) {
					// Compare with (path + 10) to trim "/data/app/"
					if (strncmp(path + 10, s.c_str(), s.length()) == 0) {
						if (inotify_add_watch(inotify_fd, path, IN_OPEN | IN_DELETE) > 0) {
							LOGI("proc_monitor: Monitoring %s\n", path);
						} else {
							LOGE("proc_monitor: Failed to monitor %s: %s\n", path, strerror(errno));
						}
						break;
					}
				}
				pthread_mutex_unlock(&list_lock);
			}
		}
	}

	closedir(dir);
}

// Iterate through /data/app and search all .apk files
void update_inotify_mask() {
	// Setup inotify
	const char data_app[] = "/data/app";

	if (inotify_fd >= 0)
		close(inotify_fd);

	inotify_fd = inotify_init();
	if (inotify_fd < 0) {
		LOGE("proc_monitor: Cannot initialize inotify: %s\n", strerror(errno));
		term_thread(TERM_THREAD);
	}

	LOGI("proc_monitor: Updating APK list\n");
	listdir_apk(data_app);

	// Add /data/app itself to the watch list to detect app (un)installations/updates
	if (inotify_add_watch(inotify_fd, data_app, IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE) > 0) {
		LOGI("proc_monitor: Monitoring %s\n", data_app, inotify_fd);
	} else {
		LOGE("proc_monitor: Failed to monitor %s: %s\n", strerror(errno));
	}
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

	if (access("/proc/1/ns/mnt", F_OK) != 0) {
		LOGE("proc_monitor: Your kernel doesn't support mount namespace :(\n");
		term_thread(TERM_THREAD);
	}

	if ((dfd = opendir("/proc")) == NULL) {
		LOGE("proc_monitor: Unable to open /proc\n");
		term_thread(TERM_THREAD);
	}

	// Detect existing processes for the first time
	detect_new_processes();

	// Read inotify events
	struct inotify_event *event;
	ssize_t len;
	char *p;
	char buffer[EVENT_BUF_LEN] __ALIGN_EVENT;
	for (;;) {
		len = read(inotify_fd, buffer, EVENT_BUF_LEN);
		if (len == -1) {
			PLOGE("proc_monitor: read inotify");
			sleep(1);
			continue;
		}

		for (p = buffer; p < buffer + len; ) {
			event = (struct inotify_event *)p;

			if (event->mask & IN_OPEN) {
				// Since we're just watching files,
				// extracting file name is not possible from querying event
				// LOGI("proc_monitor: inotify: APK opened\n");
				detect_new_processes();
			} else {
				LOGI("proc_monitor: inotify: /data/app change detected\n");
				pthread_mutex_lock(&list_lock);
				refresh_uid();
				pthread_mutex_unlock(&list_lock);
				update_inotify_mask();
				break;
			}

			p += EVENT_SIZE + event->len;
		}
	}
}
