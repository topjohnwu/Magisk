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
#include <algorithm>

#include <magisk.h>
#include <utils.h>

#include "magiskhide.h"

using namespace std;

extern char *system_block, *vendor_block, *data_block;

static int inotify_fd = -1;
static void term_thread(int);

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

static void hide_daemon(int pid) {
	RunFinally fin([=]() -> void {
		// Send resume signal
		kill(pid, SIGCONT);
		_exit(0);
	});

	if (switch_mnt_ns(pid))
		return;

	LOGD("hide_daemon: handling PID=[%d]\n", pid);
	manage_selinux();
	clean_magisk_props();

	vector<string> targets;

	// Unmount dummy skeletons and /sbin links
	file_readline("/proc/self/mounts", [&](string_view &s) -> bool {
		if (str_contains(s, "tmpfs /system/") || str_contains(s, "tmpfs /vendor/") ||
			str_contains(s, "tmpfs /sbin")) {
			char *path = (char *) s.data();
			// Skip first token
			strtok_r(nullptr, " ", &path);
			targets.emplace_back(strtok_r(nullptr, " ", &path));
		}
		return true;
	});

	for (auto &s : targets)
		lazy_unmount(s.data());
	targets.clear();

	// Unmount everything under /system, /vendor, and data mounts
	file_readline("/proc/self/mounts", [&](string_view &s) -> bool {
		if ((str_contains(s, " /system/") || str_contains(s, " /vendor/")) &&
			(str_contains(s, system_block) || str_contains(s, vendor_block) ||
			 str_contains(s, data_block))) {
			char *path = (char *) s.data();
			// Skip first token
			strtok_r(nullptr, " ", &path);
			targets.emplace_back(strtok_r(nullptr, " ", &path));
		}
		return true;
	});

	for (auto &s : targets)
		lazy_unmount(s.data());
}

/********************
 * All the damn maps
 ********************/

map<string, string> hide_map;                       /* process -> package_name */
static map<int, uint64_t> pid_ns_map;               /* pid -> last ns inode */
static map<int, vector<string_view>> uid_proc_map;  /* uid -> list of process */

// All maps are protected by this lock
pthread_mutex_t map_lock;

static bool check_pid(int pid) {
	// We're only interested in PIDs > 1000
	if (pid <= 1000)
		return true;

	struct stat ns, pns;
	int ppid;
	int uid = get_uid(pid);
	auto it = uid_proc_map.find(uid);
	if (it != uid_proc_map.end()) {
		// Make sure we can read mount namespace
		if ((ppid = parse_ppid(pid)) < 0 || read_ns(pid, &ns) || read_ns(ppid, &pns))
			return true;
		// mount namespace is not separated, we only unmount once
		if (ns.st_dev == pns.st_dev && ns.st_ino == pns.st_ino)
			return true;

		// Check if it's a process we haven't already hijacked
		auto pos = pid_ns_map.find(pid);
		if (pos != pid_ns_map.end() && pos->second == ns.st_ino)
			return true;

		// Check whether process name match hide list
		const char *process = nullptr;
		for (auto &proc : it->second)
			if (proc_name_match(pid, proc.data()))
				process = proc.data();

		if (!process)
			return true;

		// Send pause signal ASAP
		if (kill(pid, SIGSTOP) == -1)
			return true;

		pid_ns_map[pid] = ns.st_ino;
		LOGI("proc_monitor: [%s] UID=[%d] PID=[%d] ns=[%llu]\n", process, uid, pid, ns.st_ino);

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

static bool parse_packages_xml(string_view &s) {
	static const string_view APK_EXT(".apk");
	if (!str_starts(s, "<package "))
		return true;
	/* <package key1="value1" key2="value2"....> */
	char *start = (char *) s.data();
	start[s.length() - 2] = '\0';  /* Remove trailing '>' */
	char key[32], value[1024];
	char *pkg = nullptr;
	char *tok;
	start += 9;  /* Skip '<package ' */
	while ((tok = strtok_r(nullptr, " ", &start))) {
		sscanf(tok, "%[^=]=\"%[^\"]", key, value);
		string_view key_view(key);
		string_view value_view(value);
		if (key_view == "name") {
			for (auto &hide : hide_map) {
				if (hide.second == value_view) {
					pkg = hide.second.data();
					break;
				}
			}
			if (!pkg)
				return true;
		} else if (key_view == "codePath") {
			if (ends_with(value_view, APK_EXT)) {
				// Directly add to inotify list
				xinotify_add_watch(inotify_fd, value, IN_OPEN);
			} else {
				DIR *dir = opendir(value);
				if (dir == nullptr)
					return true;
				struct dirent *entry;
				while ((entry = xreaddir(dir))) {
					if (ends_with(entry->d_name, APK_EXT)) {
						value[value_view.length()] = '/';
						strcpy(value + value_view.length() + 1, entry->d_name);
						xinotify_add_watch(inotify_fd, value, IN_OPEN);
						break;
					}
				}
				closedir(dir);
			}
		} else if (key_view == "userId" || key_view == "sharedUserId") {
			int uid = parse_int(value);
			for (auto &hide : hide_map) {
				if (hide.second == pkg)
					uid_proc_map[uid].emplace_back(hide.first);
			}
		}
	}
	return true;
}

void update_inotify_mask() {
	int new_inotify = xinotify_init1(IN_CLOEXEC);
	if (new_inotify < 0)
		term_thread(TERM_THREAD);

	// Swap out and close old inotify_fd
	int tmp = inotify_fd;
	inotify_fd = new_inotify;
	if (tmp >= 0)
		close(tmp);

	LOGD("proc_monitor: Updating inotify list\n");
	{
		MutexGuard lock(map_lock);
		uid_proc_map.clear();
		file_readline("/data/system/packages.xml", parse_packages_xml, true);
	}
	xinotify_add_watch(inotify_fd, "/data/system", IN_CLOSE_WRITE);
}

// Workaround for the lack of pthread_cancel
static void term_thread(int) {
	LOGD("proc_monitor: cleaning up\n");
	hide_map.clear();
	uid_proc_map.clear();
	pid_ns_map.clear();
	hide_enabled = false;
	pthread_mutex_destroy(&map_lock);
	close(inotify_fd);
	inotify_fd = -1;
	LOGD("proc_monitor: terminate\n");
	pthread_exit(nullptr);
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
	ssize_t len;
	char buf[512];
	auto event = reinterpret_cast<inotify_event *>(buf);
	while ((len = read(inotify_fd, buf, sizeof(buf))) >= 0) {
		if (len < sizeof(*event))
			continue;
		if (event->mask & IN_OPEN) {
			// Since we're just watching files,
			// extracting file name is not possible from querying event
			MutexGuard lock(map_lock);
			crawl_procfs(check_pid);
		} else if ((event->mask & IN_CLOSE_WRITE) && strcmp(event->name, "packages.xml") == 0) {
			LOGD("proc_monitor: /data/system/packages.xml updated\n");
			update_inotify_mask();
		}
	}
	PLOGE("proc_monitor: read inotify");
}
