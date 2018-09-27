/* log_daemon.c - A dedicated daemon to monitor logcat
 *
 * A universal logcat monitor for many usages. Add listeners to the list,
 * and the new log line will be sent through sockets to trigger
 * asynchronous events without polling
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"
#include "flags.h"

static int loggable = 0;
static struct vector log_cmd, clear_cmd;
static int sockfd;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

enum {
	HIDE_EVENT,
	LOG_EVENT
};

struct log_listener {
	int fd;
	int (*filter) (const char*);
};

static int am_proc_start_filter(const char *log) {
	return strstr(log, "am_proc_start") != NULL;
}

static int magisk_log_filter(const char *log) {
	return !am_proc_start_filter(log);
}

static struct log_listener events[] = {
	{	/* HIDE_EVENT */
		.fd = -1,
		.filter = am_proc_start_filter
	},
	{	/* LOG_EVENT */
		.fd = -1,
		.filter = magisk_log_filter
	}
};
#define EVENT_NUM (sizeof(events) / sizeof(struct log_listener))

static void sigpipe_handler(int sig) {
	close(events[HIDE_EVENT].fd);
	events[HIDE_EVENT].fd = -1;
}

static void *monitor_thread(void *args) {
	// Block SIGPIPE to prevent interruption
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGPIPE);
	pthread_sigmask(SIG_SETMASK, &block_set, NULL);
	// Give the main daemon some time before we monitor it
	sleep(5);
	int fd;
	char b;
	while (1) {
		fd = connect_daemon();
		write_int(fd, HANDSHAKE);
		// This should hold unless the daemon is killed
		read(fd, &b, sizeof(b));
		// The main daemon crashed, spawn a new one
		close(fd);
	}
}

static void *logcat_thread(void *args) {
	int log_fd = -1, log_pid;
	char line[4096];
	while (1) {
		// Start logcat
		log_pid = exec_array(0, &log_fd, NULL, (char **) vec_entry(&log_cmd));
		FILE *logs = fdopen(log_fd, "r");
		while (fgets(line, sizeof(line), logs)) {
			if (line[0] == '-')
				continue;
			size_t len = strlen(line);
			pthread_mutex_lock(&lock);
			for (int i = 0; i < EVENT_NUM; ++i) {
				if (events[i].fd > 0 && events[i].filter(line))
					write(events[i].fd, line, len);
			}
			pthread_mutex_unlock(&lock);
		}

		fclose(logs);
		log_fd = -1;
		kill(log_pid, SIGTERM);
		waitpid(log_pid, NULL, 0);

		LOGI("magisklogd: logcat output EOF");
		// Clear buffer
		log_pid = exec_array(0, NULL, NULL, (char **) vec_entry(&clear_cmd));
		waitpid(log_pid, NULL, 0);
	}
}

int check_and_start_logger() {
	if (!loggable) {
		int fd;
		loggable = exec_command_sync("/system/bin/logcat", "-d", "-f", "/dev/null", NULL) == 0;
		chmod("/dev/null", 0666);
		if (loggable) {
			connect_daemon2(LOG_DAEMON, &fd);
			write_int(fd, HANDSHAKE);
			close(fd);
		}
	}
	return loggable;
}

void log_daemon() {
	setsid();
	struct sockaddr_un sun;
	socklen_t len = setup_sockaddr(&sun, LOG_DAEMON);
	sockfd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (xbind(sockfd, (struct sockaddr*) &sun, len))
		exit(1);
	xlisten(sockfd, 10);
	LOGI("Magisk v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") logger started\n");
	strcpy(argv0, "magisklogd");

	// Set SIGPIPE handler
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = sigpipe_handler;
	sigaction(SIGPIPE, &act, NULL);

	// Setup log dumps
	rename(LOGFILE, LOGFILE ".bak");
	events[LOG_EVENT].fd = xopen(LOGFILE, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC | O_APPEND, 0644);

	// Construct cmdline
	vec_init(&log_cmd);
	vec_push_back(&log_cmd, "/system/bin/logcat");
	// Test whether these buffers actually works
	const char* b[] = { "main", "events", "crash" };
	for (int i = 0; i < 3; ++i) {
		if (exec_command_sync("/system/bin/logcat", "-b", b[i], "-d", "-f", "/dev/null", NULL) == 0)
			vec_push_back_all(&log_cmd, "-b", b[i], NULL);
	}
	chmod("/dev/null", 0666);
	vec_dup(&log_cmd, &clear_cmd);
	vec_push_back_all(&log_cmd, "-v", "threadtime", "-s", "am_proc_start", "Magisk", NULL);
#ifdef MAGISK_DEBUG
	vec_push_back(&log_cmd, "*:F");
#endif
	vec_push_back(&log_cmd, NULL);
	vec_push_back(&clear_cmd, "-c");
	vec_push_back(&clear_cmd, NULL);

	// Start worker threads
	pthread_t thread;
	pthread_create(&thread, NULL, monitor_thread, NULL);
	pthread_detach(thread);
	xpthread_create(&thread, NULL, logcat_thread, NULL);
	pthread_detach(thread);

	while(1) {
		int fd = xaccept4(sockfd, NULL, NULL, SOCK_CLOEXEC);
		switch(read_int(fd)) {
			case HIDE_CONNECT:
				pthread_mutex_lock(&lock);
				close(events[HIDE_EVENT].fd);
				events[HIDE_EVENT].fd = fd;
				pthread_mutex_unlock(&lock);
				break;
			case HANDSHAKE:
			default:
				close(fd);
				break;
		}
	}
}
