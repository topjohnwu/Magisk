#include <sys/mount.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

#include <utils.h>
#include <magisk.h>
#include <daemon.h>
#include <selinux.h>
#include <db.h>
#include <flags.h>

[[noreturn]] static void usage() {
	fprintf(stderr,
		FULL_VER(Magisk) " multi-call binary\n"
		"\n"
		"Usage: magisk [applet [arguments]...]\n"
		"   or: magisk [options]...\n"
		"\n"
		"Options:\n"
		"   -c                        print current binary version\n"
		"   -v                        print running daemon version\n"
		"   -V                        print running daemon version code\n"
		"   --list                    list all available applets\n"
		"   --daemon                  manually start magisk daemon\n"
		"   --[init trigger]          start service for init trigger\n"
		"\n"
		"Advanced Options (Internal APIs):\n"
		"   --unlock-blocks           set BLKROSET flag to OFF for all block devices\n"
		"   --restorecon              restore selinux context on Magisk files\n"
		"   --clone-attr SRC DEST     clone permission, owner, and selinux context\n"
		"   --clone SRC DEST          clone SRC to DEST\n"
  		"   --sqlite SQL              exec SQL to Magisk database\n"
		"\n"
		"Supported init triggers:\n"
		"   post-fs-data, service, boot-complete\n"
		"\n"
		"Supported applets:\n");

	for (int i = 0; applet_names[i]; ++i)
		fprintf(stderr, i ? ", %s" : "    %s", applet_names[i]);
	fprintf(stderr, "\n\n");
	exit(1);
}

int magisk_main(int argc, char *argv[]) {
	if (argc < 2)
		usage();
	if (strcmp(argv[1], "-c") == 0) {
		printf(MAGISK_VERSION ":MAGISK (" str(MAGISK_VER_CODE) ")\n");
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
	} else if (strcmp(argv[1], "--list") == 0) {
		for (int i = 0; applet_names[i]; ++i)
			printf("%s\n", applet_names[i]);
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
	} else if (strcmp(argv[1], "--clone") == 0) {
		if (argc < 4) usage();
		cp_afc(argv[2], argv[3]);
		return 0;
	} else if (strcmp(argv[1], "--daemon") == 0) {
		int fd = connect_daemon(true);
		write_int(fd, DO_NOTHING);
		return 0;
	} else if (strcmp(argv[1], "--post-fs-data") == 0) {
		int fd = connect_daemon(true);
		write_int(fd, POST_FS_DATA);
		return read_int(fd);
	} else if (strcmp(argv[1], "--service") == 0) {
		int fd = connect_daemon(true);
		write_int(fd, LATE_START);
		return read_int(fd);
	} else if (strcmp(argv[1], "--boot-complete") == 0) {
		int fd = connect_daemon(true);
		write_int(fd, BOOT_COMPLETE);
		return read_int(fd);
	} else if (strcmp(argv[1], "--sqlite") == 0) {
		int fd = connect_daemon();
		write_int(fd, SQLITE_CMD);
		write_string(fd, argv[2]);
		send_fd(fd, STDOUT_FILENO);
		return read_int(fd);
	}
#if 0
	/* Entry point for testing stuffs */
	else if (strcmp(argv[1], "--test") == 0) {
		return 0;
	}
#endif
	usage();
}
