#include <zlib.h>
#include <lzma.h>

#include "magiskboot.h"

// Mode: 0 = decode gz; 1 = encode gz
void gzip(int mode, const char* filename, unsigned char* buf, size_t size) {
	size_t ret, flush, have, pos = 0;
	z_stream strm;
	unsigned char out[CHUNK];

	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd < 0)
		error(1, "Unable to create %s", filename);

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	switch(mode) {
		case 0:
			ret = inflateInit2(&strm, windowBits | ZLIB_GZIP);
			break;
		case 1:
			ret = deflateInit2(&strm, 9, Z_DEFLATED, windowBits | ZLIB_GZIP, memLevel, Z_DEFAULT_STRATEGY);
			break;
		default:
			error(1, "Unsupported gzip mode!");
	}

	if (ret != Z_OK)
		error(1, "Unable to init zlib");

	do {
		strm.next_in = buf + pos;
		if (pos + CHUNK >= size) {
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
			switch(mode) {
				case 0:
					ret = inflate(&strm, flush);
					break;
				case 1:
					ret = deflate(&strm, flush);
					break;
			}
			if (ret == Z_STREAM_ERROR)
				error(1, "Error when running gzip");

			have = CHUNK - strm.avail_out;
			if (write(fd, out, have) != have)
				error(1, "Error when writing %s", filename);

		} while (strm.avail_out == 0);

	} while(pos < size);

	switch(mode) {
		case 0:
			inflateEnd(&strm);
			break;
		case 1:
			deflateEnd(&strm);
			break;
	}
	close(fd);
}


// Mode: 0 = decode xz/lzma; 1 = encode xz; 2 = encode lzma
void lzma(int mode, const char* filename, unsigned char* buf, size_t size) {
	size_t have, pos = 0;
	lzma_ret ret;
	lzma_stream strm = LZMA_STREAM_INIT;
	lzma_options_lzma opt;
	lzma_action action = LZMA_RUN;
	unsigned char out[BUFSIZ];

	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd < 0)
		error(1, "Unable to create %s", filename);

	// Initialize preset
	lzma_lzma_preset(&opt, LZMA_PRESET_DEFAULT);
	lzma_filter filters[] = {
		{ .id = LZMA_FILTER_LZMA2, .options = &opt },
		{ .id = LZMA_VLI_UNKNOWN, .options = NULL },
	};

	switch(mode) {
		case 0:
			ret = lzma_auto_decoder(&strm, UINT64_MAX, 0);
			break;
		case 1:
			ret = lzma_stream_encoder(&strm, filters, LZMA_CHECK_CRC64);
			break;
		case 2:
			ret = lzma_alone_encoder(&strm, &opt);
			break;
		default:
			error(1, "Unsupported lzma mode!");
	}


	if (ret != LZMA_OK)
		error(1, "Unable to init lzma");

	do {
		strm.next_in = buf + pos;
		if (pos + BUFSIZ >= size) {
			strm.avail_in = size - pos;
			pos = size;
			action = LZMA_FINISH;
		} else {
			strm.avail_in = BUFSIZ;
			pos += BUFSIZ;
		}

		do {
			strm.avail_out = BUFSIZ;
			strm.next_out = out;
			ret = lzma_code(&strm, action);
			have = BUFSIZ - strm.avail_out;
			if (write(fd, out, have) != have)
				error(1, "Error in writing %s", filename);
		} while (strm.avail_out == 0 && ret == LZMA_OK);

		if (ret != LZMA_OK && ret != LZMA_STREAM_END)
			error(1, "LZMA error %d!", ret);

	} while (pos < size);

	lzma_end(&strm);
}
