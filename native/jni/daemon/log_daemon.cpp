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

bool log_daemon_started = false;
static Vector<const char *> log_cmd, clear_cmd;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

enum {
	HIDE_EVENT,
	LOG_EVENT
};

#define EVENT_NUM 2

struct log_listener {
	int fd;
	bool (*filter)(const char *);
};

static bool am_proc_start_filter(const char *log) {
	return strstr(log, "am_proc_start") != nullptr;
}

static bool magisk_log_filter(const char *log) {
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

static void sigpipe_handler(int) {
	close(events[HIDE_EVENT].fd);
	events[HIDE_EVENT].fd = -1;
}

static void *monitor_thread(void *) {
	// Block SIGPIPE to prevent interruption
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGPIPE);
	pthread_sigmask(SIG_SETMASK, &block_set, nullptr);
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

static void *logcat_thread(void *) {
	int log_fd = -1, log_pid;
	char line[4096];
	while (1) {
		// Start logcat
		log_pid = exec_array(false, &log_fd, nullptr, log_cmd.data());
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
		waitpid(log_pid, nullptr, 0);

		LOGI("magisklogd: logcat output EOF");
		// Clear buffer
		log_pid = exec_array(false, nullptr, nullptr, clear_cmd.data());
		waitpid(log_pid, nullptr, 0);
	}
}

static void log_daemon() {
	setsid();
	LOGI("Magisk v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") logger started\n");
	strcpy(argv0, "magisklogd");

	// Set SIGPIPE handler
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = sigpipe_handler;
	sigaction(SIGPIPE, &act, nullptr);

	// Setup log dumps
	rename(LOGFILE, LOGFILE ".bak");
	events[LOG_EVENT].fd = xopen(LOGFILE, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC | O_APPEND, 0644);

	// Construct cmdline
	log_cmd.push_back(MIRRDIR "/system/bin/logcat");
	// Test whether these buffers actually works
	const char *b[] = { "main", "events", "crash" };
	for (int i = 0; i < 3; ++i) {
		if (exec_command_sync(MIRRDIR "/system/bin/logcat", "-b", b[i], "-d", "-f", "/dev/null", nullptr) == 0) {
			log_cmd.push_back("-b");
			log_cmd.push_back(b[i]);
		}
	}
	chmod("/dev/null", 0666);
	clear_cmd = log_cmd;
	log_cmd.push_back("-v");
	log_cmd.push_back("threadtime");
	log_cmd.push_back("-s");
	log_cmd.push_back("am_proc_start");
	log_cmd.push_back("Magisk");
#ifdef MAGISK_DEBUG
	log_cmd.push_back("*:F");
#endif
	log_cmd.push_back(nullptr);

	clear_cmd.push_back("-c");
	clear_cmd.push_back(nullptr);

	// Start worker threads
	pthread_t thread;
	pthread_create(&thread, nullptr, monitor_thread, nullptr);
	pthread_detach(thread);
	xpthread_create(&thread, nullptr, logcat_thread, nullptr);
	pthread_detach(thread);

	// Handle socket requests
	struct sockaddr_un sun;
	socklen_t len = setup_sockaddr(&sun, LOG_SOCKET);
	int sockfd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (xbind(sockfd, (struct sockaddr*) &sun, len))
		exit(1);
	xlisten(sockfd, 10);
	while(1) {
		int fd = xaccept4(sockfd, nullptr, nullptr, SOCK_CLOEXEC);
		switch(read_int(fd)) {
			case HIDE_CONNECT:
				pthread_mutex_lock(&lock);
				close(events[HIDE_EVENT].fd);
				events[HIDE_EVENT].fd = fd;
				pthread_mutex_unlock(&lock);
				break;
			case HANDSHAKE:
				write_int(fd, HANDSHAKE);
			default:
				close(fd);
		}
	}
}

bool start_log_daemon() {
	if (!log_daemon_started) {
		if (exec_command_sync(MIRRDIR "/system/bin/logcat", "-d", "-f", "/dev/null", nullptr) == 0) {
			if (fork_dont_care() == 0)
				log_daemon();
			log_daemon_started = true;
			// Wait till we can connect to log_daemon and receive ack
			int fd = connect_log_daemon();
			write_int(fd, HANDSHAKE);
			read_int(fd);
			close(fd);
		}
		chmod("/dev/null", 0666);
	}
	return log_daemon_started;
}

int connect_log_daemon() {
	if (!log_daemon_started)
		return -1;
	struct sockaddr_un sun;
	socklen_t len = setup_sockaddr(&sun, LOG_SOCKET);
	int fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
	while (connect(fd, (struct sockaddr*) &sun, len))
		usleep(10000);
	return fd;
}
