#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <mincrypt/sha.h>
#include <logging.h>
#include <utils.h>
#include <flags.h>

#include "magiskboot.h"
#include "compress.h"

using namespace std;

static void usage(char *arg0) {
	fprintf(stderr,
		FULL_VER(MagiskBoot) " - Boot Image Modification Tool\n"
		"Usage: %s <action> [args...]\n"
		"\n"
		"Supported actions:\n"
		"  unpack [-h] <bootimg>\n"
		"    Unpack <bootimg> to, if available, kernel, kernel_dtb, ramdisk.cpio,\n"
		"    second, dtb, extra, and recovery_dtbo into current directory.\n"
		"    If '-h' is provided, it will dump header info to 'header',\n"
		"    which will be parsed when repacking.\n"
		"    Return values:\n"
		"    0:valid    1:error    2:chromeos\n"
		"\n"
		"  repack [-n] <origbootimg> [outbootimg]\n"
		"    Repack boot image components from current directory\n"
		"    to [outbootimg], or new-boot.img if not specified.\n"
		"    If '-n' is provided, it will not attempt to recompress ramdisk.cpio,\n"
		"    otherwise it will compress ramdisk.cpio and kernel with the same method\n"
		"    in <origbootimg> if the file provided is not already compressed.\n"
		"\n"
		"  hexpatch <file> <hexpattern1> <hexpattern2>\n"
		"    Search <hexpattern1> in <file>, and replace with <hexpattern2>\n"
		"\n"
		"  cpio <incpio> [commands...]\n"
		"    Do cpio commands to <incpio> (modifications are done directly)\n"
		"    Each command is a single argument, use quotes if necessary\n"
		"    Supported commands:\n"
		"      exists ENTRY\n"
		"        Return 0 if ENTRY exists, else return 1\n"
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
		"      backup ORIG\n"
		"        Create ramdisk backups from ORIG\n"
		"      restore\n"
		"        Restore ramdisk from ramdisk backup stored within incpio\n"
		"      sha1\n"
		"        Print stock boot SHA1 if previously backed up in ramdisk\n"
		"\n"
		"  dtb-<cmd> <dtb>\n"
		"    Do dtb related cmds to <dtb> (modifications are done directly)\n"
		"    Supported commands:\n"
		"      dump\n"
		"        Dump all contents from dtb for debugging\n"
		"      test\n"
		"        Check if fstab has verity/avb flags\n"
		"        Return values:\n"
		"        0:flag exists    1:no flags\n"
		"      patch\n"
		"        Search for fstab and remove verity/avb\n"
		"\n"
		"  compress[=method] <infile> [outfile]\n"
		"    Compress <infile> with [method] (default: gzip), optionally to [outfile]\n"
		"    <infile>/[outfile] can be '-' to be STDIN/STDOUT\n"
		"    Supported methods: "
	, arg0);
	for (auto &it : name2fmt)
		fprintf(stderr, "%s ", it.first.data());
	fprintf(stderr,
		"\n\n"
		"  decompress <infile> [outfile]\n"
		"    Detect method and decompress <infile>, optionally to [outfile]\n"
		"    <infile>/[outfile] can be '-' to be STDIN/STDOUT\n"
		"    Supported methods: ");
	for (auto &it : name2fmt)
		fprintf(stderr, "%s ", it.first.data());
	fprintf(stderr,
		"\n\n"
		"  sha1 <file>\n"
		"    Print the SHA1 checksum for <file>\n"
		"\n"
		"  cleanup\n"
		"    Cleanup the current working directory\n"
		"\n");

	exit(1);
}

int main(int argc, char *argv[]) {
	cmdline_logging();
	umask(0);

	if (argc < 2)
		usage(argv[0]);

	// Skip '--' for backwards compatibility
	string_view action(argv[1]);
	if (str_starts(action, "--"))
		action = argv[1] + 2;

	if (action == "cleanup") {
		fprintf(stderr, "Cleaning up...\n");
		unlink(HEADER_FILE);
		unlink(KERNEL_FILE);
		unlink(RAMDISK_FILE);
		unlink(SECOND_FILE);
		unlink(KER_DTB_FILE);
		unlink(EXTRA_FILE);
		unlink(RECV_DTBO_FILE);
		unlink(DTB_FILE);
	} else if (argc > 2 && action == "sha1") {
		uint8_t sha1[SHA_DIGEST_SIZE];
		void *buf;
		size_t size;
		mmap_ro(argv[2], buf, size);
		SHA_hash(buf, size, sha1);
		for (uint8_t i : sha1)
			printf("%02x", i);
		printf("\n");
		munmap(buf, size);
	} else if (argc > 2 && action == "unpack") {
		if (argv[2] == "-h"sv) {
			if (argc == 3)
				usage(argv[0]);
			return unpack(argv[3], true);
		} else {
			return unpack(argv[2]);
		}
	} else if (argc > 2 && action == "repack") {
		if (argv[2] == "-n"sv) {
			if (argc == 3)
				usage(argv[0]);
			repack(argv[3], argv[4] ? argv[4] : NEW_BOOT, true);
		} else {
			repack(argv[2], argv[3] ? argv[3] : NEW_BOOT);
		}
	} else if (argc > 2 && action == "decompress") {
		decompress(argv[2], argv[3]);
	} else if (argc > 2 && str_starts(action, "compress")) {
		compress(action[8] == '=' ? &action[9] : "gzip", argv[2], argv[3]);
	} else if (argc > 4 && action == "hexpatch") {
		return hexpatch(argv[2], argv[3], argv[4]);
	} else if (argc > 2 && action == "cpio"sv) {
		if (cpio_commands(argc - 2, argv + 2))
			usage(argv[0]);
	} else if (argc > 2 && str_starts(action, "dtb")) {
		if (action[3] != '-')
			usage(argv[0]);
		if (dtb_commands(&action[4], argc - 2, argv + 2))
			usage(argv[0]);
	} else {
		usage(argv[0]);
	}

	return 0;
}
