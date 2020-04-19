#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

#include <utils.hpp>
#include <magisk.hpp>
#include <daemon.hpp>
#include <selinux.hpp>
#include <flags.h>

using namespace std::literals;

[[noreturn]] static void usage() {
	fprintf(stderr,
NAME_WITH_VER(Magisk) R"EOF( multi-call binary

Usage: magisk [applet [arguments]...]
   or: magisk [options]...

Options:
   -c                        print current binary version
   -v                        print running daemon version
   -V                        print running daemon version code
   --list                    list all available applets
   --daemon                  manually start magisk daemon
   --remove-modules          remove all modules and reboot

Advanced Options (Internal APIs):
   --[init trigger]          start service for init trigger
                             Supported init triggers:
                             post-fs-data, service, boot-complete
   --unlock-blocks           set BLKROSET flag to OFF for all block devices
   --restorecon              restore selinux context on Magisk files
   --clone-attr SRC DEST     clone permission, owner, and selinux context
   --clone SRC DEST          clone SRC to DEST
   --sqlite SQL              exec SQL commands to Magisk database
   --path                    print Magisk tmpfs mount path

Available applets:
)EOF");

	for (int i = 0; applet_names[i]; ++i)
		fprintf(stderr, i ? ", %s" : "    %s", applet_names[i]);
	fprintf(stderr, "\n\n");
	exit(1);
}

int magisk_main(int argc, char *argv[]) {
	if (argc < 2)
		usage();
	if (argv[1] == "-c"sv) {
		printf(MAGISK_VERSION ":MAGISK (" str(MAGISK_VER_CODE) ")\n");
		return 0;
	} else if (argv[1] == "-v"sv) {
		int fd = connect_daemon();
		write_int(fd, CHECK_VERSION);
		char *v = read_string(fd);
		printf("%s\n", v);
		free(v);
		return 0;
	} else if (argv[1] == "-V"sv) {
		int fd = connect_daemon();
		write_int(fd, CHECK_VERSION_CODE);
		printf("%d\n", read_int(fd));
		return 0;
	} else if (argv[1] == "--list"sv) {
		for (int i = 0; applet_names[i]; ++i)
			printf("%s\n", applet_names[i]);
		return 0;
	} else if (argv[1] == "--unlock-blocks"sv) {
		unlock_blocks();
		return 0;
	} else if (argv[1] == "--restorecon"sv) {
		restorecon();
		return 0;
	} else if (argc >= 4 && argv[1] == "--clone-attr"sv) {;
		clone_attr(argv[2], argv[3]);
		return 0;
	} else if (argc >= 4 && argv[1] == "--clone"sv) {
		cp_afc(argv[2], argv[3]);
		return 0;
	} else if (argv[1] == "--daemon"sv) {
		int fd = connect_daemon(true);
		write_int(fd, DO_NOTHING);
		return 0;
	} else if (argv[1] == "--post-fs-data"sv) {
		int fd = connect_daemon(true);
		write_int(fd, POST_FS_DATA);
		return read_int(fd);
	} else if (argv[1] == "--service"sv) {
		int fd = connect_daemon(true);
		write_int(fd, LATE_START);
		return read_int(fd);
	} else if (argv[1] == "--boot-complete"sv) {
		int fd = connect_daemon(true);
		write_int(fd, BOOT_COMPLETE);
		return read_int(fd);
	} else if (argc >= 3 && argv[1] == "--sqlite"sv) {
		int fd = connect_daemon();
		write_int(fd, SQLITE_CMD);
		write_string(fd, argv[2]);
		for (;;) {
			char *res = read_string(fd);
			if (res[0] == '\0') {
				return 0;
			}
			printf("%s\n", res);
			free(res);
		}
	} else if (argv[1] == "--remove-modules"sv) {
		int fd = connect_daemon();
		write_int(fd, REMOVE_MODULES);
		return read_int(fd);
	} else if (argv[1] == "--path"sv) {
		int fd = connect_daemon();
		write_int(fd, GET_PATH);
		char *path = read_string(fd);
		printf("%s\n", path);
		return 0;
	}
#if 0
	/* Entry point for testing stuffs */
	else if (argv[1] == "--test"sv) {
		return 0;
	}
#endif
	usage();
}
