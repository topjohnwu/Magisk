/* b64xz.c - Base64-XZ Extractor
 *
 * This program expects data from stdin. The data should be compressed with xz and
 * then encoded into base64 format. What b64xz does is basically the reverse of the
 * mentioned process: decode base64 to uint8_ts, decompress xz, then dump to stdout
 *
 * The compiled binary will be hex-dumped into update-binary
 * Busybox will be xz-compressed, base64 encoded and dumped into update-binary
 * This program is to recover busybox for Magisk installation environment
 *
 * I intentionally removed stdio. This will result in a smaller binary size because
 * all I/O are handled by system calls (read/write) instead of libc wrappers
 */

#include <unistd.h>
#include <lzma.h>

#define BUFSIZE 8192

static const char trans_tbl[] =
		"|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

static void decodeblock(uint8_t* in, uint8_t* out) {
	out[0] = (uint8_t)(in[0] << 2 | in[1] >> 4);
	out[1] = (uint8_t)(in[1] << 4 | in[2] >> 2);
	out[2] = (uint8_t)(((in[2] << 6) & 0xc0) | in[3]);
}

static int unxz(lzma_stream *strm, void *buf, size_t size) {
	lzma_ret ret = 0;
	uint8_t out[BUFSIZE];
	strm->next_in = buf;
	strm->avail_in = size;
	do {
		strm->next_out = out;
		strm->avail_out = sizeof(out);
		ret = lzma_code(strm, LZMA_RUN);
		write(STDOUT_FILENO, out, sizeof(out) - strm->avail_out);
	} while (strm->avail_out == 0 && ret == LZMA_OK);

	if (ret != LZMA_OK && ret != LZMA_STREAM_END)
		write(STDERR_FILENO, "LZMA error!\n", 13);
	return ret;
}

int main(int argc, char const* argv[]) {
	if (argc > 1)
		return 0;

	uint8_t in[4], buf[BUFSIZE];
	int len = 0, pos = 0;
	char c;

	// Setup lzma stream
	lzma_stream strm = LZMA_STREAM_INIT;
	if (lzma_auto_decoder(&strm, UINT64_MAX, 0) != LZMA_OK) {
		write(STDERR_FILENO, "Unable to init lzma stream\n", 28);
		return 1;
	}

	while (read(STDIN_FILENO, &c, sizeof(c)) == 1) {
		c = ((c < 43 || c > 122) ? -1 : (trans_tbl[c - 43] == '$' ? -1 : trans_tbl[c - 43] - 62));
		if (c >= 0)
			in[len++] = c;
		if (len < 4)
			continue;
		len = 0;
		decodeblock(in, buf + pos);
		pos += 3;
		if (pos > sizeof(buf) - 3) {
			// Buffer is full, unxz
			if (unxz(&strm, buf, pos))
				return 1;
			pos = 0;
		}
	}
	if (pos) {
		if (unxz(&strm, buf, pos))
			return 1;
	}
	lzma_end(&strm);
	return 0;
}
