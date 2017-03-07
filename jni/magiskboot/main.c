#include "magiskboot.h"

/********************
  Patch Boot Image
*********************/

static void usage(char *arg0) {
	fprintf(stderr, "\n");
	fprintf(stderr, "%s --unpack <bootimg>\n", arg0);
	fprintf(stderr, "  Unpack <bootimg> to kernel, ramdisk.cpio, (second), (dtb) into the\n  current directory\n\n");

	fprintf(stderr, "%s --repack <origbootimg> [outbootimg]\n", arg0);
	fprintf(stderr, "  Repack kernel, ramdisk.cpio[.ext], second, dtb... from current directory\n");
	fprintf(stderr, "  to [outbootimg], or new-boot.img if not specified.\n");
	fprintf(stderr, "  It will compress ramdisk.cpio with the same method used in <origbootimg>\n");
	fprintf(stderr, "  if exists, or attempt to find ramdisk.cpio.[ext], and repack\n");
	fprintf(stderr, "  directly with the compressed ramdisk file\n\n");

	fprintf(stderr, "%s --hexpatch <file> <hexpattern1> <hexpattern2>\n", arg0);
	fprintf(stderr, "  Search <hexpattern1> in <file>, and replace with <hexpattern2>\n\n");

	fprintf(stderr, "%s --cpio-<cmd> <incpio> <outcpio> [flags...] [params...]\n", arg0);
	fprintf(stderr, "  Do cpio related cmds to <incpio> and output to <outcpio>\n  Supported commands:\n");
	fprintf(stderr, "  --cpio-rm [-r] <entry>\n    Remove entry from cpio, flag -r to remove recursively\n");
	fprintf(stderr, "  --cpio-mkdir <mode> <entry>\n    Create directory as an <entry>\n");
	fprintf(stderr, "  --cpio-add <mode> <entry> <infile>\n    Add <infile> as an <entry>; will replace <entry> if already exists\n");
	fprintf(stderr, "  e.g: the command to add /tmp/tmpfile as entry init.magisk.rc:\n");
	fprintf(stderr, "  %s --cpio-add <incpio> <outcpio> 0750 init.magisk.rc /tmp/tmpfile\n\n", arg0);

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

	if (argc > 1 && strcmp(argv[1], "--cleanup") == 0) {
		cleanup();
	} else if (argc > 2 && strcmp(argv[1], "--unpack") == 0) {
		unpack(argv[2]);
	} else if (argc > 2 && strcmp(argv[1], "--repack") == 0) {
		repack(argv[2], argc > 3 ? argv[3] : NEW_BOOT);
	} else if (argc > 2 && strcmp(argv[1], "--decompress") == 0) {
		decomp_file(argv[2]);
	} else if (argc > 2 && strncmp(argv[1], "--compress", 10) == 0) {
		char *method;
		method = strchr(argv[1], '=');
		if (method == NULL) method = "gzip";
		else method++;
		comp_file(method, argv[2]);
	} else if (argc > 4 && strcmp(argv[1], "--hexpatch") == 0) {
		hexpatch(argv[2], argv[3], argv[4]);
	} else if (argc > 4 && strncmp(argv[1], "--cpio", 6) == 0) {
		int recursive = 0;
		char *command;
		command_t cmd;
		command = strchr(argv[1] + 2, '-');
		if (command == NULL) usage(argv[0]);
		else command++;
		if (strcmp(command, "rm") == 0) {
			cmd = RM;
			if (argc > 5 && strcmp(argv[4], "-r") == 0) {
				recursive = 1;
				argv[4] = argv[5];
				argc--;
			}
		} else if (argc > 5 && strcmp(command, "mkdir") == 0) {
			cmd = MKDIR;
		} else if (argc > 6 && strcmp(command, "add") == 0) {
			cmd = ADD;
		} else {
			cmd = NONE;
			usage(argv[0]);
		}
		vector v;
		vec_init(&v);
		parse_cpio(argv[2], &v);
		switch(cmd) {
			case RM:
				cpio_rm(recursive, argv[4], &v);
				break;
			case MKDIR:
				cpio_mkdir(strtoul(argv[4], NULL, 8), argv[5], &v);
				break;
			case ADD:
				cpio_add(strtoul(argv[4], NULL, 8), argv[5], argv[6], &v);
				break;
			default:
				// Never happen
				break;
		}
		dump_cpio(argv[3], &v);
		cpio_vec_destroy(&v);
	} else {
		usage(argv[0]);
	}

	return 0;
}
