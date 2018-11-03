/* b64xz.c - Base64-XZ Extractor
 *
 * This program converts data from stdin to stdout.
 * The input should be compressed with xz (integrity check only support CRC32 or none) and then
 * encoded into base64 format. What b64xz does is basically the reverse of the
 * mentioned process: decode base64 to bytes, decompress xz, then dump to stdout
 *
 * The compiled binary will be hex-dumped into update-binary
 * Busybox will be xz-compressed, base64 encoded and dumped into update-binary
 * This program is to recover busybox for Magisk installation environment
 */

#include <unistd.h>
#include <xz.h>

static const char trans_tbl[] =
		"|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

static void decodeblock(uint8_t* in, uint8_t* out) {
	out[0] = (uint8_t)(in[0] << 2 | in[1] >> 4);
	out[1] = (uint8_t)(in[1] << 4 | in[2] >> 2);
	out[2] = (uint8_t)(((in[2] << 6) & 0xc0) | in[3]);
}

static int unxz(struct xz_dec *dec, const void *buf, unsigned size) {
	uint8_t out[8192];
	struct xz_buf b = {
			.in = buf,
			.in_pos = 0,
			.in_size = size,
			.out = out,
			.out_pos = 0,
			.out_size = sizeof(out)
	};
	enum xz_ret ret;
	do {
		ret = xz_dec_run(dec, &b);
		if (ret != XZ_OK && ret != XZ_STREAM_END)
			return 1;
		write(STDOUT_FILENO, out, b.out_pos);
		b.out_pos = 0;
	} while (b.in_pos != size);
	return 0;
}

int main(int argc, char const* argv[]) {
	if (argc > 1)
		return 0;

	uint8_t in[4], buf[6144];
	unsigned len = 0, pos = 0;
	int8_t c;

	xz_crc32_init();
	struct xz_dec *dec = xz_dec_init(XZ_DYNALLOC, 1 << 26);
	if (dec == NULL)
		return 1;

	while (read(STDIN_FILENO, &c, sizeof(c)) == 1) {
		c = ((c < 43 || c > 122) ? -1 : (trans_tbl[c - 43] == '$' ? -1 : trans_tbl[c - 43] - 62));
		if (c >= 0)
			in[len++] = c;
		if (len < 4)
			continue;
		len = 0;
		decodeblock(in, buf + pos);
		pos += 3;
		if (pos == sizeof(buf)) {
			if (unxz(dec, buf, pos))
				return 1;
			pos = 0;
		}
	}
	if (pos && unxz(dec, buf, pos))
		return 1;
	xz_dec_end(dec);
	return 0;
}
