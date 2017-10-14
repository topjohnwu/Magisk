/* log_monitor.c - New thread to monitor logcat
 *
 * A universal logcat monitor for many usages. Add listeners to the list,
 * and the pointer of the new log line will be sent through pipes to trigger
 * asynchronous events without polling
 */

#include <stdio.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

#include "magisk.h"
#include "utils.h"

int logcat_events[] = { -1, -1, -1 };
extern int is_restart;

#ifdef MAGISK_DEBUG
static int debug_log_pid, debug_log_fd;
#endif

static void *logger_thread(void *args) {
	int log_fd = -1, log_pid;
	char line[4096];

	LOGD("log_monitor: logger start");

	while (1) {
		// Start logcat
		log_pid = exec_command(0, &log_fd, NULL, "logcat", "-b", "all" , "-v", "threadtime", "-s", "am_proc_start", "Magisk", NULL);
		while (fdgets(line, sizeof(line), log_fd)) {
			for (int i = 0; i < (sizeof(logcat_events) / sizeof(int)); ++i) {
				if (logcat_events[i] > 0) {
					char *s = strdup(line);
					xwrite(logcat_events[i], &s, sizeof(s));
				}
			}
			if (kill(log_pid, 0))
				break;
		}
		// Clear buffer if restart required
		exec_command_sync("logcat", "-b", "all", "-c", NULL);
	}

	// Should never be here, but well...
	return NULL;
}

static void *magisk_log_thread(void *args) {
	int have_data = 0;

	// Temp buffer for logs before we have data access
	struct vector logs;
	vec_init(&logs);

	FILE *log;
	int pipefd[2];
	if (xpipe2(pipefd, O_CLOEXEC) == -1)
		return NULL;

	// Register our listener
	logcat_events[LOG_EVENT] = pipefd[1];

	LOGD("log_monitor: magisk log dumper start");

	for (char *line; xxread(pipefd[0], &line, sizeof(line)) > 0; free(line)) {
		char *ss;
		if ((ss = strstr(line, " Magisk")) && (ss[-1] != 'D') && (ss[-1] != 'V')) {
			if (!have_data) {
				if ((have_data = check_data())) {
					// Dump buffered logs to file
					if (!is_restart)
						rename(LOGFILE, LASTLOG);
					log = xfopen(LOGFILE, "a");
					setbuf(log, NULL);
					char *tmp;
					vec_for_each(&logs, tmp) {
						fprintf(log, "%s", tmp);
						free(tmp);
					}
					vec_destroy(&logs);
				} else {
					vec_push_back(&logs, strdup(line));
				}
			}
			if (have_data)
				fprintf(log, "%s", line);
		}
	}
	return NULL;
}

static void *debug_magisk_log_thread(void *args) {
	FILE *log = xfopen(DEBUG_LOG, "a");
	setbuf(log, NULL);
	int pipefd[2];
	if (xpipe2(pipefd, O_CLOEXEC) == -1)
		return NULL;

	LOGD("log_monitor: debug log dumper start");

	// Register our listener
	logcat_events[DEBUG_EVENT] = pipefd[1];

	for (char *line; xxread(pipefd[0], &line, sizeof(line)) > 0; free(line)) {
		char *ss;
		if ((ss = strstr(line, "Magisk")))
			fprintf(log, "%s", line);
	}
	return NULL;
}

/* Start new threads to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t thread;

	// Start log file dumper before monitor
	xpthread_create(&thread, NULL, magisk_log_thread, NULL);
	pthread_detach(thread);

	// Start logcat monitor
	xpthread_create(&thread, NULL, logger_thread, NULL);
	pthread_detach(thread);

}

void start_debug_full_log() {
#ifdef MAGISK_DEBUG
	// Log everything initially
	debug_log_fd = xopen(DEBUG_LOG, O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, 0644);
	debug_log_pid = exec_command(0, &debug_log_fd, NULL, "logcat", "-v", "threadtime", NULL);
	close(debug_log_fd);
#endif
}

void stop_debug_full_log() {
#ifdef MAGISK_DEBUG
	// Stop recording the boot logcat after every boot task is done
	kill(debug_log_pid, SIGTERM);
	waitpid(debug_log_pid, NULL, 0);
	pthread_t thread;
	// Start debug thread
	xpthread_create(&thread, NULL, debug_magisk_log_thread, NULL);
	pthread_detach(thread);
	start_debug_log();
#endif
}

void start_debug_log() {
#ifdef MAGISK_DEBUG
	pthread_t thread;
	// Start debug thread
	xpthread_create(&thread, NULL, debug_magisk_log_thread, NULL);
	pthread_detach(thread);
#endif
}
