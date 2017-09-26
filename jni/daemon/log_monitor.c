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

	rename(LOGFILE, LASTLOG);
	int log_fd, log_pid;

	log_fd = xopen(LOGFILE, O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, 0644);

	while (1) {
		// Start logcat
		log_pid = exec_command(0, &log_fd, NULL, "logcat", "-v", "thread", "Magisk:I", "*:S", NULL);
		if (log_pid > 0)
			waitpid(log_pid, NULL, 0);
		// For some reason it went here, clear buffer and restart
		exec_command_sync("logcat", "-c", NULL);
	}

	// Should never be here, but well...
	return NULL;
}

/* Start a new thread to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t thread;
	xpthread_create(&thread, NULL, logger_thread, NULL);
	pthread_detach(thread);
}
