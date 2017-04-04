#include <zlib.h>
#include <lzma.h>
#include <lz4.h>
#include <lz4frame.h>
#include <bzlib.h>

#include "magiskboot.h"

#define windowBits 15
#define ZLIB_GZIP 16
#define memLevel 8
#define CHUNK 0x40000

#define LZ4_HEADER_SIZE 19
#define LZ4_FOOTER_SIZE 4
#define LZ4_LEGACY_BLOCKSIZE  0x800000

static void write_file(const int fd, const void *buf, const size_t size, const char *filename) {
	if (write(fd, buf, size) != size)
		error(1, "Error in writing %s", filename);
}

static void report(const int mode, const char* filename) {
	switch(mode) {
		case 0:
			printf("Decompressing to [%s]\n\n", filename);
			break;
		default:
			printf("Compressing to [%s]\n\n", filename);
			break;
	}
}

// Mode: 0 = decode; 1 = encode
void gzip(int mode, const char* filename, const unsigned char* buf, size_t size) {
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
void lzma(int mode, const char* filename, const unsigned char* buf, size_t size) {
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
	close(fd);
}

// Mode: 0 = decode; 1 = encode
void lz4(int mode, const char* filename, const unsigned char* buf, size_t size) {
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
	close(fd);
}

// Mode: 0 = decode; 1 = encode
void bzip2(int mode, const char* filename, const unsigned char* buf, size_t size) {
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

// Mode: 0 = decode; 1 = encode
void lz4_legacy(int mode, const char* filename, const unsigned char* buf, size_t size) {
	size_t pos = 0;
	int have;
	char *out;
	unsigned block_size, insize;
	unsigned char block_size_le[4];


	report(mode, filename);
	int fd = open_new(filename);

	switch(mode) {
		case 0:
			out = malloc(LZ4_LEGACY_BLOCKSIZE);
			// Skip magic
			pos += 4;
			break;
		case 1:
			out = malloc(LZ4_COMPRESSBOUND(LZ4_LEGACY_BLOCKSIZE));
			// Write magic
			write_file(fd, "\x02\x21\x4c\x18", 4, filename);
			break;
		default:
			error(1, "Unsupported lz4_legacy mode!");
	}

	if (!out)
		error(1, "lz4_legacy malloc error");

	do {
		switch(mode) {
			case 0:
				block_size = buf[pos];
				block_size += (buf[pos + 1]<<8);
				block_size += (buf[pos + 2]<<16);
				block_size += ((unsigned)buf[pos + 3])<<24;
				pos += 4;
				if (block_size > LZ4_COMPRESSBOUND(LZ4_LEGACY_BLOCKSIZE))
					error(1, "lz4_legacy block size too large!");
				have = LZ4_decompress_safe((const char*) (buf + pos), out, block_size, LZ4_LEGACY_BLOCKSIZE);
				if (have < 0)
					error(1, "Cannot decode lz4_legacy block");
				pos += block_size;
				break;
			case 1:
				if (pos + LZ4_LEGACY_BLOCKSIZE >= size)
					insize = size - pos;
				else
					insize = LZ4_LEGACY_BLOCKSIZE;
				have = LZ4_compress_default((const char*) (buf + pos), out, insize, LZ4_COMPRESSBOUND(LZ4_LEGACY_BLOCKSIZE));
				if (have == 0)
					error(1, "lz4_legacy compression error");
				pos += insize;
				block_size_le[0] = (unsigned char)have;
				block_size_le[1] = (unsigned char)(have >> 8);
				block_size_le[2] = (unsigned char)(have >> 16);
				block_size_le[3] = (unsigned char)(have >> 24);
				write_file(fd, block_size_le, 4, filename);
				break;
		}
		// Write main data
		write_file(fd, out, have, filename);
	} while(pos < size);

	free(out);
	close(fd);
}

int decomp(file_t type, const char *to, const unsigned char *from, size_t size) {
	switch (type) {
		case GZIP:
			gzip(0, to, from, size);
			break;
		case XZ:
			lzma(0, to, from, size);
			break;
		case LZMA:
			lzma(0, to, from, size);
			break;
		case BZIP2:
			bzip2(0, to, from, size);
			break;
		case LZ4:
			lz4(0, to, from, size);
			break;
		case LZ4_LEGACY:
			lz4_legacy(0, to, from, size);
			break;
		default:
			// Unsupported
			return 1;
	}
	return 0;
}

// Output will be to.ext
int comp(file_t type, const char *to, const unsigned char *from, size_t size) {
	char name[PATH_MAX];
	const char *ext = strrchr(to, '.');
	if (ext == NULL) ext = to;
	strcpy(name, to);
	switch (type) {
		case GZIP:
			if (strcmp(ext, ".gz") != 0)
				sprintf(name, "%s.%s", to, "gz");
			gzip(1, name, from, size);
			break;
		case XZ:
			if (strcmp(ext, ".xz") != 0)
				sprintf(name, "%s.%s", to, "xz");
			lzma(1, name, from, size);
			break;
		case LZMA:
			if (strcmp(ext, ".lzma") != 0)
				sprintf(name, "%s.%s", to, "lzma");
			lzma(2, name, from, size);
			break;
		case BZIP2:
			if (strcmp(ext, ".bz2") != 0)
				sprintf(name, "%s.%s", to, "bz2");
			bzip2(1, name, from, size);
			break;
		case LZ4:
			if (strcmp(ext, ".lz4") != 0)
				sprintf(name, "%s.%s", to, "lz4");
			lz4(1, name, from, size);
			break;
		case LZ4_LEGACY:
			if (strcmp(ext, ".lz4") != 0)
				sprintf(name, "%s.%s", to, "lz4");
			lz4_legacy(1, name, from, size);
			break;
		default:
			// Unsupported
			return 1;
	}
	return 0;
}

void decomp_file(char *from, const char *to) {
	int ok = 1;
	unsigned char *file;
	size_t size;
	mmap_ro(from, &file, &size);
	file_t type = check_type(file);
	char *ext;
	ext = strrchr(from, '.');
	if (ext == NULL)
		error(1, "Bad filename extention");

	// File type and extension should match
	switch (type) {
		case GZIP:
			if (strcmp(ext, ".gz") != 0)
				ok = 0;
			break;
		case XZ:
			if (strcmp(ext, ".xz") != 0)
				ok = 0;
			break;
		case LZMA:
			if (strcmp(ext, ".lzma") != 0)
				ok = 0;
			break;
		case BZIP2:
			if (strcmp(ext, ".bz2") != 0)
				ok = 0;
			break;
		case LZ4_LEGACY:
		case LZ4:
			if (strcmp(ext, ".lz4") != 0)
				ok = 0;
			break;
		default:
			error(1, "Provided file \'%s\' is not a supported archive format", from);
	}
	if (ok) {
		// If all match, strip out the suffix
		if (!to) {
			*ext = '\0';
			to = from;
		}
		decomp(type, to, file, size);
		if (to == from) {
			*ext = '.';
			unlink(from);
		}
	} else {
		error(1, "Bad filename extention \'%s\'", ext);
	}
	munmap(file, size);
}

void comp_file(const char *method, const char *from, const char *to) {
	file_t type;
	if (strcmp(method, "gzip") == 0) {
		type = GZIP;
	} else if (strcmp(method, "xz") == 0) {
		type = XZ;
	} else if (strcmp(method, "lzma") == 0) {
		type = LZMA;
	} else if (strcmp(method, "lz4") == 0) {
		type = LZ4;
	} else if (strcmp(method, "lz4_legacy") == 0) {
		type = LZ4_LEGACY;
	} else if (strcmp(method, "bzip2") == 0) {
		type = BZIP2;
	} else {
		error(1, "Only support following methods: " SUP_LIST);
	}
	unsigned char *file;
	size_t size;
	mmap_ro(from, &file, &size);
	if (!to)
		to = from;
	comp(type, to, file, size);
	munmap(file, size);
	if (to == from)
		unlink(from);
}

