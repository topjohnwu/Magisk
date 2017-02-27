#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "magiskboot.h"

/********************
  Patch Boot Image
*********************/

static void usage(char *arg0) {
	fprintf(stderr, "Boot Image Unpack/Repack Tool\n");
	fprintf(stderr, "%s --unpack <bootimage>\n", arg0);
	fprintf(stderr, "  Unpack <bootimage> into current directory\n\n");
	fprintf(stderr, "%s --repack <bootimage>\n", arg0);
	fprintf(stderr, "  Repack kernel, dtb, ramdisk... from current directory to new-image.img\n  <bootimage> is the image you've just unpacked\n\n");
	fprintf(stderr, "%s --hexpatch <bootimage> <hexpattern1> <hexpattern2>\n", arg0);
	fprintf(stderr, "  Search <hexpattern1> in <bootimage>, and replace with <hexpattern2>\n\n");
	exit(1);
}

void error(int rc, const char *msg, ...) {
	va_list	ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	fprintf(stderr,"\n\n");
	va_end(ap);
	exit(rc);
}

int main(int argc, char *argv[]) {
	if (argc < 3)
		usage(argv[0]);

	if (strcmp(argv[1], "--unpack") == 0) {
		unpack(argv[2]);
	} else if (strcmp(argv[1], "--repack") == 0) {
		repack(argv[2]);
	} else if (strcmp(argv[1], "--hexpatch") == 0) {
		if (argc < 5)
			usage(argv[0]);
		hexpatch(argv[2], argv[3], argv[4]);
	} else {
		usage(argv[0]);
	}

	return 0;

}
