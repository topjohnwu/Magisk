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

static int zygote_num;
static char init_ns[32], zygote_ns[2][32], cache_block[256];
static int log_pid, log_fd, target_pid;
static char *buffer;

// Workaround for the lack of pthread_cancel
static void quit_pthread(int sig) {
	err_handler = do_nothing;
	LOGD("proc_monitor: running cleanup\n");
	destroy_list();
	free(buffer);
	hideEnabled = 0;
	// Kill the logging if needed
	if (log_pid > 0) {
		kill(log_pid, SIGTERM);
		waitpid(log_pid, NULL, 0);
		close(log_fd);
	}
	// Resume process if possible
	if (target_pid > 0)
		kill(target_pid, SIGCONT);
	pthread_mutex_destroy(&hide_lock);
	pthread_mutex_destroy(&file_lock);
	LOGD("proc_monitor: terminating...\n");
	pthread_exit(NULL);
}

static void proc_monitor_err() {
	LOGD("proc_monitor: error occured, stopping magiskhide services\n");
	quit_pthread(SIGUSR1);
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

static void hide_daemon_err() {
	LOGD("hide_daemon: error occured, stopping magiskhide services\n");
	_exit(-1);
}

static void hide_daemon(int pid) {
	LOGD("hide_daemon: start unmount for pid=[%d]\n", pid);
	// When an error occurs, report its failure to main process
	err_handler = hide_daemon_err;

	char *line;
	struct vector mount_list;

	manage_selinux();
	relink_sbin();
	clean_magisk_props();

	if (switch_mnt_ns(pid))
		return;

	snprintf(buffer, PATH_MAX, "/proc/%d/mounts", pid);
	vec_init(&mount_list);
	file_to_vector(buffer, &mount_list);

	// Find the cache block name if not found yet
	if (cache_block[0] == '\0') {
		vec_for_each(&mount_list, line) {
			if (strstr(line, " /cache ")) {
				sscanf(line, "%256s", cache_block);
				break;
			}
		}
	}

	// First unmount dummy skeletons, /sbin links, cache mounts, and mirrors
	vec_for_each(&mount_list, line) {
		if (strstr(line, "tmpfs /system") || strstr(line, "tmpfs /vendor") || strstr(line, "tmpfs /sbin")
			|| (strstr(line, cache_block) && (strstr(line, " /system") || strstr(line, " /vendor")))
			|| strstr(line, MIRRDIR)) {
			sscanf(line, "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
		free(line);
	}
	vec_destroy(&mount_list);

	// Re-read mount infos
	snprintf(buffer, PATH_MAX, "/proc/%d/mounts", pid);
	vec_init(&mount_list);
	file_to_vector(buffer, &mount_list);

	// Unmount any loop mounts and dummy mounts
	vec_for_each(&mount_list, line) {
		if (strstr(line, "/dev/block/loop") || strstr(line, DUMMDIR)) {
			sscanf(line, "%*s %4096s", buffer);
			lazy_unmount(buffer);
		}
		free(line);
	}

	// Free uo memory
	vec_destroy(&mount_list);
}

void proc_monitor() {
	// Register the cancel signal
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = quit_pthread;
	sigaction(SIGUSR1, &act, NULL);

	// The error handler should stop magiskhide services
	err_handler = proc_monitor_err;
	log_pid = target_pid = -1;

	buffer = xmalloc(PATH_MAX);
	cache_block[0] = '\0';

	// Get the mount namespace of init
	if (read_namespace(1, init_ns, 32)) {
		LOGE("proc_monitor: Your kernel doesn't support mount namespace :(\n");
		proc_monitor_err();
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

	while (1) {
		// Clear previous logcat buffer
		char *const restart[] = { "logcat", "-b", "events", "-c", NULL };
		run_command(restart);

		// Monitor am_proc_start
		char *const command[] = { "logcat", "-b", "events", "-v", "raw", "-s", "am_proc_start", NULL };
		log_fd = -1;
		log_pid = run_command2(0, &log_fd, NULL, command);

		if (log_pid < 0) continue;
		if (kill(log_pid, 0)) continue;

		while(fdgets(buffer, PATH_MAX, log_fd)) {
			int pid, ret, comma = 0;
			char *pos = buffer, *line, processName[256];

			while(1) {
				pos = strchr(pos, ',');
				if(pos == NULL)
					break;
				pos[0] = ' ';
				++comma;
			}

			if (comma == 6)
				ret = sscanf(buffer, "[%*d %d %*d %*d %256s", &pid, processName);
			else
				ret = sscanf(buffer, "[%*d %d %*d %256s", &pid, processName);

			if(ret != 2)
				continue;

			ret = 0;

			// Critical region
			pthread_mutex_lock(&hide_lock);
			vec_for_each(hide_list, line) {
				if (strcmp(processName, line) == 0) {
					target_pid = pid;
					while(1) {
						ret = 1;
						for (int i = 0; i < zygote_num; ++i) {
							read_namespace(target_pid, buffer, 32);
							if (strcmp(buffer, zygote_ns[i]) == 0) {
								usleep(50);
								ret = 0;
								break;
							}
						}
						if (ret) break;
					}

					// Send pause signal ASAP
					if (kill(target_pid, SIGSTOP) == -1) continue;

					LOGI("proc_monitor: %s (PID=%d ns=%s)\n", processName, target_pid, buffer);

					/*
					 * The setns system call do not support multithread processes
					 * We have to fork a new process, setns, then do the unmounts
					 */
					int hide_pid = fork();
					switch(hide_pid) {
					case -1:
						PLOGE("fork");
						return;
					case 0:
						hide_daemon(target_pid);
						_exit(0);
					default:
						break;
					}

					// Wait till the unmount process is done
					waitpid(hide_pid, &ret, 0);
					if (WEXITSTATUS(ret))
						quit_pthread(SIGUSR1);

					// All done, send resume signal
					kill(target_pid, SIGCONT);
					target_pid = -1;
					break;
				}
			}
			pthread_mutex_unlock(&hide_lock);
		}

		// For some reason it went here, restart logging
		kill(log_pid, SIGTERM);
		waitpid(log_pid, NULL, 0);
		close(log_fd);
	}
}
