/* main.c - The multicall entry point
 */

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "magisk.h"
#include "daemon.h"

char *argv0;

void usage() {
        fprintf(stderr,
                "Magisk v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu) multi-call binary\n"
                "\n"
                "Usage: %s --createimg <PATH> <SIZE>\n"
                "       create ext4 image, SIZE is interpreted in MB\n"
                "   or: %s --imgsize <PATH>\n"
                "   or: %s --resizeimg <PATH> <SIZE>\n"
                "       SIZE is interpreted in MB\n"
                "   or: %s --mountimg <IMG> <PATH>\n"
                "       Prints out the loop device\n"
                "   or: %s --umountimg <PATH> <LOOP>\n"
                "   or: %s [options]\n"
                "\n"
                "Options:\n"
                "       -c            print client version\n"
                "\n"
        , argv0, argv0, argv0, argv0, argv0, argv0);

        exit(1);
}

int main(int argc, char *argv[]) {
	argv0 = argv[0];
	// Exit the whole app if error occurs by default
	char * arg = strrchr(argv[0], '/');
	if (arg) ++arg;
	else arg = argv[0];
	if (strcmp(arg, "magisk_utils") == 0) {
		if (argc < 2) usage();
		if (strcmp(argv[1], "-c") == 0) {
			printf("%s\n", MAGISK_VER_STR);
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
		}else{
			fprintf(stderr, "Command not found\n");
			return 1;
		}
	}

	fprintf(stderr, "applet not found\n");
	return 1;
}
