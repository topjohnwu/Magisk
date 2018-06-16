/* main.c - The multicall entry point
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>

#include "utils.h"
#include "magisk.h"
#include "daemon.h"

char *argv0;

int (*applet_main[]) (int, char *[]) = { su_client_main, resetprop_main, magiskhide_main, NULL };

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
		"   --createimg IMG SIZE      create ext4 image. SIZE is interpreted in MB\n"
		"   --imgsize IMG             report ext4 image used/total size\n"
		"   --resizeimg IMG SIZE      resize ext4 image. SIZE is interpreted in MB\n"
		"   --mountimg IMG PATH       mount IMG to PATH and prints the loop device\n"
		"   --umountimg PATH LOOP     unmount PATH and delete LOOP device\n"
		"   --daemon                  manually start magisk daemon\n"
		"   --[init trigger]          start service for init trigger\n"
		"   --unlock-blocks           set BLKROSET flag to OFF for all block devices\n"
		"   --restorecon              fix selinux context on Magisk files and folders\n"
		"   --clone-attr SRC DEST     clone permission, owner, and selinux context\n"
		"\n"
		"Supported init triggers:\n"
		"   startup, post-fs-data, service\n"
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
		printf("%s (%d)\n", MAGISK_VER_STR, MAGISK_VER_CODE);
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
	} else if (strcmp(argv[1], "--createimg") == 0) {
		if (argc < 4) usage();
		int size;
		sscanf(argv[3], "%d", &size);
		return create_img(argv[2], size);
	} else if (strcmp(argv[1], "--imgsize") == 0) {
		if (argc < 3) usage();
		int used, total;
		if (get_img_size(argv[2], &used, &total)) {
			fprintf(stderr, "Cannot check %s size\n", argv[2]);
			return 1;
		}
		printf("%d %d\n", used, total);
		return 0;
	} else if (strcmp(argv[1], "--resizeimg") == 0) {
		if (argc < 4) usage();
		int used, total, size;
		sscanf(argv[3], "%d", &size);
		if (get_img_size(argv[2], &used, &total)) {
			fprintf(stderr, "Cannot check %s size\n", argv[2]);
			return 1;
		}
		if (size <= used) {
			fprintf(stderr, "Cannot resize smaller than %dM\n", used);
			return 1;
		}
		return resize_img(argv[2], size);
	} else if (strcmp(argv[1], "--mountimg") == 0) {
		if (argc < 4) usage();
		char *loop = mount_image(argv[2], argv[3]);
		if (loop == NULL) {
			fprintf(stderr, "Cannot mount image!\n");
			return 1;
		} else {
			printf("%s\n", loop);
			free(loop);
			return 0;
		}
	} else if (strcmp(argv[1], "--umountimg") == 0) {
		if (argc < 4) usage();
		umount_image(argv[2], argv[3]);
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
		close(fd);
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
	}

	// Applets
	argc--;
	argv++;
	for (int i = 0; applet[i]; ++i) {
		if (strcmp(basename(argv[0]), applet[i]) == 0)
			return (*applet_main[i])(argc, argv);
	}

	usage();
	return 1;
}

int main(int argc, char *argv[]) {
	umask(0);
	argv0 = argv[0];
	if (strcmp(basename(argv0), "magisk.bin") == 0) {
		if (argc >= 2) {
			// It's calling applets
			--argc;
			++argv;
		}
	}

	// Applets
	for (int i = 0; applet[i]; ++i) {
		if (strcmp(basename(argv[0]), applet[i]) == 0)
			return (*applet_main[i])(argc, argv);
	}

	// Not an applet
	return magisk_main(argc, argv);
}
