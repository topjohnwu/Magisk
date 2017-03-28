#include "magiskboot.h"

/********************
  Patch Boot Image
*********************/

static void usage(char *arg0) {
	fprintf(stderr, "%s --unpack <bootimg>\n", arg0);
	fprintf(stderr, "  Unpack <bootimg> to kernel, ramdisk.cpio, (second), (dtb) into the\n  current directory\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s --repack <origbootimg> [outbootimg]\n", arg0);
	fprintf(stderr, "  Repack kernel, ramdisk.cpio[.ext], second, dtb... from current directory\n");
	fprintf(stderr, "  to [outbootimg], or new-boot.img if not specified.\n");
	fprintf(stderr, "  It will compress ramdisk.cpio with the same method used in <origbootimg>\n");
	fprintf(stderr, "  if exists, or attempt to find ramdisk.cpio.[ext], and repack\n");
	fprintf(stderr, "  directly with the compressed ramdisk file\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s --hexpatch <file> <hexpattern1> <hexpattern2>\n", arg0);
	fprintf(stderr, "  Search <hexpattern1> in <file>, and replace with <hexpattern2>\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s --cpio-<cmd> <incpio> [flags...] [params...]\n", arg0);
	fprintf(stderr, "  Do cpio related cmds to <incpio> (modifications are done directly)\n  Supported commands:\n");
	fprintf(stderr, "  --cpio-rm <incpio> [-r] <entry>\n    Remove entry from cpio, flag -r to remove recursively\n");
	fprintf(stderr, "  --cpio-mkdir <incpio> <mode> <entry>\n    Create directory as an <entry>\n");
	fprintf(stderr, "  --cpio-add <incpio> <mode> <entry> <infile>\n    Add <infile> as an <entry>; replaces <entry> if already exists\n");
	fprintf(stderr, "  --cpio-extract <incpio> <entry> <outfile>\n    Extract <entry> to <outfile>\n");
	fprintf(stderr, "  --cpio-test <incpio>\n    Return value: 0/not patched 1/Magisk 2/SuperSU\n");
	fprintf(stderr, "  --cpio-patch-dmverity <incpio>\n    Remove dm-verity\n");
	fprintf(stderr, "  --cpio-patch-forceencrypt <incpio>\n    Change forceencrypt flag to encryptable\n");
	fprintf(stderr, "  --cpio-backup <incpio> <origcpio>\n    Create ramdisk backups into <incpio> from <origcpio>\n");
	fprintf(stderr, "  --cpio-restore <incpio>\n    Restore ramdisk from ramdisk backup within <incpio>\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s --compress[=method] <infile> [outfile]\n", arg0);
	fprintf(stderr, "  Compress <infile> with [method] (default: gzip), optionally to [outfile]\n  Supported methods: " SUP_LIST "\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s --decompress <infile> [outfile]\n", arg0);
	fprintf(stderr, "  Detect method and decompress <infile>, optionally to [outfile]\n  Supported methods: " SUP_LIST "\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s --sha1 <file>\n", arg0);
	fprintf(stderr, "  Print the SHA1 checksum for <file>\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "%s --cleanup\n", arg0);
	fprintf(stderr, "  Cleanup the current working directory\n");
	fprintf(stderr, "\n");

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
	printf("MagiskBoot (by topjohnwu) - Boot Image Modification Tool\n\n");

	if (argc > 1 && strcmp(argv[1], "--cleanup") == 0) {
		cleanup();
	} else if (argc > 2 && strcmp(argv[1], "--sha1") == 0) {
		char sha1[21], *buf;
		size_t size;
		mmap_ro(argv[2], (unsigned char **) &buf, &size);
		SHA1(sha1, buf, size);
		for (int i = 0; i < 20; ++i)
			printf("%02x", sha1[i]);
		printf("\n");
		munmap(buf, size);
	} else if (argc > 2 && strcmp(argv[1], "--unpack") == 0) {
		unpack(argv[2]);
	} else if (argc > 2 && strcmp(argv[1], "--repack") == 0) {
		repack(argv[2], argc > 3 ? argv[3] : NEW_BOOT);
	} else if (argc > 2 && strcmp(argv[1], "--decompress") == 0) {
		decomp_file(argv[2], argc > 3 ? argv[3] : NULL);
	} else if (argc > 2 && strncmp(argv[1], "--compress", 10) == 0) {
		char *method;
		method = strchr(argv[1], '=');
		if (method == NULL) method = "gzip";
		else method++;
		comp_file(method, argv[2], argc > 3 ? argv[3] : NULL);
	} else if (argc > 4 && strcmp(argv[1], "--hexpatch") == 0) {
		hexpatch(argv[2], argv[3], argv[4]);
	} else if (argc > 2 && strncmp(argv[1], "--cpio", 6) == 0) {
		char *command;
		command = strchr(argv[1] + 2, '-');
		if (command == NULL) usage(argv[0]);
		else ++command;
		if (cpio_commands(command, argc - 2, argv + 2)) usage(argv[0]);
	} else {
		usage(argv[0]);
	}

	return 0;
}
