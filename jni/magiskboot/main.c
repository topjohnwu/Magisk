#include "magiskboot.h"

/********************
  Patch Boot Image
*********************/

static void usage(char *arg0) {
	fprintf(stderr, "\n");
	fprintf(stderr, "%s --unpack <bootimage>\n", arg0);
	fprintf(stderr, "  Unpack <bootimage> to kernel, ramdisk.cpio, (second), (dtb) into the\n  current directory\n\n");
	fprintf(stderr, "%s --repack <bootimage>\n", arg0);
	fprintf(stderr, "  Repack kernel, ramdisk.cpio[.ext], second, dtb... from current directory\n");
	fprintf(stderr, "  to new-image.img. <bootimage> is the original boot image you've just unpacked.\n");
	fprintf(stderr, "  If file ramdisk.cpio exists, it will auto re-compress with the same method\n");
	fprintf(stderr, "  used in <bootimage>, or it will attempt to find ramdisk.cpio.[ext], and repack\n");
	fprintf(stderr, "  directly with the compressed file\n\n");
	fprintf(stderr, "%s --hexpatch <file> <hexpattern1> <hexpattern2>\n", arg0);
	fprintf(stderr, "  Search <hexpattern1> in <file>, and replace with <hexpattern2>\n\n");
	fprintf(stderr, "%s --compress[=method] <file>\n", arg0);
	fprintf(stderr, "  Compress <file> with [method], or gzip if not specified.\n  Supported methods: " SUP_LIST "\n\n");
	fprintf(stderr, "%s --decompress <file>\n", arg0);
	fprintf(stderr, "  Auto check file type, and decompress <file> accordingly\n  Supported methods: " SUP_LIST "\n\n");
	fprintf(stderr, "%s --cleanup\n", arg0);
	fprintf(stderr, "  Cleanup the current working directory\n\n");
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
	printf("MagiskBoot (by topjohnwu) - Boot Image Modification Tool\n");
	if (argc < 3) {
		if (strcmp(argv[1], "--cleanup") == 0) {
			cleanup();
		} else {
			usage(argv[0]);
		}
	} else {
		if (strcmp(argv[1], "--unpack") == 0) {
			unpack(argv[2]);
		} else if (strcmp(argv[1], "--repack") == 0) {
			repack(argv[2]);
		} else if (strcmp(argv[1], "--hexpatch") == 0) {
			if (argc < 5)
				usage(argv[0]);
			hexpatch(argv[2], argv[3], argv[4]);
		} else if (strcmp(argv[1], "--decompress") == 0) {
			decomp_file(argv[2]);
		} else if (strstr(argv[1], "--compress") != NULL) {
			char *method;
			method = strchr(argv[1], '=');
			if (method == NULL) method = "gzip";
			else method++;
			comp_file(method, argv[2]);
		} else {
			usage(argv[0]);
		}
	}
	return 0;

}
