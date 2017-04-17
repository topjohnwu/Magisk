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
	char buffer[PATH_MAX];
	xrename("/cache/magisk.log", "/cache/last_magisk.log");
	FILE *logfile = xfopen("/cache/magisk.log", "w");
	// Disable buffering
	setbuf(logfile, NULL);
	// Start logcat
	FILE *log_monitor = popen("logcat -s Magisk -v time", "r");
	while (fgets(buffer, sizeof(buffer), log_monitor)) {
		fprintf(logfile, "%s", buffer);
	}
	return NULL;
}

/* Start a new thread to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t log_monitor_thread;
	pthread_create(&log_monitor_thread, NULL, logger_thread, NULL);
}
