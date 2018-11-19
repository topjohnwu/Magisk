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
#include "flags.h"

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

static void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		LOGD("hide_daemon: Unmounted (%s)\n", mountpoint);
}

static int parse_ppid(int pid) {
	char stat[512], path[32];
	int fd, ppid;
	sprintf(path, "/proc/%d/stat", pid);
	fd = xopen(path, O_RDONLY);
	if (fd < 0)
		return -1;
	xread(fd, stat, sizeof(stat));
	close(fd);
	/* PID COMM STATE PPID ..... */
	sscanf(stat, "%*d %*s %*c %d", &ppid);
	return ppid;
}

static void hide_daemon(int pid) {
	LOGD("hide_daemon: start unmount for pid=[%d]\n", pid);

	char buffer[PATH_MAX];
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
		return;
	write_int(sockfd, HIDE_CONNECT);

	FILE *log_in = fdopen(sockfd, "r");
	char buf[4096];
	while (fgets(buf, sizeof(buf), log_in)) {
		char *ss = strchr(buf, '[');
		int pid, ppid, num = 0;
		char *pos = ss, proc[256];
		struct stat ns, pns;

		while(1) {
			pos = strchr(pos, ',');
			if(pos == NULL)
				break;
			pos[0] = ' ';
			++num;
		}

		if(sscanf(ss, num == 6 ? "[%*d %d %*d %*d %256s" : "[%*d %d %*d %256s", &pid, proc) != 2)
			continue;

		// Make sure our target is alive
		if (kill(pid, 0))
			continue;

		// Allow hiding sub-services of applications
		char *colon = strchr(proc, ':');
		if (colon)
			*colon = '\0';

		bool hide = false;
		pthread_mutex_lock(&list_lock);
		for (auto &s : hide_list) {
			if (s == proc) {
				hide = true;
				break;
			}
		}
		pthread_mutex_unlock(&list_lock);

		if (!hide || (ppid = parse_ppid(pid)) < 0 || read_ns(ppid, &pns) == -1)
			continue;

		do {
			if (read_ns(pid, &ns) == -1)
				break;
			if (ns.st_dev == pns.st_dev && ns.st_ino == pns.st_ino)
				usleep(50);
			else
				break;
		} while (1);

		// Send pause signal ASAP
		if (kill(pid, SIGSTOP) == -1)
			continue;

		// Restore the colon so we can log the actual process name
		if (colon)
			*colon = ':';
#ifdef MAGISK_DEBUG
		LOGI("proc_monitor: %s (PID=[%d] ns=%llu)(PPID=[%d] ns=%llu)\n",
			 proc, pid, ns.st_ino, ppid, pns.st_ino);
#else
		LOGI("proc_monitor: %s\n", proc);
#endif

		/*
		 * The setns system call do not support multithread processes
		 * We have to fork a new process, setns, then do the unmounts
		 */
		if (fork_dont_care() == 0)
			hide_daemon(pid);
	}
	pthread_exit(nullptr);
}
