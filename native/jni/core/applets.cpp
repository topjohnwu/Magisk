#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <magisk.h>
#include <selinux.h>
#include <utils.h>

using namespace std::literals;

static int (*applet_main[]) (int, char *[]) =
		{ su_client_main, resetprop_main, magiskhide_main, nullptr };

[[noreturn]] static void call_applet(int argc, char **argv) {
	// Applets
	for (int i = 0; applet_names[i]; ++i) {
		if (strcmp(basename(argv[0]), applet_names[i]) == 0) {
			exit((*applet_main[i])(argc, argv));
		}
	}
	fprintf(stderr, "%s: applet not found\n", basename(argv[0]));
	exit(1);
}

int main(int argc, char *argv[]) {
	umask(0);
	dload_selinux();
	cmdline_logging();
	init_argv0(argc, argv);

	if (basename(argv[0]) == "magisk.bin"sv) {
		if (argc == 1)
			return 1;
		// Running through wrapper
		--argc;
		++argv;
	}

	if (basename(argv[0]) == "magisk"sv) {
		if (argc > 1 && argv[1][0] != '-') {
			// Calling applet via magisk [applet] args
			--argc;
			++argv;
		} else {
			return magisk_main(argc, argv);
		}
	}

	call_applet(argc, argv);
}

