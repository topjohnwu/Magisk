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
static set<int> hide_uid;

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

static bool is_snet(const int pid) {
	char path[32];
	char buf[64];
	int fd;
	ssize_t len;

	sprintf(path, "/proc/%d/cmdline", pid);
	fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd == -1)
		return false;

	len = read(fd, buf, sizeof(buf));
	close(fd);
	if (len == -1)
		return false;

	return !strcmp(buf, SAFETYNET_PROCESS);
}

static void hide_daemon(int pid) {
	char buffer[4096];
	if (switch_mnt_ns(pid))
		goto exit;

	LOGD("hide_daemon: handling PID=[%d]\n", pid);
	manage_selinux();
	clean_magisk_props();
	snprintf(buffer, sizeof(buffer), "/proc/%d", pid);
	chdir(buffer);

	// Unmount dummy skeletons and /sbin links
	file_readline("mounts", [&](string_view &s) -> bool {
		if (str_contains(s, "tmpfs /system/") || str_contains(s, "tmpfs /vendor/") ||
			str_contains(s, "tmpfs /sbin")) {
			sscanf(s.data(), "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
		return true;
	});

	// Unmount everything under /system, /vendor, and data mounts
	file_readline("mounts", [&](string_view &s) -> bool {
		if ((str_contains(s, " /system/") || str_contains(s, " /vendor/")) &&
			(str_contains(s, system_block) || str_contains(s, vendor_block) ||
			 str_contains(s, data_block))) {
			sscanf(s.data(), "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
		return true;
	});

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
			if (!is_snet(pid))
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

static int new_inotify;
static const string_view APK_EXT(".apk");
static vector<string> hide_apks;

static bool parse_packages_xml(string_view &s) {
	if (!str_starts(s, "<package "))
		return true;
	/* <package key1="value1" key2="value2"....> */
	char *start = (char *) s.data();
	start[s.length() - 2] = '\0';  /* Remove trailing '>' */
	char key[32], value[1024];
	char *tok;
	start += 9;  /* Skip '<package ' */
	while ((tok = strtok_r(nullptr, " ", &start))) {
		sscanf(tok, "%[^=]=\"%[^\"]", key, value);
		string_view value_view(value);
		if (strcmp(key, "name") == 0) {
			if (std::count(hide_list.begin(), hide_list.end(), value_view) == 0)
				return true;
		} else if (strcmp(key, "codePath") == 0) {
			if (ends_with(value_view, APK_EXT)) {
				// Directly add to inotify list
				hide_apks.emplace_back(value);
			} else {
				DIR *dir = opendir(value);
				if (dir == nullptr)
					return true;
				struct dirent *entry;
				while ((entry = xreaddir(dir))) {
					if (ends_with(entry->d_name, APK_EXT)) {
						strcpy(value + value_view.length(), "/");
						strcpy(value + value_view.length() + 1, entry->d_name);
						hide_apks.emplace_back(value);
						break;
					}
				}
				closedir(dir);
			}
		} else if (strcmp(key, "userId") == 0 || strcmp(key, "sharedUserId") == 0) {
			hide_uid.insert(parse_int(value));
		}
	}
	return true;
}

void update_inotify_mask() {
	new_inotify = inotify_init();
	if (new_inotify < 0) {
		LOGE("proc_monitor: Cannot initialize inotify: %s\n", strerror(errno));
		term_thread(TERM_THREAD);
	}
	fcntl(new_inotify, F_SETFD, FD_CLOEXEC);

	LOGD("proc_monitor: Updating inotify list\n");
	hide_apks.clear();
	{
		MutexGuard lock(list_lock);
		hide_uid.clear();
		file_readline("/data/system/packages.xml", parse_packages_xml, true);
	}

	// Swap out and close old inotify_fd
	int tmp = inotify_fd;
	inotify_fd = new_inotify;
	if (tmp >= 0)
		close(tmp);

	for (auto apk : hide_apks)
		xinotify_add_watch(inotify_fd, apk.data(), IN_OPEN);
	xinotify_add_watch(inotify_fd, "/data/system", IN_CLOSE_WRITE);
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
			MutexGuard lock(list_lock);
			crawl_procfs(process_pid);
		} else if ((event->mask & IN_CLOSE_WRITE) && strcmp(event->name, "packages.xml") == 0) {
			LOGD("proc_monitor: /data/system/packages.xml updated\n");
			update_inotify_mask();
		}
	}
	PLOGE("proc_monitor: read inotify");
}
