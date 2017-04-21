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
#include "resetprop.h"

int sv[2], hide_pid = -1;
struct vector *hide_list = NULL;

int hideEnabled = 0;
static pthread_t proc_monitor_thread;
pthread_mutex_t hide_lock;

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
	if (hideEnabled) {
		write_int(client, HIDE_IS_ENABLED);
		close(client);
		return;
	}

	LOGI("* Starting MagiskHide\n");

	hideEnabled = 1;
	setprop("persist.magisk.hide", "1");

	hide_sensitive_props();

	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) == -1)
		goto error;

	/*
	 * The setns system call do not support multithread processes
	 * We have to fork a new process, and communicate with sockets
	 */

	if (hide_daemon())
		goto error;

	close(sv[1]);

	// Initialize the hide list
	if (init_list())
		goto error;

	// Add SafetyNet by default
	add_list(strdup("com.google.android.gms.unstable"));

	// Initialize the mutex lock
	pthread_mutex_init(&hide_lock, NULL);

	write_int(client, HIDE_SUCCESS);
	close(client);

	// Get thread reference
	proc_monitor_thread = pthread_self();
	// Start monitoring
	proc_monitor();
	return;

error:
	hideEnabled = 0;
	write_int(client, HIDE_ERROR);
	close(client);
	if (hide_pid != -1) {
		int kill = -1;
		// Kill hide daemon
		write(sv[0], &kill, sizeof(kill));
		close(sv[0]);
		waitpid(hide_pid, NULL, 0);
		hide_pid = -1;
	}
	return;
}

void stop_magiskhide(int client) {
	if (!hideEnabled) {
		write_int(client, HIDE_NOT_ENABLED);
		close(client);
		return;
	}

	LOGI("* Stopping MagiskHide\n");

	hideEnabled = 0;
	setprop("persist.magisk.hide", "0");
	pthread_kill(proc_monitor_thread, SIGUSR1);

	write_int(client, HIDE_SUCCESS);
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
	hide_ret code = read_int(fd);
	close(fd);
	switch (code) {
	case HIDE_ERROR:
		fprintf(stderr, "Error occured in daemon...\n");
		break;
	case HIDE_SUCCESS:
		break;
	case HIDE_NOT_ENABLED:
		fprintf(stderr, "Magisk hide is not enabled yet\n");
		break;
	case HIDE_IS_ENABLED:
		fprintf(stderr, "Magisk hide is already enabled\n");
		break;
	case HIDE_ITEM_EXIST:
		fprintf(stderr, "Process [%s] already exists in hide list\n", argv[2]);
		break;
	case HIDE_ITEM_NOT_EXIST:
		fprintf(stderr, "Process [%s] does not exist in hide list\n", argv[2]);
		break;
	}
	return code;
}
