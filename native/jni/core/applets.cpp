#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <magisk.hpp>
#include <selinux.hpp>
#include <utils.hpp>

using namespace std::literals;

using main_fun = int (*)(int, char *[]);

static main_fun applet_main[] = { su_client_main, resetprop_main, magiskhide_main, nullptr };

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

