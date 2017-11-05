/* monogisk.c - Monolithic binary hosting Magisk binaries
 *
 * This code has to be compiled statically to work properly.
 *
 * To unify Magisk support for both legacy "normal" devices and new skip_initramfs devices,
 * this tool is born. Magisk binary compilation is split into two parts - first part contains
 * "magisk" and "magiskinit". The python build script will load these 2 binaries and compress
 * them with lzma2, dumping the results into "dump.h". Monogisk is simply just a static lzma
 * extractor embedded with binary blobs, with a few additional operations to construct an
 * environment for "magiskinit" to handle the rest of the work.
 */


#include <fcntl.h>
#include <unistd.h>
#include <lzma.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "dump.h"
#include "magiskrc.h"

#define str(a) #a
#define xstr(a) str(a)
#define BUFSIZE 0x100000

lzma_stream strm = LZMA_STREAM_INIT;

static void usage() {
	const char usage[] =
	"Monogisk v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu)\n"
	"A monolithic binary used as /init to add Magisk support\n"
	"\n"
	"Usage:\n"
	"   monogisk -x <binary> <out>\n"
	"      extract \"magisk\", \"magiskinit\", or \"magiskrc\"\n"
	"   /init\n"
	"      Startup the system with Magisk support\n"
	"\n";
	write(STDERR_FILENO, usage, sizeof(usage));
	exit(1);
}

static int unxz(lzma_stream *strm, const void *buf, size_t size, int fd) {
	lzma_ret ret = 0;
	uint8_t out[BUFSIZE];
	strm->next_in = buf;
	strm->avail_in = size;
	do {
		strm->next_out = out;
		strm->avail_out = sizeof(out);
		ret = lzma_code(strm, LZMA_RUN);
		write(fd, out, sizeof(out) - strm->avail_out);
	} while (strm->avail_out == 0 && ret == LZMA_OK);

	if (ret != LZMA_OK && ret != LZMA_STREAM_END)
		write(STDERR_FILENO, "LZMA error!\n", 13);
	return ret;
}

static int dump_magisk(const char *path, mode_t mode) {
	if (lzma_auto_decoder(&strm, UINT64_MAX, 0) != LZMA_OK)
		return 1;
	unlink(path);
	int fd = creat(path, mode);
	int ret = unxz(&strm, magisk_dump, sizeof(magisk_dump), fd);
	close(fd);
	return ret;
}

static int dump_magiskinit(const char *path, mode_t mode) {
	if (lzma_auto_decoder(&strm, UINT64_MAX, 0) != LZMA_OK)
		return 1;
	unlink(path);
	int fd = creat(path, mode);
	int ret = unxz(&strm, magiskinit_dump, sizeof(magiskinit_dump), fd);
	close(fd);
	return ret;
}

static int dump_magiskrc(const char *path, mode_t mode) {
	unlink(path);
	int fd = creat(path, mode);
	write(fd, magiskrc, sizeof(magiskrc));
	close(fd);
	return 0;
}

static int init_main(int argc, char *argv[]) {
	dump_magiskinit("/init", 0750);
	mkdir("/overlay", 0);
	dump_magiskrc("/overlay/init.magisk.rc", 0750);
	mkdir("/overlay/sbin", 0755);
	dump_magisk("/overlay/sbin/magisk", 0755);
	mkdir("/overlay/root", 0755);
	link("/init", "/overlay/root/magiskpolicy");
	link("/init", "/overlay/root/supolicy");
	execv("/init", argv);
	return 1;  /*  Should not happen */
}

int main(int argc, char *argv[]) {
	umask(0);

	if (argc == 1)
		return init_main(argc, argv);

	if (argc < 4)
		usage();

	if (strcmp(argv[2], "magisk") == 0)
		dump_magisk(argv[3], 0755);
	else if (strcmp(argv[2], "magiskinit") == 0)
		dump_magiskinit(argv[3], 0755);
	else
		usage();

	lzma_end(&strm);
	return 0;
}
