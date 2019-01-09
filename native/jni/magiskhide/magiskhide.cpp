#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "magisk.h"
#include "magiskhide.h"
#include "daemon.h"
#include "flags.h"

bool hide_enabled = false;

[[noreturn]] static void usage(char *arg0) {
	fprintf(stderr,
		"MagiskHide v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu)\n\n"
		"Usage: %s [--option [arguments...] ]\n\n"
		"Options:\n"
  		"  --status          Return the status of MagiskHide\n"
		"  --enable          Start magiskhide\n"
		"  --disable         Stop magiskhide\n"
		"  --add TARGET      Add TARGET to the hide list\n"
		"  --rm TARGET       Remove TARGET from the hide list\n"
		"  --ls              Print out the current hide list\n"
		"\n"
		"TARGET can be either a package name or a specific component name\n"
		"If TARGET is a package name, all components of the app will be targeted\n"
		"A component name is composed of <pkg>/<cls>\n"
		, arg0);
	exit(1);
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
	case HIDE_STATUS:
		res = hide_enabled ? HIDE_IS_ENABLED : HIDE_NOT_ENABLED;
		break;
	}

	write_int(client, res);
	close(client);
}

int magiskhide_main(int argc, char *argv[]) {
	if (argc < 2)
		usage(argv[0]);

	int req;
	if (strcmp(argv[1], "--enable") == 0)
		req = LAUNCH_MAGISKHIDE;
	else if (strcmp(argv[1], "--disable") == 0)
		req = STOP_MAGISKHIDE;
	else if (strcmp(argv[1], "--add") == 0 && argc > 2)
		req = ADD_HIDELIST;
	else if (strcmp(argv[1], "--rm") == 0 && argc > 2)
		req = RM_HIDELIST;
	else if (strcmp(argv[1], "--ls") == 0)
		req = LS_HIDELIST;
	else if (strcmp(argv[1], "--status") == 0)
		req = HIDE_STATUS;
	else
		usage(argv[0]);

	// Send request
	int fd = connect_daemon();
	write_int(fd, MAGISKHIDE);
	write_int(fd, req);
	if (req == ADD_HIDELIST || req == RM_HIDELIST)
		write_string(fd, argv[2]);
	if (req == LS_HIDELIST)
		send_fd(fd, STDOUT_FILENO);

	// Get response
	int code = read_int(fd);
	switch (code) {
	case DAEMON_SUCCESS:
		break;
	case LOGCAT_DISABLED:
		fprintf(stderr, "Logcat is disabled, cannot start MagiskHide\n");
		break;
	case HIDE_NOT_ENABLED:
		fprintf(stderr, "MagiskHide is not enabled\n");
		break;
	case HIDE_IS_ENABLED:
		fprintf(stderr, "MagiskHide is enabled\n");
		break;
	case HIDE_ITEM_EXIST:
		fprintf(stderr, "[%s] already exists in hide list\n", argv[2]);
		break;
	case HIDE_ITEM_NOT_EXIST:
		fprintf(stderr, "[%s] does not exist in hide list\n", argv[2]);
		break;

	/* Errors */
	case ROOT_REQUIRED:
		fprintf(stderr, "Root is required for this operation\n");
		break;
	case DAEMON_ERROR:
	default:
		fprintf(stderr, "Error occured in daemon...\n");
		return DAEMON_ERROR;
	}

	return req == HIDE_STATUS ? (code == HIDE_IS_ENABLED ? 0 : 1) : code != DAEMON_SUCCESS;
}
