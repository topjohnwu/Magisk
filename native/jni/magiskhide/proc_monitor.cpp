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
#include <sys/ptrace.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <set>

#include <magisk.h>
#include <utils.h>

#include "magiskhide.h"

using namespace std;

extern char *system_block, *vendor_block, *data_block;

static int inotify_fd = -1;

static void term_thread(int sig = SIGTERMTHRD);
static void new_zygote(int pid);

/************************
 * All the maps and sets
 ************************/

map<string, string> hide_map;                       /* process -> package_name */
static map<int, struct stat> zygote_map;            /* zygote pid -> mnt ns */
static map<int, vector<string_view>> uid_proc_map;  /* uid -> list of process */

pthread_mutex_t monitor_lock;

static set<int> attaches;   /* A set of pid that should be attached */
static set<int> detaches;   /* A set of tid that should be detached */
static set<int> unknown;    /* A set of pid/tid that is unknown */

/********
 * Utils
 ********/

static inline int read_ns(const int pid, struct stat *st) {
	char path[32];
	sprintf(path, "/proc/%d/ns/mnt", pid);
	return stat(path, st);
}

static inline void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		LOGD("hide_daemon: Unmounted (%s)\n", mountpoint);
}

static long xptrace(bool log, int request, pid_t pid, void *addr, void *data) {
	long ret = ptrace(request, pid, addr, data);
	if (log && ret == -1)
		PLOGE("ptrace %d", pid);
	return ret;
}

static long xptrace(int request, pid_t pid, void *addr, void *data) {
	return xptrace(true, request, pid, addr, data);
}

static long xptrace(int request, pid_t pid, void *addr = nullptr, intptr_t data = 0) {
	return xptrace(true, request, pid, addr, reinterpret_cast<void *>(data));
}

