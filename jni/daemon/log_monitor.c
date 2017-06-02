/* log_monitor.c - New thread to monitor logcat
 *
 * Open a new thread to call logcat and get logs with tag "Magisk"
 * Also, write the logs to a log file for debugging purpose
 *
 */

#include <stdio.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"

static void *logger_thread(void *args) {
	// Setup error handler
	err_handler = exit_thread;

	char *buffer = xmalloc(PATH_MAX);
	rename(LOGFILE, LASTLOG);
	FILE *logfile = xfdopen(xopen(LOGFILE, O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, 0644), "w");
	// Disable buffering
	setbuf(logfile, NULL);
	int fd, log_pid;

	while (1) {
		// Start logcat
		char *const command[] = { "logcat", "-s", "Magisk", "-v", "time", NULL };
		log_pid = run_command(&fd, "/system/bin/logcat", command);
		while (fdgets(buffer, PATH_MAX, fd)) {
			fprintf(logfile, "%s", buffer);
		}

		// For some reason it went here, clear buffer and restart
		close(fd);
		kill(log_pid, SIGTERM);
		waitpid(log_pid, NULL, 0);
		system("logcat -c");
	}

	// Should never be here, but well...
	fclose(logfile);
	free(buffer);
	return NULL;
}

/* Start a new thread to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t log_monitor_thread;
	xpthread_create(&log_monitor_thread, NULL, logger_thread, NULL);
	pthread_detach(log_monitor_thread);
}
