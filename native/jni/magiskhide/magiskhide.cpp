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
#include "flags.h"

int hide_enabled = 0;
static pthread_t proc_monitor_thread;
pthread_mutex_t list_lock;

[[noreturn]] static void usage(char *arg0) {
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

int launch_magiskhide(int client) {
	if (hide_enabled)
		return HIDE_IS_ENABLED;

	if (!log_daemon_started) {
		setprop(MAGISKHIDE_PROP, "0");
		// Remove without actually removing persist props
		deleteprop(MAGISKHIDE_PROP);
		return LOGCAT_DISABLED;
	}

	hide_enabled = 1;
	LOGI("* Starting MagiskHide\n");

	deleteprop(MAGISKHIDE_PROP, true);

	hide_sensitive_props();

	// Initialize the mutex lock
	pthread_mutex_init(&list_lock, nullptr);

	// Initialize the hide list
	if (!init_list())
		goto error;

	// Add SafetyNet by default
	add_list("com.google.android.gms.unstable");

	// Get thread reference
	proc_monitor_thread = pthread_self();
	if (client >= 0) {
		write_int(client, DAEMON_SUCCESS);
		close(client);
	}
	// Start monitoring
	proc_monitor();

error:
	hide_enabled = 0;
	return DAEMON_ERROR;
}

int stop_magiskhide() {
	LOGI("* Stopping MagiskHide\n");

	hide_enabled = 0;
	setprop(MAGISKHIDE_PROP, "0");
	// Remove without actually removing persist props
	deleteprop(MAGISKHIDE_PROP);
	pthread_kill(proc_monitor_thread, TERM_THREAD);

	return DAEMON_SUCCESS;
}

void magiskhide_handler(int client) {
	int req = read_int(client);
	int res = DAEMON_ERROR;

	switch (req) {
	case STOP_MAGISKHIDE:
	case ADD_HIDELIST:
	case RM_HIDELIST:
	case LS_HIDELIST:
		if (!hide_enabled) {
			write_int(client, HIDE_NOT_ENABLED);
			close(client);
			return;
		}
	}

	switch (req) {
	case LAUNCH_MAGISKHIDE:
		res = launch_magiskhide(client);
		break;
	case STOP_MAGISKHIDE:
		res = stop_magiskhide();
		break;
	case ADD_HIDELIST:
		res = add_list(client);
		break;
	case RM_HIDELIST:
		res = rm_list(client);
		break;
	case LS_HIDELIST:
		ls_list(client);
		client = -1;
		break;
	}

	write_int(client, res);
	close(client);
}

int magiskhide_main(int argc, char *argv[]) {
	if (argc < 2) {
		usage(argv[0]);
	}
	int req;
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
	write_int(fd, MAGISKHIDE);
	write_int(fd, req);
	if (req == ADD_HIDELIST || req == RM_HIDELIST) {
		write_string(fd, argv[2]);
	}
	int code = read_int(fd);
	switch (code) {
	case DAEMON_SUCCESS:
		break;
	case ROOT_REQUIRED:
		fprintf(stderr, "Root is required for this operation\n");
		return code;
	case LOGCAT_DISABLED:
		fprintf(stderr, "Logcat is disabled, cannot start MagiskHide\n");
		return code;
	case HIDE_NOT_ENABLED:
		fprintf(stderr, "MagiskHide is not enabled yet\n");
		return code;
	case HIDE_IS_ENABLED:
		fprintf(stderr, "MagiskHide is already enabled\n");
		return code;
	case HIDE_ITEM_EXIST:
		fprintf(stderr, "Process [%s] already exists in hide list\n", argv[2]);
		return code;
	case HIDE_ITEM_NOT_EXIST:
		fprintf(stderr, "Process [%s] does not exist in hide list\n", argv[2]);
		return code;
	case DAEMON_ERROR:
	default:
		fprintf(stderr, "Error occured in daemon...\n");
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
