/* log_monitor.c - New thread to monitor logcat
 *
 * Open a new thread to call logcat and get logs with tag "Magisk"
 * Also, write the logs to a log file for debugging purpose
 *
 */

#include <stdio.h>
#include <pthread.h>

#include "magisk.h"
#include "utils.h"

static void *logger_thread(void *args) {
	rename("/cache/magisk.log", "/cache/last_magisk.log");
	FILE *logfile = xfopen("/cache/magisk.log", "w");
	// Disable buffering
	setbuf(logfile, NULL);
	// Start logcat
	FILE *p = popen("logcat -s Magisk", "r");
	while (fgets(magiskbuf, BUF_SIZE, p)) {
		fprintf(logfile, "%s", magiskbuf);
	}
	return NULL;
}

/* Start a new thread to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t log_monitor;
	pthread_create(&log_monitor, NULL, logger_thread, NULL);
}