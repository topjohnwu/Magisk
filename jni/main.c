/* main.c - The multicall entry point
 */

#include <stdlib.h>

#include "utils.h"
#include "magisk.h"
#include "daemon.h"

char *argv0;

// Global error hander function
// Should be changed each thread/process
__thread void (*err_handler)(void);

int main(int argc, char *argv[]) {
	argv0 = argv[0];
	// Exit the whole app if error occurs by default
	err_handler = exit_proc;
	char * arg = strrchr(argv[0], '/');
	if (arg) ++arg;
	if (strcmp(arg, "magisk") == 0) {
		if (strcmp(argv[1], "-v") == 0) {
			printf("Client: %s\n", VERSION_STR);
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
		} else if (strcmp(argv[1], "--post-fs") == 0) {
			// TODO: post-fs mode
			return 0;
		} else if (strcmp(argv[1], "--post-fs-data") == 0) {
			// TODO: post-fs-data mode
			return 0;
		} else if (strcmp(argv[1], "--service") == 0) {
			// TODO: late_start service mode
			return 0;
		} else if (strcmp(argv[1], "--install") == 0) {
			// TODO: Install symlinks
			return 0;
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
	if (strcmp(arg, "su") == 0) {
		return su_client_main(argc, argv);
	} else if (strcmp(arg, "magiskpolicy") == 0) {
		return magiskpolicy_main(argc, argv);
	} else if (strcmp(arg, "resetprop") == 0) {
		return resetprop_main(argc, argv);
	} else if (strcmp(arg, "magiskhide") == 0) {
		return magiskhide_main(argc, argv);
	} else {
		fprintf(stderr, "Applet \'%s\' not found\n", arg);
	}
	return 1;
}
