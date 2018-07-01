/* log_monitor.c - New thread to monitor logcat
 *
 * A universal logcat monitor for many usages. Add listeners to the list,
 * and the pointer of the new log line will be sent through pipes to trigger
 * asynchronous events without polling
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "magisk.h"
#include "utils.h"
#include "resetprop.h"

int loggable = 1;

static int am_proc_start_filter(const char *log) {
	return strstr(log, "am_proc_start") != NULL;
}

static int magisk_log_filter(const char *log) {
	char *ss;
	return (ss = strstr(log, " Magisk")) && (ss[-1] != 'D') && (ss[-1] != 'V');
}

static int magisk_debug_log_filter(const char *log) {
	return strstr(log, "am_proc_start") == NULL;
}

struct log_listener log_events[] = {
	{	/* HIDE_EVENT */
		.fd = -1,
		.filter = am_proc_start_filter
	},
	{	/* LOG_EVENT */
		.fd = -1,
		.filter = magisk_log_filter
	},
	{	/* DEBUG_EVENT */
		.fd = -1,
		.filter = magisk_debug_log_filter
	}
};
#define EVENT_NUM (sizeof(log_events) / sizeof(struct log_listener))

static void test_logcat() {
	int log_fd = -1, log_pid;
	char buf[1];
	log_pid = exec_command(0, &log_fd, NULL, "logcat", NULL);
	if (read(log_fd, buf, sizeof(buf)) != sizeof(buf)) {
		loggable = 0;
		LOGD("log_monitor: cannot read from logcat, disable logging");
	}
	kill(log_pid, SIGTERM);
	waitpid(log_pid, NULL, 0);
}

static void *logger_thread(void *args) {
	int log_fd = -1, log_pid;
	char line[PIPE_BUF];

	LOGD("log_monitor: logger start");

	while (1) {
		if (!loggable) {
			// Disable all services
			for (int i = 0; i < EVENT_NUM; ++i) {
				close(log_events[i].fd);
				log_events[i].fd = -1;
			}
			return NULL;
		}

		// Start logcat
		log_pid = exec_command(0, &log_fd, NULL,
							   "/system/bin/logcat",
							   "-b", "events", "-b", "main", "-b", "crash",
							   "-v", "threadtime",
							   "-s", "am_proc_start", "Magisk", "*:F",
							   NULL);
		FILE *logs = fdopen(log_fd, "r");
		while (fgets(line, sizeof(line), logs)) {
			if (line[0] == '-')
				continue;
			size_t len = strlen(line);
			for (int i = 0; i < EVENT_NUM; ++i) {
				if (log_events[i].fd > 0 && log_events[i].filter(line))
					xwrite(log_events[i].fd, line, len);
			}
			if (kill(log_pid, 0))
				break;
		}

		// Cleanup
		fclose(logs);
		log_fd = -1;
		kill(log_pid, SIGTERM);
		waitpid(log_pid, NULL, 0);
		test_logcat();
	}

	// Should never be here, but well...
	return NULL;
}

/* Start new threads to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t thread;

	test_logcat();

	if (loggable) {
		rename(LOGFILE, LOGFILE ".bak");
		log_events[LOG_EVENT].fd = creat(LOGFILE, 0644);
#ifdef MAGISK_DEBUG
		log_events[DEBUG_EVENT].fd = creat(DEBUG_LOG, 0644);
#endif

		// Start logcat monitor
		xpthread_create(&thread, NULL, logger_thread, NULL);
		pthread_detach(thread);
	}
}
