/* proc_monitor.c - Monitor am_proc_start events
 * 
 * We monitor the logcat am_proc_start events, pause it,
 * and send the target PID to hide daemon ASAP
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "magisk.h"
#include "utils.h"
#include "magiskhide.h"

static int zygote_num;
static char init_ns[32], zygote_ns[2][32];
static int log_pid, log_fd;
static char *buffer;

static void read_namespace(const int pid, char* target, const size_t size) {
	char path[32];
	snprintf(path, sizeof(path), "/proc/%d/ns/mnt", pid);
	xreadlink(path, target, size);
}

// Workaround for the lack of pthread_cancel
static void quit_pthread(int sig) {
	err_handler = do_nothing;
	LOGD("proc_monitor: running cleanup\n");
	destroy_list();
	free(buffer);
	hideEnabled = 0;
	// Kill the logging if needed
	if (log_pid) {
		kill(log_pid, SIGTERM);
		waitpid(log_pid, NULL, 0);
		close(log_fd);
		log_fd = log_pid = 0;
	}
	int kill = -1;
	// If process monitor dies, kill hide daemon too
	write(sv[0], &kill, sizeof(kill));
	close(sv[0]);
	waitpid(hide_pid, NULL, 0);
	pthread_mutex_destroy(&hide_lock);
	pthread_mutex_destroy(&file_lock);
	LOGD("proc_monitor: terminating...\n");
	pthread_exit(NULL);
}

static void store_zygote_ns(int pid) {
	if (zygote_num == 2) return;
	do {
		usleep(500);
		read_namespace(pid, zygote_ns[zygote_num], 32);
	} while (strcmp(zygote_ns[zygote_num], init_ns) == 0);
	++zygote_num;
}

static void proc_monitor_err() {
	LOGD("proc_monitor: error occured, stopping magiskhide services\n");
	quit_pthread(SIGUSR1);
}

void proc_monitor() {
	// Register the cancel signal
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = quit_pthread;
	sigaction(SIGUSR1, &act, NULL);

	// The error handler should stop magiskhide services
	err_handler = proc_monitor_err;

	int pid;
	buffer = xmalloc(PATH_MAX);

	// Get the mount namespace of init
	read_namespace(1, init_ns, 32);
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
		LOGI("proc_monitor: zygote (32-bit) ns=%s (64-bit) ns=%s\n", zygote_ns[0], zygote_ns[1]);
		break;
	}

	while (1) {
		// Clear previous buffer
		system("logcat -b events -c");

		// Monitor am_proc_start
		char *const command[] = { "logcat", "-b", "events", "-v", "raw", "-s", "am_proc_start", NULL };
		log_fd = 0;
		log_pid = run_command(0, &log_fd, "/system/bin/logcat", command);

		if (log_pid < 0) continue;
		if (kill(log_pid, 0)) continue;

		while(fdgets(buffer, PATH_MAX, log_fd)) {
			int ret, comma = 0;
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
					while(1) {
						ret = 1;
						for (int i = 0; i < zygote_num; ++i) {
							read_namespace(pid, buffer, 32);
							if (strcmp(buffer, zygote_ns[i]) == 0) {
								usleep(50);
								ret = 0;
								break;
							}
						}
						if (ret) break;
					}

					ret = 0;

					// Send pause signal ASAP
					if (kill(pid, SIGSTOP) == -1) continue;

					LOGI("proc_monitor: %s (PID=%d ns=%s)\n", processName, pid, buffer);

					// Unmount start
					xwrite(sv[0], &pid, sizeof(pid));

					// Get the hide daemon return code
					xxread(sv[0], &ret, sizeof(ret));
					LOGD("proc_monitor: hide daemon response code: %d\n", ret);
					break;
				}
			}
			pthread_mutex_unlock(&hide_lock);

			if (ret) {
				// Wait hide process to kill itself
				waitpid(hide_pid, NULL, 0);
				quit_pthread(SIGUSR1);
			}
		}

		// For some reason it went here, restart logging
		kill(log_pid, SIGTERM);
		waitpid(log_pid, NULL, 0);
		close(log_fd);
		log_pid = 0;
	}
}
