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

static char init_ns[32], zygote_ns[2][32], cache_block[256];
static int zygote_num, has_cache = 1, pipefd[2] = { -1, -1 };

// Workaround for the lack of pthread_cancel
static void quit_pthread(int sig) {
	LOGD("proc_monitor: running cleanup\n");
	destroy_list();
	hideEnabled = 0;
	// Unregister listener
	logcat_events[HIDE_EVENT] = -1;
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
	snprintf(path, sizeof(path), "/proc/%d/ns/mnt", pid);
	if (access(path, R_OK) == -1)
		return 1;
	xreadlink(path, target, size);
	return 0;
}

static void store_zygote_ns(int pid) {
	if (zygote_num == 2) return;
	do {
		usleep(500);
		read_namespace(pid, zygote_ns[zygote_num], 32);
	} while (strcmp(zygote_ns[zygote_num], init_ns) == 0);
	++zygote_num;
}

static void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		LOGD("hide_daemon: Unmounted (%s)\n", mountpoint);
	else
		LOGD("hide_daemon: Unmount Failed (%s)\n", mountpoint);
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

	// Find the cache block name if not found yet
	if (has_cache && cache_block[0] == '\0') {
		vec_for_each(&mount_list, line) {
			if (strstr(line, " /cache ")) {
				sscanf(line, "%256s", cache_block);
				break;
			}
		}
		if (strlen(cache_block) == 0)
			has_cache = 0;
	}

	// Unmout cache mounts
	if (has_cache) {
		vec_for_each(&mount_list, line) {
			if (strstr(line, cache_block) && (strstr(line, " /system/") || strstr(line, " /vendor/"))) {
				sscanf(line, "%*s %4096s", buffer);
				lazy_unmount(buffer);
			}
		}
	}

	// Unmount dummy skeletons, /sbin links, and mirrors
	vec_for_each(&mount_list, line) {
		if (strstr(line, "tmpfs /system") || strstr(line, "tmpfs /vendor") || strstr(line, "tmpfs /sbin") || strstr(line, MIRRDIR)) {
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

	// Unmount any loop mounts
	vec_for_each(&mount_list, line) {
		if (strstr(line, "/dev/block/loop")) {
			sscanf(line, "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
		free(line);
	}

exit:
	// Send resume signal
	kill(pid, SIGCONT);
	// Free up memory
	vec_destroy(&mount_list);
	_exit(0);
}

void proc_monitor() {
	// Register the cancel signal
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = quit_pthread;
	sigaction(SIGUSR1, &act, NULL);

	cache_block[0] = '\0';

	// Get the mount namespace of init
	if (read_namespace(1, init_ns, 32)) {
		LOGE("proc_monitor: Your kernel doesn't support mount namespace :(\n");
		quit_pthread(SIGUSR1);
	}
	LOGI("proc_monitor: init ns=%s\n", init_ns);

	// Get the mount namespace of zygote
	zygote_num = 0;
	while(!zygote_num) {
		// Check zygote every 2 secs
		sleep(2);
		ps_filter_proc_name("zygote", store_zygote_ns);
	}
	ps_filter_proc_name("zygote64", store_zygote_ns);

	switch(zygote_num) {
	case 1:
		LOGI("proc_monitor: zygote ns=%s\n", zygote_ns[0]);
		break;
	case 2:
		LOGI("proc_monitor: zygote ns=%s zygote64 ns=%s\n", zygote_ns[0], zygote_ns[1]);
		break;
	}

	// Register our listener to logcat monitor
	xpipe2(pipefd, O_CLOEXEC);
	logcat_events[HIDE_EVENT] = pipefd[1];

	for (char *log, *line; xxread(pipefd[0], &log, sizeof(log)) > 0; free(log)) {
		char *ss;
		if ((ss = strstr(log, "am_proc_start")) && (ss = strchr(ss, '['))) {
			int pid, ret, comma = 0;
			char *pos = ss, processName[256], ns[32];

			while(1) {
				pos = strchr(pos, ',');
				if(pos == NULL)
					break;
				pos[0] = ' ';
				++comma;
			}

			if (comma == 6)
				ret = sscanf(ss, "[%*d %d %*d %*d %256s", &pid, processName);
			else
				ret = sscanf(ss, "[%*d %d %*d %256s", &pid, processName);

			if(ret != 2)
				continue;

			ret = 0;

			// Critical region
			pthread_mutex_lock(&hide_lock);
			vec_for_each(hide_list, line) {
				if (strcmp(processName, line) == 0) {
					while(1) {
						ret = 1;
						for (int i = 0; i < zygote_num; ++i) {
							read_namespace(pid, ns, sizeof(ns));
							if (strcmp(ns, zygote_ns[i]) == 0) {
								usleep(50);
								ret = 0;
								break;
							}
						}
						if (ret) break;
					}

					// Send pause signal ASAP
					if (kill(pid, SIGSTOP) == -1) continue;

					LOGI("proc_monitor: %s (PID=%d ns=%s)\n", processName, pid, ns);

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
}