static bool parse_packages_xml(string_view &s) {
	if (!str_starts(s, "<package "))
		return true;
	/* <package key1="value1" key2="value2"....> */
	char *start = (char *) s.data();
	start[s.length() - 2] = '\0';  /* Remove trailing '>' */
	start += 9;  /* Skip '<package ' */

	char key[32], value[1024];
	char *pkg = nullptr;

	char *tok;
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

void *update_uid_map(void*) {
	LOGD("proc_monitor: Updating uid maps\n");
	MutexGuard lock(monitor_lock);
	uid_proc_map.clear();
	file_readline("/data/system/packages.xml", parse_packages_xml, true);
	return nullptr;
}

/*************************
 * The actual hide daemon
 **************************/

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

/************************
 * Async signal handlers
 ************************/

static void inotify_event(int) {
	/* Make sure we can actually read stuffs
	 * or else the whole thread will be blocked.*/
	struct pollfd pfd = {
			.fd = inotify_fd,
			.events = POLLIN,
			.revents = 0
	};
	if (poll(&pfd, 1, 0) <= 0)
		return;  // Nothing to read
	char buf[512];
	auto event = reinterpret_cast<struct inotify_event *>(buf);
	read(inotify_fd, buf, sizeof(buf));
	if ((event->mask & IN_CLOSE_WRITE) && strcmp(event->name, "packages.xml") == 0) {
		LOGD("proc_monitor: /data/system/packages.xml updated\n");
		// Use new thread to parse xml, don't block zygote tracing
		new_daemon_thread(update_uid_map);
	}
}

static void zygote_sig(int) {
	int pid;
	{
		MutexGuard lock(monitor_lock);
		pid = next_zygote;
		next_zygote = -1;
	}
	if (pid > 0)
		new_zygote(pid);
}

// Workaround for the lack of pthread_cancel
static void term_thread(int) {
	LOGD("proc_monitor: cleaning up\n");
	// Clear maps
	hide_map.clear();
	uid_proc_map.clear();
	zygote_map.clear();
	// Clear sets
	attaches.clear();
	detaches.clear();
	unknown.clear();
	// Misc
	hide_enabled = false;
	pthread_mutex_destroy(&monitor_lock);
	close(inotify_fd);
	inotify_fd = -1;
	LOGD("proc_monitor: terminate\n");
	pthread_exit(nullptr);
}

/******************
 * Ptrace Madness
 ******************/

/* Ptrace is super tricky, preserve all excessive debug in code
 * but disable when actually building for usage (you won't want
 * your logcat spammed with new thread events from all apps) */

//#define PTRACE_LOG(fmt, args...) LOGD("PID=[%d] " fmt, pid, ##args)
#define PTRACE_LOG(...)

static bool check_pid(int pid) {
	char path[128];
	char cmdline[1024];
	sprintf(path, "/proc/%d/cmdline", pid);
	FILE *f = fopen(path, "re");
	// Process killed unexpectedly, ignore
	if (!f) return true;
	fgets(cmdline, sizeof(cmdline), f);
	fclose(f);
	if (strncmp(cmdline, "zygote", 6) == 0)
		return false;

	/* This process is fully initialized, we will stop
	 * tracing it no matter if it is a target or not. */
	attaches.erase(pid);

	sprintf(path, "/proc/%d", pid);
	struct stat st;
	lstat(path, &st);
	int uid = st.st_uid % 100000;
	auto it = uid_proc_map.find(uid);
	if (it != uid_proc_map.end()) {
		for (auto &s : it->second) {
			if (s == cmdline) {
				// Double check whether ns is separated
				read_ns(pid, &st);
				bool mnt_ns = true;
				for (auto &zit : zygote_map) {
					if (zit.second.st_ino == st.st_ino &&
						zit.second.st_dev == st.st_dev) {
						mnt_ns = false;
						break;
					}
				}
				// For some reason ns is not separated, abort
				if (!mnt_ns)
					break;

				/* Finally this is our target!
				 * Detach from ptrace but should still remain stopped.
				 * The hide daemon will resume the process. */
				xptrace(PTRACE_DETACH, pid, nullptr, SIGSTOP);
				LOGI("proc_monitor: [%s] PID=[%d] UID=[%d]\n", cmdline, pid, uid);
				PTRACE_LOG("target found\n");
				if (fork_dont_care() == 0)
					hide_daemon(pid);
				return true;
			}
		}
	}
	PTRACE_LOG("not our target\n");
	xptrace(PTRACE_DETACH, pid);
	return true;
}

static void handle_unknown(int tid, int pid = -1) {
	if (unknown.count(tid)) {
		unknown.erase(tid);
		tgkill(pid < 0 ? tid : pid, tid, SIGSTOP);
	}
}

static void new_zygote(int pid) {
	if (zygote_map.count(pid))
		return;

	LOGD("proc_monitor: ptrace zygote PID=[%d]\n", pid);

	struct stat st;
	if (read_ns(pid, &st))
		return;
	zygote_map[pid] = st;

	xptrace(PTRACE_ATTACH, pid);

	waitpid(pid, nullptr, __WALL | __WNOTHREAD);
	xptrace(PTRACE_SETOPTIONS, pid, nullptr,
			PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACEEXIT);
	xptrace(PTRACE_CONT, pid);
}

void proc_monitor() {
	inotify_fd = xinotify_init1(IN_CLOEXEC);
	if (inotify_fd < 0)
		term_thread();

	// Unblock some signals
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGTERMTHRD);
	sigaddset(&block_set, SIGZYGOTE);
	sigaddset(&block_set, SIGIO);
	pthread_sigmask(SIG_UNBLOCK, &block_set, nullptr);

	struct sigaction act{};
	act.sa_handler = term_thread;
	sigaction(SIGTERMTHRD, &act, nullptr);
	act.sa_handler = zygote_sig;
	sigaction(SIGZYGOTE, &act, nullptr);
	act.sa_handler = inotify_event;
	sigaction(SIGIO, &act, nullptr);

	// Setup inotify asynchronous I/O
	fcntl(inotify_fd, F_SETFL, O_ASYNC);
	struct f_owner_ex ex = {
		.type = F_OWNER_TID,
		.pid = gettid()
	};
	fcntl(inotify_fd, F_SETOWN_EX, &ex);

	// Start monitoring packages.xml
	inotify_add_watch(inotify_fd, "/data/system", IN_CLOSE_WRITE);

	// First find existing zygotes
	crawl_procfs([](int pid) -> bool {
		char buf[512];
		snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);
		FILE *f = fopen(buf, "re");
		if (f) {
			fgets(buf, sizeof(buf), f);
			if (strncmp(buf, "zygote", 6) == 0)
				new_zygote(pid);
			fclose(f);
		}
		return true;
	});

	int status;

	for (;;) {
		int pid = waitpid(-1, &status, __WALL | __WNOTHREAD);
		if (pid < 0)
			continue;
		if (WIFSTOPPED(status)) {
			if (detaches.count(pid)) {
				PTRACE_LOG("detach\n");
				detaches.erase(pid);
				xptrace(PTRACE_DETACH, pid);
				continue;
			}
			if (WSTOPSIG(status) == SIGTRAP && WEVENT(status)) {
				unsigned long msg;
				xptrace(PTRACE_GETEVENTMSG, pid, nullptr, &msg);
				if (zygote_map.count(pid)) {
					// Zygote event
					switch (WEVENT(status)) {
						case PTRACE_EVENT_FORK:
						case PTRACE_EVENT_VFORK:
							PTRACE_LOG("zygote forked: [%d]\n", msg);
							attaches.insert(msg);
							handle_unknown(msg);
							break;
						case PTRACE_EVENT_EXIT:
							PTRACE_LOG("zygote exited with status: [%d]\n", msg);
							zygote_map.erase(pid);
							break;
						default:
							PTRACE_LOG("unknown event: %d\n", WEVENT(status));
							break;
					}
					xptrace(PTRACE_CONT, pid);
				} else {
					switch (WEVENT(status)) {
						case PTRACE_EVENT_CLONE:
							PTRACE_LOG("create new threads: [%d]\n", msg);
							detaches.insert(msg);
							handle_unknown(msg, pid);
							if (attaches.count(pid) && check_pid(pid))
								continue;
							break;
						case PTRACE_EVENT_EXIT:
							PTRACE_LOG("exited with status: [%d]\n", msg);
							attaches.erase(pid);
							unknown.erase(pid);
							break;
						default:
							PTRACE_LOG("unknown event: %d\n", WEVENT(status));
							break;
					}
					xptrace(PTRACE_CONT, pid);
				}
			} else if (WSTOPSIG(status) == SIGSTOP) {
				if (attaches.count(pid)) {
					PTRACE_LOG("SIGSTOP from zygote child\n");
					xptrace(PTRACE_SETOPTIONS, pid, nullptr, PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT);
				} else {
					PTRACE_LOG("SIGSTOP from unknown\n");
					unknown.insert(pid);
				}
				xptrace(PTRACE_CONT, pid);
			} else {
				// Not caused by us, resend signal
				xptrace(PTRACE_CONT, pid, nullptr, WSTOPSIG(status));
				PTRACE_LOG("signal [%d]\n", WSTOPSIG(status));
			}
		} else {
			// Nothing to do with us
			ptrace(PTRACE_DETACH, pid);
			PTRACE_LOG("terminate\n");
		}
	}
}
