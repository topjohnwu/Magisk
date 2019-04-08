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
#include <vector>

#include <magisk.h>
#include <utils.h>

#include "magiskhide.h"

using namespace std;

static int inotify_fd = -1;

static void term_thread(int sig = SIGTERMTHRD);
static void new_zygote(int pid);

/**********************
 * All data structures
 **********************/

set<pair<string, string>> hide_set;                 /* set of <pkg, process> pair */
static map<int, struct stat> zygote_map;            /* zygote pid -> mnt ns */
static map<int, vector<string_view>> uid_proc_map;  /* uid -> list of process */

pthread_mutex_t monitor_lock;

#define PID_MAX 32768
static vector<bool> attaches(PID_MAX);  /* true if pid is monitored */
static vector<bool> detaches(PID_MAX);  /* true if tid should be detached */

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

static int parse_ppid(int pid) {
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

static bool parse_packages_xml(string_view s) {
	if (!str_starts(s, "<package "))
		return true;
	/* <package key1="value1" key2="value2"....> */
	char *start = (char *) s.data();
	start[s.length() - 1] = '\0';  /* Remove trailing '>' */
	start += 9;  /* Skip '<package ' */

	string_view pkg;
	for (char *tok = start; *tok;) {
		char *eql = strchr(tok, '=');
		if (eql == nullptr)
			break;
		*eql = '\0';  /* Terminate '=' */
		string_view key(tok, eql - tok);
		eql += 2;  /* Skip '="' */
		tok = strchr(eql, '\"');  /* Find closing '"' */
		*tok = '\0';
		string_view value(eql, tok - eql);
		tok += 2;
		if (key == "name") {
			for (auto &hide : hide_set) {
				if (hide.first == value) {
					pkg = hide.first;
					break;
				}
			}
			if (pkg.empty())
				return true;
		} else if (key == "userId" || key == "sharedUserId") {
			int uid = parse_int(value);
			for (auto &hide : hide_set) {
				if (hide.first == pkg)
					uid_proc_map[uid].emplace_back(hide.second);
			}
		}
	}
	return true;
}

static void check_zygote() {
	int min_zyg = 1;
	if (access("/system/bin/app_process64", R_OK) == 0)
		min_zyg = 2;
	for (bool first = true; zygote_map.size() < min_zyg; first = false) {
		if (!first)
			usleep(10000);
		crawl_procfs([](int pid) -> bool {
			char buf[512];
			snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);
			FILE *f = fopen(buf, "re");
			if (f) {
				fgets(buf, sizeof(buf), f);
				if (strncmp(buf, "zygote", 6) == 0 && parse_ppid(pid) == 1)
					new_zygote(pid);
				fclose(f);
			}
			return true;
		});
	}
}

void *update_uid_map(void*) {
	MutexGuard lock(monitor_lock);
	uid_proc_map.clear();
	file_readline("/data/system/packages.xml", parse_packages_xml, true);
	return nullptr;
}

/*************************
 * The actual hide daemon
 *************************/

