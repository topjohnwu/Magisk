#include "magiskboot.h"

void gzip(int dec, const char* filename, unsigned char* buf, size_t size) {
	int ret, flush, have, pos = 0;
	z_stream strm;
	unsigned char out[CHUNK];

	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd < 0)
		error(1, "Unable to create %s", filename);

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	if (dec) {
		ret = inflateInit2(&strm, windowBits | ZLIB_GZIP);
	} else {
		ret = deflateInit2(&strm, 9, Z_DEFLATED, windowBits | ZLIB_GZIP, memLevel, Z_DEFAULT_STRATEGY);
	}

	if (ret != Z_OK)
		error(1, "Unable to init zlib");

	do {
		strm.next_in = buf + pos;
		if (pos + CHUNK > size) {
			strm.avail_in = size - pos;
			pos = size;
			flush = Z_FINISH;
		} else {
			strm.avail_in = CHUNK;
			pos += CHUNK;
			flush = Z_NO_FLUSH;
		}

		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			if (dec) {
				inflate(&strm, flush);
			} else {
				deflate(&strm, flush);
			}
			have = CHUNK - strm.avail_out;
			if (write(fd, out, have) != have)
				error(1, "Error in writing %s", filename);
		} while (strm.avail_out == 0);

	} while(pos < size);

	if (dec) {
		inflateEnd(&strm);
	} else {
		deflateEnd(&strm);
	}
	close(fd);
}
