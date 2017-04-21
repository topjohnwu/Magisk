/* main.c - The multicall entry point
 */

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "magisk.h"
#include "daemon.h"

char *argv0;

char *applet[] =
	{ "su", "resetprop", "magiskpolicy", "supolicy", "magiskhide", NULL };

int (*applet_main[]) (int, char *[]) =
	{ su_client_main, resetprop_main, magiskpolicy_main, magiskpolicy_main, magiskhide_main, NULL };

// Global error hander function
// Should be changed each thread/process
__thread void (*err_handler)(void);

static void usage() {
	fprintf(stderr,
		"Magisk v" xstr(MAGISK_VERSION) " multi-call binary\n"
		"\n"
		"Usage: %s [applet [arguments]...]\n"
		"   or: %s --install [SOURCE] <DIR> \n"
		"   or: %s --list\n"
		"   or: %s --[boot stage]    start boot stage service\n"
		"   or: %s [options]\n"
		"   or: applet [arguments]...\n"
		"\n"
		"Supported boot stages:\n"
		"       post-fs, post-fs-data, service\n"
		"\n"
		"Options:\n"
		"       -v            print client and daemon version\n"
		"       -V            print daemon version code\n"
		"\n"
		"Supported applets:\n"
	, argv0, argv0, argv0, argv0, argv0);

	for (int i = 0; applet[i]; ++i) {
		fprintf(stderr, i ? ", %s" : "       %s", applet[i]);
	}
	fprintf(stderr, "\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	argv0 = argv[0];
	// Exit the whole app if error occurs by default
	err_handler = exit_proc;
	char * arg = strrchr(argv[0], '/');
	if (arg) ++arg;
	else arg = argv[0];
	if (strcmp(arg, "magisk") == 0) {
		if (argc < 2) usage();
		if (strcmp(argv[1], "-v") == 0) {
			printf("Client: %s\n", MAGISK_VER_STR);
			int fd = connect_daemon();
			write_int(fd, CHECK_VERSION);
			char *v = read_string(fd);
			printf("Daemon: %s\n", v);
			free(v);
			return 0;
		} else if (strcmp(argv[1], "-V") == 0) {
			int fd = connect_daemon();
			write_int(fd, CHECK_VERSION_CODE);
			printf("%d\n", read_int(fd));
			return 0;
		} else if (strcmp(argv[1], "--install") == 0) {
			if (argc < 3) usage();
			if (argc == 3) return create_links(NULL, argv[2]);
			else return create_links(argv[2], argv[3]);
		} else if (strcmp(argv[1], "--list") == 0) {
			for (int i = 0; applet[i]; ++i)
				printf("%s\n", applet[i]);
			return 0;
		} else if (strcmp(argv[1], "--post-fs") == 0) {
			int fd = connect_daemon();
			write_int(fd, POST_FS);
			return read_int(fd);
		} else if (strcmp(argv[1], "--post-fs-data") == 0) {
			int fd = connect_daemon();
			write_int(fd, POST_FS_DATA);
			return read_int(fd);
		} else if (strcmp(argv[1], "--service") == 0) {
			int fd = connect_daemon();
			write_int(fd, LATE_START);
			return read_int(fd);
		} else if (strcmp(argv[1], "--test") == 0) {
			// Temporary testing entry
			int fd = connect_daemon();
			write_int(fd, TEST);
			write_string(fd, argv[2]);
			return read_int(fd);
		} else {
			// It's calling applets
			--argc;
			++argv;
			arg = argv[0];
		}
	}

	// Applets
	for (int i = 0; applet[i]; ++i) {
		if (strcmp(arg, applet[i]) == 0)
			return (*applet_main[i])(argc, argv);
	}

	fprintf(stderr, "%s: applet not found\n", arg);
	return 1;
}
