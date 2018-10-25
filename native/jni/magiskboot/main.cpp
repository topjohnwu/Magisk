#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <mincrypt/sha.h>

#include "magiskboot.h"
#include "logging.h"
#include "utils.h"

/********************
  Patch Boot Image
*********************/

static void usage(char *arg0) {
	fprintf(stderr,
		"Usage: %s <action> [args...]\n"
		"\n"
		"Supported actions:\n"
		"  --unpack <bootimg>\n"
		"    Unpack <bootimg> to kernel, ramdisk.cpio, and if available, second, dtb,\n"
		"    and extra into the current directory. Return values:\n"
		"    0:valid    1:error    2:chromeos    3:ELF32    4:ELF64\n"
		"\n"
		"  --repack <origbootimg> [outbootimg]\n"
		"    Repack kernel, ramdisk.cpio[.ext], second, dtb... from current directory\n"
		"    to [outbootimg], or new-boot.img if not specified.\n"
		"    It will compress ramdisk.cpio with the same method used in <origbootimg>,\n"
		"    or attempt to find ramdisk.cpio.[ext], and repack directly with the\n"
		"    compressed ramdisk file\n"
		"\n"
		"  --hexpatch <file> <hexpattern1> <hexpattern2>\n"
		"    Search <hexpattern1> in <file>, and replace with <hexpattern2>\n"
		"\n"
		"  --cpio <incpio> [commands...]\n"
		"    Do cpio commands to <incpio> (modifications are done directly)\n"
		"    Each command is a single argument, use quotes if necessary\n"
		"    Supported commands:\n"
		"      rm [-r] ENTRY\n"
		"        Remove ENTRY, specify [-r] to remove recursively\n"
		"      mkdir MODE ENTRY\n"
		"        Create directory ENTRY in permissions MODE\n"
		"      ln TARGET ENTRY\n"
		"        Create a symlink to TARGET with the name ENTRY\n"
		"      mv SOURCE DEST\n"
		"        Move SOURCE to DEST\n"
		"      add MODE ENTRY INFILE\n"
		"        Add INFILE as ENTRY in permissions MODE; replaces ENTRY if exists\n"
		"      extract [ENTRY OUT]\n"
		"        Extract ENTRY to OUT, or extract all entries to current directory\n"
		"      test\n"
		"        Test the current cpio's patch status\n"
		"        Return values:\n"
		"        0:stock    1:Magisk    2:unsupported (phh, SuperSU, Xposed)\n"
		"      patch KEEPVERITY KEEPFORCEENCRYPT\n"
		"        Ramdisk patches. KEEP**** are boolean values\n"
		"      backup ORIG [SHA1]\n"
		"        Create ramdisk backups from ORIG\n"
		"        SHA1 of stock boot image is optional\n"
		"      restore\n"
		"        Restore ramdisk from ramdisk backup stored within incpio\n"
		"      magisk ORIG KEEPVERITY KEEPFORCEENCRYPT [SHA1]\n"
		"        Do Magisk patches and backups all in one step\n"
		"        Create ramdisk backups from ORIG\n"
		"        KEEP**** are boolean values\n"
		"        SHA1 of stock boot image is optional\n"
		"      sha1\n"
		"        Print stock boot SHA1 if previously stored\n"
		"\n"
		"  --dtb-<cmd> <dtb>\n"
		"    Do dtb related cmds to <dtb> (modifications are done directly)\n"
		"    Supported commands:\n"
		"      dump\n"
		"        Dump all contents from dtb for debugging\n"
		"      test\n"
		"        Check if fstab has verity/avb flags\n"
		"        Return values:\n"
		"        0:no flags    1:flag exists\n"
		"      patch\n"
		"        Search for fstab and remove verity/avb\n"
		"\n"
		"  --compress[=method] <infile> [outfile]\n"
		"    Compress <infile> with [method] (default: gzip), optionally to [outfile]\n"
		"    <infile>/[outfile] can be '-' to be STDIN/STDOUT\n"
		"    Supported methods: "
	, arg0);
	for (int i = 0; SUP_LIST[i]; ++i)
		fprintf(stderr, "%s ", SUP_LIST[i]);
	fprintf(stderr,
		"\n\n"
		"  --decompress <infile> [outfile]\n"
		"    Detect method and decompress <infile>, optionally to [outfile]\n"
		"    <infile>/[outfile] can be '-' to be STDIN/STDOUT\n"
		"    Supported methods: ");
	for (int i = 0; SUP_LIST[i]; ++i)
		fprintf(stderr, "%s ", SUP_LIST[i]);
	fprintf(stderr,
		"\n\n"
		"  --sha1 <file>\n"
		"    Print the SHA1 checksum for <file>\n"
		"\n"
		"  --cleanup\n"
		"    Cleanup the current working directory\n"
		"\n");

	exit(1);
}

int main(int argc, char *argv[]) {
	cmdline_logging();
	fprintf(stderr, "MagiskBoot v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu) - Boot Image Modification Tool\n");

	umask(0);
	if (argc > 1 && strcmp(argv[1], "--cleanup") == 0) {
		fprintf(stderr, "Cleaning up...\n");
		char name[PATH_MAX];
		unlink(KERNEL_FILE);
		unlink(RAMDISK_FILE);
		unlink(RAMDISK_FILE ".raw");
		unlink(SECOND_FILE);
		unlink(DTB_FILE);
		unlink(EXTRA_FILE);
		unlink(RECV_DTBO_FILE);
		for (int i = 0; SUP_EXT_LIST[i]; ++i) {
			sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
			unlink(name);
		}
	} else if (argc > 2 && strcmp(argv[1], "--sha1") == 0) {
		uint8_t sha1[SHA_DIGEST_SIZE];
		void *buf;
		size_t size;
		mmap_ro(argv[2], &buf, &size);
		SHA_hash(buf, size, sha1);
		for (int i = 0; i < SHA_DIGEST_SIZE; ++i)
			printf("%02x", sha1[i]);
		printf("\n");
		munmap(buf, size);
	} else if (argc > 2 && strcmp(argv[1], "--unpack") == 0) {
		return unpack(argv[2]);
	} else if (argc > 2 && strcmp(argv[1], "--repack") == 0) {
		repack(argv[2], argc > 3 ? argv[3] : NEW_BOOT);
	} else if (argc > 2 && strcmp(argv[1], "--decompress") == 0) {
		decompress(argv[2], argc > 3 ? argv[3] : NULL);
	} else if (argc > 2 && strncmp(argv[1], "--compress", 10) == 0) {
		const char *method;
		method = strchr(argv[1], '=');
		if (method == NULL) method = "gzip";
		else method++;
		compress(method, argv[2], argc > 3 ? argv[3] : NULL);
	} else if (argc > 4 && strcmp(argv[1], "--hexpatch") == 0) {
		hexpatch(argv[2], argv[3], argv[4]);
	} else if (argc > 2 && strcmp(argv[1], "--cpio") == 0) {
		if (cpio_commands(argc - 2, argv + 2)) usage(argv[0]);
	} else if (argc > 2 && strncmp(argv[1], "--dtb", 5) == 0) {
		char *cmd = argv[1] + 5;
		if (*cmd == '\0') usage(argv[0]);
		else ++cmd;
		if (dtb_commands(cmd, argc - 2, argv + 2))
			usage(argv[0]);
	} else {
		usage(argv[0]);
	}

	return 0;
}
