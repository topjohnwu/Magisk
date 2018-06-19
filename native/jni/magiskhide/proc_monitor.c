/* proc_monitor.c - Monitor am_proc_start events and unmount
 *
 * We monitor the logcat am_proc_start events. When a target starts up,
 * we pause it ASAP, and fork a new process to join its mount namespace
 * and do all the unmounting/mocking
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>

#include "magisk.h"
#include "utils.h"
#include "magiskhide.h"

static int pipefd[2] = { -1, -1 };

// Workaround for the lack of pthread_cancel
static void term_thread(int sig) {
	LOGD("proc_monitor: running cleanup\n");
	destroy_list();
	hideEnabled = 0;
	// Unregister listener
	log_events[HIDE_EVENT].fd = -1;
	close(pipefd[0]);
	close(pipefd[1]);
	pipefd[0] = pipefd[1] = -1;
	pthread_mutex_destroy(&hide_lock);
	pthread_mutex_destroy(&file_lock);
	LOGD("proc_monitor: terminating...\n");
	pthread_exit(NULL);
}

static int read_namespace(const int pid, char* target, const size_t size) {
	char path[32];
	sprintf(path, "/proc/%d/ns/mnt", pid);
	if (access(path, R_OK) == -1)
		return 1;
	xreadlink(path, target, size);
	return 0;
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
	xread(fd, stat, sizeof(stat));
	close(fd);
	/* PID COMM STATE PPID ..... */
	sscanf(stat, "%*d %*s %*c %d", &ppid);
	return ppid;
}

static void hide_daemon(int pid) {
	LOGD("hide_daemon: start unmount for pid=[%d]\n", pid);

	char *line, buffer[PATH_MAX];
	struct vector mount_list;

	manage_selinux();
	clean_magisk_props();

	if (switch_mnt_ns(pid))
		goto exit;

	snprintf(buffer, sizeof(buffer), "/proc/%d/mounts", pid);
	vec_init(&mount_list);
	file_to_vector(buffer, &mount_list);

	// Unmount dummy skeletons and /sbin links
	vec_for_each(&mount_list, line) {
		if (strstr(line, "tmpfs /system/") || strstr(line, "tmpfs /vendor/") || strstr(line, "tmpfs /sbin")) {
			sscanf(line, "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
		free(line);
	}
	vec_destroy(&mount_list);

	// Re-read mount infos
	snprintf(buffer, sizeof(buffer), "/proc/%d/mounts", pid);
	vec_init(&mount_list);
	file_to_vector(buffer, &mount_list);

	// Unmount everything under /system, /vendor, and loop mounts
	vec_for_each(&mount_list, line) {
		if (strstr(line, "/dev/block/loop") || strstr(line, " /system/") || strstr(line, " /vendor/")) {
			sscanf(line, "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
		free(line);
	}
	vec_destroy(&mount_list);

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

	// Register our listener to logcat monitor
	xpipe2(pipefd, O_CLOEXEC);
	log_events[HIDE_EVENT].fd = pipefd[1];

	for (char *log, *line;; free(log)) {
		if (read(pipefd[0], &log, sizeof(log)) != sizeof(log)) {
			/* It might be interrupted */
			log = NULL;
			continue;
		}
		char *ss = strchr(log, '[');
		int pid, ppid, ret, comma = 0;
		char *pos = ss, proc[256], ns[32], pns[32];

		while(1) {
			pos = strchr(pos, ',');
			if(pos == NULL)
				break;
			pos[0] = ' ';
			++comma;
		}

		if (comma == 6)
			ret = sscanf(ss, "[%*d %d %*d %*d %256s", &pid, proc);
		else
			ret = sscanf(ss, "[%*d %d %*d %256s", &pid, proc);

		if(ret != 2)
			continue;

		ppid = parse_ppid(pid);

		// Allow hiding sub-services of applications
		char *colon = strchr(proc, ':');
		if (colon)
			*colon = '\0';

		// Critical region
		pthread_mutex_lock(&hide_lock);
		vec_for_each(hide_list, line) {
			if (strcmp(proc, line) == 0) {
				read_namespace(ppid, pns, sizeof(pns));
				do {
					read_namespace(pid, ns, sizeof(ns));
					if (strcmp(ns, pns) == 0)
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
				LOGI("proc_monitor: %s (PID=[%d] ns=%s)(PPID=[%d] ns=%s)\n", proc, pid, ns + 4, ppid, pns + 4);
#else
				LOGI("proc_monitor: %s\n", proc);
#endif

				/*
				 * The setns system call do not support multithread processes
				 * We have to fork a new process, setns, then do the unmounts
				 */
				if (fork_dont_care() == 0)
					hide_daemon(pid);

				break;
			}
		}
		pthread_mutex_unlock(&hide_lock);
	}
}
