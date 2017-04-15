/* magiskhide.c - initialize the environment for Magiskhide
 */

// TODO: Functions to modify hiding list

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "magisk.h"
#include "utils.h"
#include "magiskhide.h"
#include "daemon.h"

int sv[2], hide_pid = -1;
struct vector *hide_list, *new_list;

int isEnabled = 0;
static pthread_t proc_monitor_thread;

void kill_proc(int pid) {
	kill(pid, SIGTERM);
}

static void usage(char *arg0) {
	fprintf(stderr,
		"%s [--options [arguments...] ]\n\n"
		"Options:\n"
		"  --enable: Start the magiskhide daemon\n"
		"  --disable: Stop the magiskhide daemon\n"
		"  --add <process name>: Add <process name> to the list\n"
		"  --rm <process name>: Remove <process name> from the list\n"
		"  --ls: Print out the current hide list\n"
		, arg0);
	exit(1);
}

void launch_magiskhide(int client) {
	if (isEnabled) {
		write_int(client, 0);
		return;
	}
	/*
	 * The setns system call do not support multithread processes
	 * We have to fork a new process, and communicate with pipe
	 */

	LOGI("* Starting MagiskHide\n");

	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) == -1)
		goto error;

	// Launch the hide daemon
	if (hide_daemon())
		goto error;

	close(sv[1]);

	// Initialize the hide list
	hide_list = new_list = xmalloc(sizeof(*hide_list));
	if (hide_list == NULL)
		goto error;
	vec_init(hide_list);
	FILE *fp = xfopen(HIDELIST, "r");
	if (fp == NULL)
		goto error;
	file_to_vector(hide_list, fp);
	fclose(fp);
	char *line;
	vec_for_each(hide_list, line) {
		LOGI("hide_list: [%s]\n", line);
		ps_filter_proc_name(line, kill_proc);
	}

	// Start a new thread to monitor processes
	if (xpthread_create(&proc_monitor_thread, NULL, proc_monitor, NULL))
		goto error;

	isEnabled = 1;
	write_int(client, 0);
	close(client);
	return;
error:
	write_int(client, 1);
	close(client);
	if (hide_pid != -1) {
		int kill = -1;
		// Kill hide daemon
		write(sv[0], &kill, sizeof(kill));
		close(sv[0]);
		waitpid(hide_pid, NULL, 0);
	}
	return;
}

void stop_magiskhide(int client) {
	if (!isEnabled)
		return;

	LOGI("* Stopping MagiskHide\n");

	int kill = -1;
	// Terminate hide daemon
	write(sv[0], &kill, sizeof(kill));
	close(sv[0]);
	waitpid(hide_pid, NULL, 0);
	// Stop process monitor
	pthread_kill(proc_monitor_thread, SIGUSR1);
	pthread_join(proc_monitor_thread, NULL);

	isEnabled = 0;
	write_int(client, 0);
	close(client);
}

int magiskhide_main(int argc, char *argv[]) {
	if (argc < 2) {
		usage(argv[0]);
	}
	client_request req;
	if (strcmp(argv[1], "--enable") == 0) {
		req = LAUNCH_MAGISKHIDE;
	} else if (strcmp(argv[1], "--disable") == 0) {
		req = STOP_MAGISKHIDE;
	} else if (strcmp(argv[1], "--add") == 0 && argc > 2) {
		req = ADD_HIDELIST;
	} else if (strcmp(argv[1], "--rm") == 0 && argc > 2) {
		req = RM_HIDELIST;
	} else if (strcmp(argv[1], "--ls") == 0) {
		FILE *fp = xfopen(HIDELIST, "r");
		char buffer[512];
		while (fgets(buffer, sizeof(buffer), fp)) {
			printf("%s", buffer);
		}
		fclose(fp);
		return 0;
	}
	int fd = connect_daemon();
	write_int(fd, req);
	if (req == ADD_HIDELIST || req == RM_HIDELIST) {
		write_string(fd, argv[2]);
	}
	int code = read_int(fd);
	close(fd);
	if (code) {
		fprintf(stderr, "Error occured in MagiskHide daemon\n");
	}
	return code;
}
