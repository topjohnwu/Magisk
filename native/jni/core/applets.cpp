#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <magisk.h>
#include <selinux.h>
#include <utils.h>

static int (*applet_main[]) (int, char *[]) =
		{ magisk_main, su_client_main, resetprop_main, magiskhide_main, nullptr };

[[noreturn]] static void call_applets(int argc, char *argv[]) {
	// Applets
	for (int i = 0; applet_names[i]; ++i) {
		if (strcmp(basename(argv[0]), applet_names[i]) == 0) {
			exit((*applet_main[i])(argc, argv));
		}
	}
	if (strncmp(basename(argv[0]), "app_process", 11) == 0)
		exit(app_process_main(argc, argv));
	fprintf(stderr, "%s: applet not found\n", argv[0]);
	exit(1);
}

int main(int argc, char *argv[]) {
	umask(0);
	dload_selinux();
	cmdline_logging();
	init_argv0(argc, argv);

	if ((strcmp(basename(argv[0]), "magisk") == 0 && argc > 1 && argv[1][0] != '-')) {
		--argc;
		++argv;
	}

	call_applets(argc, argv);
}

