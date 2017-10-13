/* magiskhide.c - initialize the environment for Magiskhide
 */

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

struct vector *hide_list = NULL;

int hideEnabled = 0;
static pthread_t proc_monitor_thread;
pthread_mutex_t hide_lock, file_lock;

void kill_proc(int pid) {
	kill(pid, SIGTERM);
}

static void usage(char *arg0) {
	fprintf(stderr,
		"MagiskHide v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu) - Hide Magisk!\n\n"
		"Usage: %s [--options [arguments...] ]\n\n"
		"Options:\n"
		"  --enable          Start magiskhide\n"
		"  --disable         Stop magiskhide\n"
		"  --add PROCESS     Add PROCESS to the hide list\n"
		"  --rm PROCESS      Remove PROCESS from the hide list\n"
		"  --ls              Print out the current hide list\n"
		, arg0);
	exit(1);
}

void launch_magiskhide(int client) {
	if (hideEnabled) {
		if (client > 0) {
			write_int(client, HIDE_IS_ENABLED);
			close(client);
		}
		return;
	}

	hideEnabled = 1;
	LOGI("* Starting MagiskHide\n");

	deleteprop2(MAGISKHIDE_PROP, 1);

	hide_sensitive_props();

	// Initialize the mutex lock
	pthread_mutex_init(&hide_lock, NULL);
	pthread_mutex_init(&file_lock, NULL);

	// Initialize the hide list
	if (init_list())
		goto error;

	// Add SafetyNet by default
	add_list(strdup("com.google.android.gms.unstable"));

	if (client > 0) {
		write_int(client, DAEMON_SUCCESS);
		close(client);
	}

	// Get thread reference
	proc_monitor_thread = pthread_self();
	// Start monitoring
	proc_monitor();
	return;

error:
	hideEnabled = 0;
	if (client > 0) {
		write_int(client, DAEMON_ERROR);
		close(client);
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
	setprop(MAGISKHIDE_PROP, "0");
	// Remove without actually removing persist props
	deleteprop2(MAGISKHIDE_PROP, 0);
	pthread_kill(proc_monitor_thread, SIGUSR1);

	write_int(client, DAEMON_SUCCESS);
	close(client);
}

int magiskhide_main(int argc, char *argv[]) {
	if (argc < 2) {
		usage(argv[0]);
	}
	client_request req = DO_NOTHING;
	if (strcmp(argv[1], "--enable") == 0) {
		req = LAUNCH_MAGISKHIDE;
	} else if (strcmp(argv[1], "--disable") == 0) {
		req = STOP_MAGISKHIDE;
	} else if (strcmp(argv[1], "--add") == 0 && argc > 2) {
		req = ADD_HIDELIST;
	} else if (strcmp(argv[1], "--rm") == 0 && argc > 2) {
		req = RM_HIDELIST;
	} else if (strcmp(argv[1], "--ls") == 0) {
		req = LS_HIDELIST;
	} else {
		usage(argv[0]);
	}
	int fd = connect_daemon();
	write_int(fd, req);
	if (req == ADD_HIDELIST || req == RM_HIDELIST) {
		write_string(fd, argv[2]);
	}
	daemon_response code = read_int(fd);
	switch (code) {
	case DAEMON_ERROR:
		fprintf(stderr, "Error occured in daemon...\n");
		return code;
	case DAEMON_SUCCESS:
		break;
	case ROOT_REQUIRED:
		fprintf(stderr, "Root is required for this operation\n");
		return code;
	case HIDE_NOT_ENABLED:
		fprintf(stderr, "Magisk hide is not enabled yet\n");
		return code;
	case HIDE_IS_ENABLED:
		fprintf(stderr, "Magisk hide is already enabled\n");
		return code;
	case HIDE_ITEM_EXIST:
		fprintf(stderr, "Process [%s] already exists in hide list\n", argv[2]);
		return code;
	case HIDE_ITEM_NOT_EXIST:
		fprintf(stderr, "Process [%s] does not exist in hide list\n", argv[2]);
		return code;
	}

	if (req == LS_HIDELIST) {
		int argc = read_int(fd);
		for (int i = 0; i < argc; ++i) {
			char *s = read_string(fd);
			printf("%s\n", s);
			free(s);
		}
	}
	close(fd);

	return 0;
}
