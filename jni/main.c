/* main.c - The entry point, should be multi-call
 */

#include "utils.h"
#include "magisk.h"

// Global buffer
char magiskbuf[BUF_SIZE];

void stub(const char *fmt, ...) {}

int main(int argc, char *argv[]) {
	char * argv0 = strrchr(argv[0], '/');
	if (argv0) argv[0] = argv0 + 1;
	if (strcmp(argv[0], "magisk") == 0) {
		if (strcmp(argv[1], "--daemon") == 0) {
			// Start daemon
			return 0;
		} else if (strcmp(argv[1], "--install") == 0) {
			// Install symlinks
			return 0;
		} else {
			// It's calling applets
			--argc;
			++argv;
		}
	}

	// Applets
	if (strcmp(argv[0], "magiskhide") == 0) {
		magiskhide_main(argc, argv);
	} else if (strcmp(argv[0], "magiskpolicy") == 0) {
		magiskpolicy_main(argc, argv);
	} else if (strcmp(argv[0], "resetprop") == 0) {
		resetprop_main(argc, argv);
	} else if (strcmp(argv[0], "su") == 0) {
		su_main(argc, argv);
	} else {
		fprintf(stderr, "Applet \'%s\' not found\n", argv[0]);
	}
	return 0;
}