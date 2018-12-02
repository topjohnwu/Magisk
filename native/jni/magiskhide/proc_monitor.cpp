/* proc_monitor.cpp - Monitor am_proc_start events and unmount
 *
 * We monitor the logcat am_proc_start events. When a target starts up,
 * we pause it ASAP, and fork a new process to join its mount namespace
 * and do all the unmounting/mocking
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>

#include "magisk.h"
#include "daemon.h"
#include "utils.h"
#include "magiskhide.h"

static int sockfd = -1;
extern char *system_block, *vendor_block, *magiskloop;

// Workaround for the lack of pthread_cancel
static void term_thread(int) {
	LOGD("proc_monitor: running cleanup\n");
	hide_list.clear(true);
	hide_enabled = false;
	close(sockfd);
	sockfd = -1;
	pthread_mutex_destroy(&list_lock);
	LOGD("proc_monitor: terminating\n");
	pthread_exit(nullptr);
}

static int read_ns(const int pid, struct stat *st) {
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

static void hide_daemon(int pid) {
	LOGD("hide_daemon: handling pid=[%d]\n", pid);

	char buffer[4096];
	Vector<CharArray> mounts;

	manage_selinux();
	clean_magisk_props();

	if (switch_mnt_ns(pid))
		goto exit;

	snprintf(buffer, sizeof(buffer), "/proc/%d", pid);
	chdir(buffer);

	file_to_vector("mounts", mounts);
	// Unmount dummy skeletons and /sbin links
	for (auto &s : mounts) {
		if (s.contains("tmpfs /system/") || s.contains("tmpfs /vendor/") || s.contains("tmpfs /sbin")) {
			sscanf(s, "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
	}
	mounts.clear();

	// Re-read mount infos
	file_to_vector("mounts", mounts);

	// Unmount everything under /system, /vendor, and loop mounts
	for (auto &s : mounts) {
		if ((s.contains(" /system/") || s.contains(" /vendor/")) &&
			(s.contains(system_block) || s.contains(vendor_block) || s.contains(magiskloop))) {
			sscanf(s, "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
	}

exit:
	// Send resume signal
	kill(pid, SIGCONT);
	_exit(0);
}

void proc_monitor() {
	// Unblock user signals
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, TERM_THREAD);
	pthread_sigmask(SIG_UNBLOCK, &block_set, NULL);

	// Register the cancel signal
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = term_thread;
	sigaction(TERM_THREAD, &act, NULL);

	if (access("/proc/1/ns/mnt", F_OK) != 0) {
		LOGE("proc_monitor: Your kernel doesn't support mount namespace :(\n");
		term_thread(TERM_THREAD);
	}

	// Connect to the log daemon
	sockfd = connect_log_daemon();
	if (sockfd < 0)
		pthread_exit(nullptr);
	write_int(sockfd, HIDE_CONNECT);

	FILE *log_in = fdopen(sockfd, "r");
	char buf[4096];
	while (fgets(buf, sizeof(buf), log_in)) {
		char *log;
		int pid, ppid;
		struct stat ns, pns;

		if ((log = strchr(buf, '[')) == nullptr)
			continue;

		// Extract pid
		if (sscanf(log, "[%*d,%d", &pid) != 1)
			continue;

		// Extract last token (component name)
		const char *tok, *cpnt = "";
		while ((tok = strtok_r(nullptr, ",[]\n", &log)))
			cpnt = tok;
		if (cpnt[0] == '\0')
			continue;

		// Make sure our target is alive
		if ((ppid = parse_ppid(pid)) < 0 || read_ns(ppid, &pns))
			continue;

		bool hide = false;
		pthread_mutex_lock(&list_lock);
		for (auto &s : hide_list) {
			if (strncmp(cpnt, s, s.size() - 1) == 0) {
				hide = true;
				break;
			}
		}
		pthread_mutex_unlock(&list_lock);

		if (!hide)
			continue;

		while (read_ns(pid, &ns) == 0 && ns.st_dev == pns.st_dev && ns.st_ino == pns.st_ino)
			usleep(500);

		// Send pause signal ASAP
		if (kill(pid, SIGSTOP) == -1)
			continue;

		/*
		 * The setns system call do not support multithread processes
		 * We have to fork a new process, setns, then do the unmounts
		 */
		LOGI("proc_monitor: %s PID=[%d] ns=[%llu]\n", cpnt, pid, ns.st_ino);
		if (fork_dont_care() == 0)
			hide_daemon(pid);
	}
	pthread_exit(nullptr);
}
