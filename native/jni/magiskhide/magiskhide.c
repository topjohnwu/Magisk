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

	if (!check_and_start_logger()) {
		if (client > 0) {
			write_int(client, LOGCAT_DISABLED);
			close(client);
		}
		setprop(MAGISKHIDE_PROP, "0");
		// Remove without actually removing persist props
		deleteprop2(MAGISKHIDE_PROP, 0);
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
	pthread_kill(proc_monitor_thread, TERM_THREAD);

	write_int(client, DAEMON_SUCCESS);
	close(client);
}

int magiskhide_main(int argc, char *argv[]) {
	if (argc < 2) {
		usage(argv[0]);
	}
	int code = 0;
	int req = DO_NOTHING;
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
	code = read_int(fd);
	switch (code) {
	case DAEMON_SUCCESS:
		break;
	case ROOT_REQUIRED:
		fprintf(stderr, "Root is required for this operation\n");
		goto END;
	case LOGCAT_DISABLED:
		fprintf(stderr, "Logcat is disabled, cannot start MagiskHide\n");
		goto END;
	case HIDE_NOT_ENABLED:
		fprintf(stderr, "MagiskHide is not enabled yet\n");
		goto END;
	case HIDE_IS_ENABLED:
		fprintf(stderr, "MagiskHide is already enabled\n");
		goto END;
	case HIDE_ITEM_EXIST:
		fprintf(stderr, "Process [%s] already exists in hide list\n", argv[2]);
		goto END;
	case HIDE_ITEM_NOT_EXIST:
		fprintf(stderr, "Process [%s] does not exist in hide list\n", argv[2]);
		goto END;
	case DAEMON_ERROR:
	default:
		fprintf(stderr, "Error occured in daemon...\n");
		goto END;
	}

	if (req == LS_HIDELIST) {
		int argc = read_int(fd);
		for (int i = 0; i < argc; ++i) {
			char *s = read_string(fd);
			printf("%s\n", s);
			free(s);
		}
	}

END:
	close(fd);
	return code;
}
