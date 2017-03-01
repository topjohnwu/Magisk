#include <zlib.h>
#include <lzma.h>
#include <lz4frame.h>
#include <bzlib.h>

#include "magiskboot.h"

static int open_new(const char *filename) {
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		error(1, "Unable to create %s", filename);
	return fd;
}

static void write_file(const int fd, const void *buf, const size_t size, const char *filename) {
	if (write(fd, buf, size) != size)
		error(1, "Error in writing %s", filename);
}

static void report(const int mode, const char* filename) {
	switch(mode) {
		case 0:
			printf("Decompressing to %s\n", filename);
			break;
		default:
			printf("Compressing to %s\n", filename);
			break;
	}
}

// Mode: 0 = decode; 1 = encode
void gzip(int mode, const char* filename, unsigned char* buf, size_t size) {
	size_t ret = 0, flush, have, pos = 0;
	z_stream strm;
	unsigned char out[CHUNK];

	report(mode, filename);
	int fd = open_new(filename);

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
		error(1, "Unable to init zlib stream");

	do {
		strm.next_in = buf + pos;
		if (pos + CHUNK >= size) {
			strm.avail_in = size - pos;
			flush = Z_FINISH;
		} else {
			strm.avail_in = CHUNK;
			flush = Z_NO_FLUSH;
		}
		pos += strm.avail_in;

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
			write_file(fd, out, have, filename);

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
	lzma_ret ret = 0;
	lzma_stream strm = LZMA_STREAM_INIT;
	lzma_options_lzma opt;
	lzma_action action;
	unsigned char out[BUFSIZ];

	report(mode, filename);
	int fd = open_new(filename);

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
		error(1, "Unable to init lzma stream");

	do {
		strm.next_in = buf + pos;
		if (pos + BUFSIZ >= size) {
			strm.avail_in = size - pos;
			action = LZMA_FINISH;
		} else {
			strm.avail_in = BUFSIZ;
			action = LZMA_RUN;
		}
		pos += strm.avail_in;

		do {
			strm.avail_out = BUFSIZ;
			strm.next_out = out;
			ret = lzma_code(&strm, action);
			have = BUFSIZ - strm.avail_out;
			write_file(fd, out, have, filename);
		} while (strm.avail_out == 0 && ret == LZMA_OK);

		if (ret != LZMA_OK && ret != LZMA_STREAM_END)
			error(1, "LZMA error %d!", ret);

	} while (pos < size);

	lzma_end(&strm);
}

// Mode: 0 = decode; 1 = encode
void lz4(int mode, const char* filename, unsigned char* buf, size_t size) {
	LZ4F_decompressionContext_t dctx;
	LZ4F_compressionContext_t cctx;
	LZ4F_frameInfo_t info;

	size_t outCapacity, avail_in, ret = 0, pos = 0;
	size_t have, read;
	unsigned char *out = NULL;

	report(mode, filename);
	int fd = open_new(filename);

	// Initialize context
	switch(mode) {
		case 0:
			ret = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
			break;
		case 1:
			ret = LZ4F_createCompressionContext(&cctx, LZ4F_VERSION);
			break;
		default:
			error(1, "Unsupported lz4 mode!");
	}
	
	if (LZ4F_isError(ret))
		error(1, "Context creation error: %s\n", LZ4F_getErrorName(ret));

	// Allocate out buffer
	switch(mode) {
		case 0:
			// Read header
			read = CHUNK;
			ret = LZ4F_getFrameInfo(dctx, &info, buf, &read);
			if (LZ4F_isError(ret))
				error(1, "LZ4F_getFrameInfo error: %s\n", LZ4F_getErrorName(ret));
			switch (info.blockSizeID) {
				case LZ4F_default:
				case LZ4F_max64KB:  outCapacity = 1 << 16; break;
				case LZ4F_max256KB: outCapacity = 1 << 18; break;
				case LZ4F_max1MB:   outCapacity = 1 << 20; break;
				case LZ4F_max4MB:   outCapacity = 1 << 22; break;
				default:
					error(1, "Impossible unless more block sizes are allowed\n");
			}
			pos += read;
			break;
		case 1:
			outCapacity = LZ4F_compressBound(CHUNK, NULL) + LZ4_HEADER_SIZE + LZ4_FOOTER_SIZE;
			break;
	}

	out = malloc(outCapacity);
	if (!out)
		error(1, "LZ4 malloc error!");

	// Write header
	if (mode == 1) {
		have = ret = LZ4F_compressBegin(cctx, out, size, NULL);
		if (LZ4F_isError(ret))
			error(1, "Failed to start compression: error %s\n", LZ4F_getErrorName(ret));
		write_file(fd, out, have, filename);
	}

	do {
		if (pos + CHUNK >= size) {
			avail_in = size - pos;
		} else {
			avail_in = CHUNK;
		}

		do {
			switch(mode) {
				case 0:
					have = outCapacity, read = avail_in;
					ret = LZ4F_decompress(dctx, out, &have, buf + pos, &read, NULL);
					break;
				case 1:
					read = avail_in;
					have = ret = LZ4F_compressUpdate(cctx, out, outCapacity, buf + pos, avail_in, NULL);
					break;
			}
			if (LZ4F_isError(ret))
				error(1, "LZ4 coding error: %s\n", LZ4F_getErrorName(ret));

			write_file(fd, out, have, filename);
			// Update status
			pos += read;
			avail_in -= read;
		} while(avail_in != 0 && ret != 0);

	} while(pos < size && ret != 0);

	switch(mode) {
		case 0:
			LZ4F_freeDecompressionContext(dctx);
			break;
		case 1:
			have = ret = LZ4F_compressEnd(cctx, out, outCapacity, NULL);
			if (LZ4F_isError(ret))
				error(1, "Failed to end compression: error %s\n", LZ4F_getErrorName(ret));

			write_file(fd, out, have, filename);

			LZ4F_freeCompressionContext(cctx);
			break;
	}

	free(out);
}

// Mode: 0 = decode; 1 = encode
void bzip2(int mode, const char* filename, unsigned char* buf, size_t size) {
	size_t ret = 0, action, have, pos = 0;
	bz_stream strm;
	char out[CHUNK];

	report(mode, filename);
	int fd = open_new(filename);

	strm.bzalloc = NULL;
	strm.bzfree = NULL;
	strm.opaque = NULL;

	switch(mode) {
		case 0:
			ret = BZ2_bzDecompressInit(&strm, 0, 0);
			break;
		case 1:
			ret = BZ2_bzCompressInit(&strm, 9, 0, 0);
			break;
		default:
			error(1, "Unsupported bzip2 mode!");
	}

	if (ret != BZ_OK)
		error(1, "Unable to init bzlib stream");

	do {
		strm.next_in = (char *) buf + pos;
		if (pos + CHUNK >= size) {
			strm.avail_in = size - pos;
			action = BZ_FINISH;
		} else {
			strm.avail_in = CHUNK;
			action = BZ_RUN;
		}
		pos += strm.avail_in;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			switch(mode) {
				case 0:
					ret = BZ2_bzDecompress(&strm);
					break;
				case 1:
					ret = BZ2_bzCompress(&strm, action);
					break;
			}

			have = CHUNK - strm.avail_out;
			write_file(fd, out, have, filename);

		} while (strm.avail_out == 0);

	} while(pos < size);

	switch(mode) {
		case 0:
			BZ2_bzDecompressEnd(&strm);
			break;
		case 1:
			BZ2_bzCompressEnd(&strm);
			break;
	}
	close(fd);
}
