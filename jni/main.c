/* main.c - The entry point, should be multi-call
 */

#include <stdlib.h>
#include <pthread.h>

#include "utils.h"
#include "magisk.h"
#include "daemon.h"

char magiskbuf[BUF_SIZE];
char *argv0;

void stub(const char *fmt, ...) {}

// Global error hander function
// Should be changed each thread/process
__thread void (*err_handler)(void);

void exit_proc() {
	exit(-1);
}

void exit_thread() {
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	argv0 = argv[0];
	// Exit the whole app if error occurs by default
	err_handler = exit_proc;
	char * arg = strrchr(argv[0], '/');
	if (arg) ++arg;
	if (strcmp(arg, "magisk") == 0) {
		if (strcmp(argv[1], "--post-fs") == 0) {
			// TODO: post-fs mode
			return 0;
		} else if (strcmp(argv[1], "--post-fs-data") == 0) {
			// TODO: post-fs-data mode
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
		// return su_main(argc, argv);
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