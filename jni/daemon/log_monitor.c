/* log_monitor.c - New thread to monitor logcat
 *
 * Open a new thread to call logcat and get logs with tag "Magisk"
 * Also, write the logs to a log file for debugging purpose
 *
 */

#include <stdio.h>
#include <limits.h>
#include <pthread.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"

static void *logger_thread(void *args) {
	// Setup error handler
	err_handler = exit_thread;

	char buffer[PATH_MAX];
	rename("/cache/magisk.log", "/cache/last_magisk.log");
	FILE *logfile = xfopen("/cache/magisk.log", "w");
	// Disable buffering
	setbuf(logfile, NULL);
	// Start logcat
	char *const command[] = { "logcat", "-s", "Magisk", "-v", "time", NULL };
	int fd;
	run_command(&fd, "/system/bin/logcat", command);
	while (fdgets(buffer, sizeof(buffer), fd)) {
		fprintf(logfile, "%s", buffer);
	}
	return NULL;
}

/* Start a new thread to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t log_monitor_thread;
	xpthread_create(&log_monitor_thread, NULL, logger_thread, NULL);
	pthread_detach(log_monitor_thread);
}
