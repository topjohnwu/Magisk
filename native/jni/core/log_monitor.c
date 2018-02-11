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

#include "magisk.h"
#include "utils.h"
#include "resetprop.h"

extern int is_daemon_init;
int logd = 0;

static int am_proc_start_filter(const char *log) {
	return strstr(log, "am_proc_start") != NULL;
}

static int magisk_log_filter(const char *log) {
	char *ss;
	return (ss = strstr(log, " Magisk")) && (ss[-1] != 'D') && (ss[-1] != 'V');
}

static int magisk_debug_log_filter(const char *log) {
	return strstr(log, "Magisk") != NULL;
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

#ifdef MAGISK_DEBUG
static int debug_log_pid = -1, debug_log_fd = -1;
#endif

static void check_logd() {
	char *prop = getprop("init.svc.logd");
	if (prop != NULL) {
		free(prop);
		logd = 1;
	} else {
		LOGD("log_monitor: logd not started, disable logging");
		logd = 0;
	}
}

static void *logger_thread(void *args) {
	int log_fd = -1, log_pid;
	char line[4096];

	LOGD("log_monitor: logger start");

	while (1) {
		if (!logd) {
			// Disable all services
			for (int i = 0; i < (sizeof(log_events) / sizeof(struct log_listener)); ++i) {
				close(log_events[i].fd);
				log_events[i].fd = -1;
			}
			return NULL;
		}

		// Start logcat
		log_pid = exec_command(0, &log_fd, NULL, "logcat", "-b", "events", "-b", "main", "-v", "threadtime", "-s", "am_proc_start", "-s", "Magisk", NULL);
		while (fdgets(line, sizeof(line), log_fd)) {
			for (int i = 0; i < (sizeof(log_events) / sizeof(struct log_listener)); ++i) {
				if (log_events[i].fd > 0 && log_events[i].filter(line)) {
					char *s = strdup(line);
					xwrite(log_events[i].fd, &s, sizeof(s));
				}
			}
			if (kill(log_pid, 0))
				break;
		}

		// Cleanup
		close(log_fd);
		log_fd = -1;
		kill(log_pid, SIGTERM);
		waitpid(log_pid, NULL, 0);

		// Clear buffer before restart
		exec_command_sync("logcat", "-b", "events", "-b", "main", "-c", NULL);

		check_logd();
	}

	// Should never be here, but well...
	return NULL;
}

static void *magisk_log_thread(void *args) {
	// Buffer logs before we have data access
	struct vector logs;
	vec_init(&logs);

	int pipefd[2];
	if (xpipe2(pipefd, O_CLOEXEC) == -1)
		return NULL;

	// Register our listener
	log_events[LOG_EVENT].fd = pipefd[1];

	LOGD("log_monitor: magisk log dumper start");

	FILE *log = NULL;
	for (char *line; xxread(pipefd[0], &line, sizeof(line)) > 0; free(line)) {
		if (!is_daemon_init) {
			vec_push_back(&logs, strdup(line));
		} else {
			if (log == NULL) {
				// Dump buffered logs to file
				log = xfopen(LOGFILE, "w");
				setbuf(log, NULL);
				char *tmp;
				vec_for_each(&logs, tmp) {
					fprintf(log, "%s", tmp);
					free(tmp);
				}
				vec_destroy(&logs);
			}
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
	log_events[DEBUG_EVENT].fd = pipefd[1];

	for (char *line; xxread(pipefd[0], &line, sizeof(line)) > 0; free(line))
		fprintf(log, "%s", line);

	return NULL;
}

/* Start new threads to monitor logcat and dump to logfile */
void monitor_logs() {
	pthread_t thread;

	check_logd();

	if (logd) {
		// Start log file dumper before monitor
		xpthread_create(&thread, NULL, magisk_log_thread, NULL);
		pthread_detach(thread);

		// Start logcat monitor
		xpthread_create(&thread, NULL, logger_thread, NULL);
		pthread_detach(thread);
	}
}

void start_debug_full_log() {
#ifdef MAGISK_DEBUG
	if (logd) {
		// Log everything initially
		debug_log_fd = xopen(DEBUG_LOG, O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, 0644);
		debug_log_pid = exec_command(0, &debug_log_fd, NULL, "logcat", "-v", "threadtime", NULL);
		close(debug_log_fd);
	}
#endif
}

void stop_debug_full_log() {
#ifdef MAGISK_DEBUG
	// Stop recording the boot logcat after every boot task is done
	if (debug_log_pid > 0) {
		kill(debug_log_pid, SIGTERM);
		waitpid(debug_log_pid, NULL, 0);
		// Start debug thread
		start_debug_log();
	}
#endif
}

void start_debug_log() {
#ifdef MAGISK_DEBUG
	if (logd) {
		pthread_t thread;
		// Start debug thread
		xpthread_create(&thread, NULL, debug_magisk_log_thread, NULL);
		pthread_detach(thread);
	}
#endif
}
