#include <getopt.h>
#include <stdio.h>

#include "bootimgtools.h"

/********************
  Patch Boot Image
*********************/

int usage(char *arg0) {
	fprintf(stderr, "Boot Image Unpack/Repack Tool\n");
	fprintf(stderr, "%s --extract <bootimage>\n", arg0);
	fprintf(stderr, "  Unpack <bootimage> into current directory\n\n");
	fprintf(stderr, "%s --repack <bootimage>\n", arg0);
	fprintf(stderr, "  Repack kernel, dt, ramdisk... from current directory to new-image.img\n  <bootimage> is the image you've just unpacked\n\n");
	fprintf(stderr, "%s --hexpatch <bootimage> <hexpattern1> <hexpattern2>\n", arg0);
	fprintf(stderr, "  Search <hexpattern1> in <bootimage>, and replace with <hexpattern2>\n\n");
	return 1;
}

int main(int argc, char *argv[])
{
	char ch;
	struct option long_options[] = {
		{"extract", required_argument, NULL, 'e'},
		{"repack", required_argument, NULL, 'r'},
		{"hexpatch", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	while ((ch = getopt_long(argc, argv, "e:r:p:", long_options, NULL)) != -1) {
		switch (ch) {
			case 'e':
				return extract(optarg);
			case 'r':
				return repack(optarg);
			case 'p':
				if (argc < 5) return usage(argv[0]);
				optind += 2;
				return hexpatch(argv[optind - 3], argv[optind - 2], argv[optind - 1]);
			default:
				return usage(argv[0]);
		}
	}
	return 0;
}