static void hide_daemon(int pid) {
	RunFinally fin([=]() -> void {
		// Send resume signal
		tgkill(pid, pid, SIGCONT);
		_exit(0);
	});

	if (switch_mnt_ns(pid))
		return;

	LOGD("hide_daemon: handling PID=[%d]\n", pid);
	manage_selinux();
	clean_magisk_props();

	vector<string> targets;

	// Unmount dummy skeletons and /sbin links
	file_readline("/proc/self/mounts", [&](string_view s) -> bool {
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

	// Unmount all Magisk created mounts
	file_readline("/proc/self/mounts", [&](string_view s) -> bool {
		if (str_contains(s, BLOCKDIR)) {
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
		new_daemon_thread(update_uid_map);
	}
}

// Workaround for the lack of pthread_cancel
static void term_thread(int) {
	LOGD("proc_monitor: cleaning up\n");
	uid_proc_map.clear();
	zygote_map.clear();
	hide_set.clear();
	std::fill(attaches.begin(), attaches.end(), false);
	std::fill(detaches.begin(), detaches.end(), false);
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

static void detach_pid(int pid, int signal = 0) {
	char path[128];
	xptrace(PTRACE_DETACH, pid, nullptr, signal);

	// Detach all child threads too
	sprintf(path, "/proc/%d/task", pid);
	DIR *dir = opendir(path);
	crawl_procfs(dir, [&](int tid) -> bool {
		if (tid != pid) {
			// Check if we should force a SIGSTOP
			if (waitpid(tid, nullptr, __WALL | __WNOTHREAD | WNOHANG) == tid) {
				PTRACE_LOG("detach thread [%d]\n", tid);
				xptrace(PTRACE_DETACH, tid);
			} else {
				detaches[tid] = true;
				tgkill(pid, tid, SIGSTOP);
			}
		}
		return true;
	});
	closedir(dir);
}

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
	attaches[pid] = false;

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
				PTRACE_LOG("target found\n");
				LOGI("proc_monitor: [%s] PID=[%d] UID=[%d]\n", cmdline, pid, uid);
				detach_pid(pid, SIGSTOP);
				if (fork_dont_care() == 0)
					hide_daemon(pid);
				return true;
			}
		}
	}
	PTRACE_LOG("not our target\n");
	detach_pid(pid);
	return true;
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

#define DETACH_AND_CONT { detach = true; continue; }
void proc_monitor() {
	inotify_fd = xinotify_init1(IN_CLOEXEC);
	if (inotify_fd < 0)
		term_thread();

	// Unblock some signals
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGTERMTHRD);
	sigaddset(&block_set, SIGIO);
	pthread_sigmask(SIG_UNBLOCK, &block_set, nullptr);

	struct sigaction act{};
	act.sa_handler = term_thread;
	sigaction(SIGTERMTHRD, &act, nullptr);
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
	check_zygote();

	int status;

	for (;;) {
		const int pid = waitpid(-1, &status, __WALL | __WNOTHREAD);
		if (pid < 0)
			continue;
		bool detach = false;
		RunFinally detach_task([&]() -> void {
			if (detach) {
				// Non of our business now
				attaches[pid] = false;
				detaches[pid] = false;
				ptrace(PTRACE_DETACH, pid, 0, 0);
			}
		});
		if (!WIFSTOPPED(status) || detaches[pid]) {
			PTRACE_LOG("detached\n");
			DETACH_AND_CONT;
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
						attaches[msg] = true;
						break;
					case PTRACE_EVENT_EXIT:
						PTRACE_LOG("zygote exited with status: [%d]\n", msg);
						zygote_map.erase(pid);
						DETACH_AND_CONT;
					default:
						PTRACE_LOG("unknown event: %d\n", WEVENT(status));
						break;
				}
				xptrace(PTRACE_CONT, pid);
			} else {
				switch (WEVENT(status)) {
					case PTRACE_EVENT_CLONE:
						PTRACE_LOG("create new threads: [%d]\n", msg);
						if (attaches[pid] && check_pid(pid))
							continue;
						break;
					case PTRACE_EVENT_EXEC:
					case PTRACE_EVENT_EXIT:
						PTRACE_LOG("exited or execve\n");
						DETACH_AND_CONT;
					default:
						PTRACE_LOG("unknown event: %d\n", WEVENT(status));
						break;
				}
				xptrace(PTRACE_CONT, pid);
			}
		} else if (WSTOPSIG(status) == SIGSTOP) {
			PTRACE_LOG("SIGSTOP from child\n");
			xptrace(PTRACE_SETOPTIONS, pid, nullptr,
					PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT);
			xptrace(PTRACE_CONT, pid);
		} else {
			// Not caused by us, resend signal
			xptrace(PTRACE_CONT, pid, nullptr, WSTOPSIG(status));
			PTRACE_LOG("signal [%d]\n", WSTOPSIG(status));
		}
	}
}
