/* main.c - The multicall entry point
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

#include "utils.h"
#include "magisk.h"
#include "daemon.h"
#include "selinux.h"
#include "flags.h"

char *argv0;

int (*applet_main[]) (int, char *[]) =
		{ su_client_main, resetprop_main, magiskhide_main, imgtool_main, NULL };

int create_links(const char *bin, const char *path) {
	char self[PATH_MAX], linkpath[PATH_MAX];
	if (bin == NULL) {
		xreadlink("/proc/self/exe", self, sizeof(self));
		bin = self;
	}
	int ret = 0;
	for (int i = 0; applet[i]; ++i) {
		snprintf(linkpath, sizeof(linkpath), "%s/%s", path, applet[i]);
		unlink(linkpath);
		ret |= symlink(bin, linkpath);
	}
	return ret;
}

static void usage() {
	fprintf(stderr,
		"Magisk v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu) multi-call binary\n"
		"\n"
		"Usage: magisk [applet [arguments]...]\n"
		"   or: magisk [options]...\n"
		"\n"
		"Options:\n"
		"   -c                        print current binary version\n"
		"   -v                        print running daemon version\n"
		"   -V                        print running daemon version code\n"
		"   --list                    list all available applets\n"
		"   --install [SOURCE] DIR    symlink all applets to DIR. SOURCE is optional\n"
		"   --daemon                  manually start magisk daemon\n"
		"   --[init trigger]          start service for init trigger\n"
		"   --unlock-blocks           set BLKROSET flag to OFF for all block devices\n"
		"   --restorecon              fix selinux context on Magisk files and folders\n"
		"   --clone-attr SRC DEST     clone permission, owner, and selinux context\n"
		"\n"
		"Supported init triggers:\n"
		"   startup, post-fs-data, service, boot-complete\n"
		"\n"
		"Supported applets:\n");

	for (int i = 0; applet[i]; ++i)
		fprintf(stderr, i ? ", %s" : "    %s", applet[i]);
	fprintf(stderr, "\n\n");
	exit(1);
}

int magisk_main(int argc, char *argv[]) {
	if (argc < 2)
		usage();
	if (strcmp(argv[1], "-c") == 0) {
		printf("%s (%d)\n", xstr(MAGISK_VERSION) ":MAGISK", MAGISK_VER_CODE);
		return 0;
	} else if (strcmp(argv[1], "-v") == 0) {
		int fd = connect_daemon();
		write_int(fd, CHECK_VERSION);
		char *v = read_string(fd);
		printf("%s\n", v);
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
	} else if (strcmp(argv[1], "--unlock-blocks") == 0) {
		unlock_blocks();
		return 0;
	} else if (strcmp(argv[1], "--restorecon") == 0) {
		restorecon();
		return 0;
	} else if (strcmp(argv[1], "--clone-attr") == 0) {
		if (argc < 4) usage();
		clone_attr(argv[2], argv[3]);
		return 0;
	} else if (strcmp(argv[1], "--daemon") == 0) {
		int fd = connect_daemon();
		write_int(fd, DO_NOTHING);
		return 0;
	} else if (strcmp(argv[1], "--startup") == 0) {
		startup();
		return 0;
	} else if (strcmp(argv[1], "--post-fs-data") == 0) {
		int fd = connect_daemon();
		write_int(fd, POST_FS_DATA);
		return read_int(fd);
	} else if (strcmp(argv[1], "--service") == 0) {
		int fd = connect_daemon();
		write_int(fd, LATE_START);
		return read_int(fd);
	} else if (strcmp(argv[1], "--boot-complete") == 0) {
		int fd = connect_daemon();
		write_int(fd, BOOT_COMPLETE);
		return read_int(fd);
	}

	// Applets
	argc--;
	argv++;
	for (int i = 0; applet[i]; ++i) {
		if (strcmp(basename(argv[0]), applet[i]) == 0) {
			strcpy(argv0, basename(argv[0]));
			return (*applet_main[i])(argc, argv);
		}
	}

	usage();
	return 1;
}

int main(int argc, char *argv[]) {
	umask(0);
	argv0 = argv[0];
	setup_selinux();

	if (strcmp(basename(argv0), "magisk.bin") == 0) {
		if (argc >= 2) {
			// It's calling applets
			--argc;
			++argv;
		}
	}

	// Applets
	for (int i = 0; applet[i]; ++i) {
		if (strcmp(basename(argv[0]), applet[i]) == 0) {
			strcpy(argv0, basename(argv[0]));
			return (*applet_main[i])(argc, argv);
		}
	}

	// Not an applet
	return magisk_main(argc, argv);
}
