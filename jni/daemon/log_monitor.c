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
		char *const command[] = { "logcat", "-s", "Magisk", "-v", "thread", NULL };
		log_pid = run_command(0, &log_fd, "/system/bin/logcat", command);
		waitpid(log_pid, NULL, 0);
		// For some reason it went here, clear buffer and restart
		system("logcat -c");
	}

	// Should never be here, but well...
	close(log_fd);
	return NULL;
}

/* Start a new thread to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t thread;
	xpthread_create(&thread, NULL, logger_thread, NULL);
	pthread_detach(thread);
}
