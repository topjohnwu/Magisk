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

static int zygote_num = 0;
static char init_ns[32], zygote_ns[2][32];
static FILE *p;

static void read_namespace(const int pid, char* target, const size_t size) {
	char path[32];
	snprintf(path, sizeof(path), "/proc/%d/ns/mnt", pid);
	ssize_t len = readlink(path, target, size);
	target[len] = '\0';
}

// Workaround for the lack of pthread_cancel
static void quit_pthread(int sig) {
	LOGD("proc_monitor: running cleanup\n");
	char *line;
	vec_for_each(hide_list, line) {
		ps_filter_proc_name(line, kill_proc);
	}
	vec_deep_destroy(hide_list);
	free(hide_list);
	if (new_list != hide_list) {
		vec_deep_destroy(new_list);
		free(new_list);
	}
	hide_list = new_list = NULL;
	isEnabled = 0;
	LOGD("proc_monitor: terminating...\n");
	pclose(p);
	pthread_exit(NULL);
}

static void store_zygote_ns(int pid) {
	do {
		usleep(500);
		read_namespace(pid, zygote_ns[zygote_num], 32);
	} while (strcmp(zygote_ns[zygote_num], init_ns) == 0);
	++zygote_num;
}

static void proc_monitor_err() {
	LOGD("proc_monitor: error occured, stopping magiskhide services\n");
	int kill = -1;
	// If process monitor dies, kill hide daemon too
	write(sv[0], &kill, sizeof(kill));
	close(sv[0]);
	waitpid(hide_pid, NULL, 0);
	quit_pthread(SIGUSR1);
}

void *proc_monitor(void *args) {
	// Register the cancel signal
	signal(SIGUSR1, quit_pthread);
	// The error handler should only exit the thread, not the whole process
	err_handler = proc_monitor_err;

	int pid;
	char buffer[512];

	// Get the mount namespace of init
	read_namespace(1, init_ns, 32);
	LOGI("proc_monitor: init ns=%s\n", init_ns);

	// Get the mount namespace of zygote
	ps_filter_proc_name("zygote", store_zygote_ns);

	switch(zygote_num) {
	case 1:
		LOGI("proc_monitor: zygote ns=%s\n", zygote_ns[0]);
		break;
	case 2:
		LOGI("proc_monitor: zygote (1) ns=%s (2) ns=%s\n", zygote_ns[0], zygote_ns[1]);
		break;
	}

	// Monitor am_proc_start (the command shall never end)
	p = popen("while true; do logcat -b events -c; logcat -b events -v raw -s am_proc_start; sleep 1; done", "r");

	while(fgets(buffer, sizeof(buffer), p)) {
		int ret, comma = 0;
		char *pos = buffer, *line, processName[256];
		struct vector *temp = NULL;

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

		// Should be thread safe
		if (hide_list != new_list) {
			temp = hide_list;
			hide_list = new_list;
		}

		ret = 0;

		vec_for_each(hide_list, line) {
			if (strcmp(processName, line) == 0) {
				read_namespace(pid, buffer, 32);
				while(1) {
					ret = 1;
					for (int i = 0; i < zygote_num; ++i) {
						if (strcmp(buffer, zygote_ns[i]) == 0) {
							usleep(500);
							ret = 0;
							break;
						}
					}
					if (ret) break;
				}

				ret = 0;

				// Send pause signal ASAP
				if (kill(pid, SIGSTOP) == -1) continue;

				LOGI("proc_monitor: %s(PID=%d ns=%s)\n", processName, pid, buffer);

				// Unmount start
				xwrite(sv[0], &pid, sizeof(pid));

				// Get the hide daemon return code
				xxread(sv[0], &ret, sizeof(ret));
				LOGD("proc_monitor: hide daemon response code: %d\n", ret);
				break;
			}
		}
		if (temp) {
			vec_deep_destroy(temp);
			free(temp);
		}
		if (ret) {
			// Wait hide process to kill itself
			waitpid(hide_pid, NULL, 0);
			quit_pthread(SIGUSR1);
		}
	}

	// Should never be here
	pclose(p);
	pthread_exit(NULL);
	return NULL;
}
