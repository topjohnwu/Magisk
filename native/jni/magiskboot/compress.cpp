#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <zlib.h>
#include <lzma.h>
#include <lz4.h>
#include <lz4frame.h>
#include <lz4hc.h>
#include <bzlib.h>

#include "magiskboot.h"
#include "logging.h"
#include "utils.h"

#define CHUNK 0x40000

// Mode: 0 = decode; 1 = encode
size_t gzip(int mode, int fd, const void *buf, size_t size) {
	size_t ret = 0, have, total = 0;
	z_stream strm;
	unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	switch(mode) {
		case 0:
			ret = inflateInit2(&strm, 15 | 16);
			break;
		case 1:
			ret = deflateInit2(&strm, 9, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
			break;
	}

	if (ret != Z_OK)
		LOGE("Unable to init zlib stream\n");

	strm.next_in = (Bytef *) buf;
	strm.avail_in = size;

	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		switch(mode) {
			case 0:
				ret = inflate(&strm, Z_FINISH);
				break;
			case 1:
				ret = deflate(&strm, Z_FINISH);
				break;
		}
		if (ret == Z_STREAM_ERROR)
			LOGE("Error when running gzip\n");
		have = CHUNK - strm.avail_out;
		total += xwrite(fd, out, have);
	} while (strm.avail_out == 0);

	switch(mode) {
		case 0:
			inflateEnd(&strm);
			break;
		case 1:
			deflateEnd(&strm);
			break;
	}
	return total;
}

// Mode: 0 = decode xz/lzma; 1 = encode xz; 2 = encode lzma
size_t lzma(int mode, int fd, const void *buf, size_t size) {
	size_t have, total = 0;
	lzma_ret ret = LZMA_OK;
	lzma_stream strm = LZMA_STREAM_INIT;
	lzma_options_lzma opt;
	unsigned char out[CHUNK];

	// Initialize preset
	lzma_lzma_preset(&opt, 9);
	lzma_filter filters[] = {
		{ .id = LZMA_FILTER_LZMA2, .options = &opt },
		{ .id = LZMA_VLI_UNKNOWN, .options = nullptr },
	};

	switch(mode) {
		case 0:
			ret = lzma_auto_decoder(&strm, UINT64_MAX, 0);
			break;
		case 1:
			ret = lzma_stream_encoder(&strm, filters, LZMA_CHECK_CRC32);
			break;
		case 2:
			ret = lzma_alone_encoder(&strm, &opt);
			break;
	}


	if (ret != LZMA_OK)
		LOGE("Unable to init lzma stream\n");

	strm.next_in = static_cast<const uint8_t *>(buf);
	strm.avail_in = size;

	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		ret = lzma_code(&strm, LZMA_FINISH);
		if (ret != LZMA_OK && ret != LZMA_STREAM_END)
			LOGE("LZMA error %d!\n", ret);
		have = CHUNK - strm.avail_out;
		total += xwrite(fd, out, have);
	} while (strm.avail_out == 0);

	lzma_end(&strm);
	return total;
}

// Mode: 0 = decode; 1 = encode
size_t lz4(int mode, int fd, const uint8_t *buf, size_t size) {
	LZ4F_decompressionContext_t dctx;
	LZ4F_compressionContext_t cctx;
	LZ4F_frameInfo_t info;

	size_t blockSize, outCapacity, avail_in, ret = 0, pos = 0, total = 0;
	size_t have, read;
	uint8_t *out = nullptr;

	// Initialize context
	switch(mode) {
		case 0:
			ret = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
			break;
		case 1:
			ret = LZ4F_createCompressionContext(&cctx, LZ4F_VERSION);
			break;
	}

	if (LZ4F_isError(ret))
		LOGE("Context creation error: %s\n", LZ4F_getErrorName(ret));

	// Allocate out buffer
	blockSize = 1 << 22;
	switch(mode) {
		case 0:
			// Read header
			read = blockSize;
			ret = LZ4F_getFrameInfo(dctx, &info, buf, &read);
			if (LZ4F_isError(ret))
				LOGE("LZ4F_getFrameInfo error: %s\n", LZ4F_getErrorName(ret));
			switch (info.blockSizeID) {
				case LZ4F_default:
				case LZ4F_max64KB:  outCapacity = 1 << 16; break;
				case LZ4F_max256KB: outCapacity = 1 << 18; break;
				case LZ4F_max1MB:   outCapacity = 1 << 20; break;
				case LZ4F_max4MB:   outCapacity = 1 << 22; break;
				default:
					LOGE("Impossible unless more block sizes are allowed\n");
			}
			pos += read;
			break;
		case 1:
			outCapacity = LZ4F_compressFrameBound(blockSize, nullptr);
			break;
	}

	out = new uint8_t[outCapacity];

	// Write header
	if (mode == 1) {
		LZ4F_preferences_t prefs = LZ4F_preferences_t();
		prefs.autoFlush = 1;
		prefs.compressionLevel = 9;
		prefs.frameInfo.blockMode = LZ4F_blockIndependent;
		prefs.frameInfo.blockSizeID = LZ4F_max4MB;
		prefs.frameInfo.blockChecksumFlag = LZ4F_noBlockChecksum;
		prefs.frameInfo.contentChecksumFlag = LZ4F_contentChecksumEnabled;
		have = ret = LZ4F_compressBegin(cctx, out, size, &prefs);
		if (LZ4F_isError(ret))
			LOGE("Failed to start compression: error %s\n", LZ4F_getErrorName(ret));
		total += xwrite(fd, out, have);
	}

	do {
		if (pos + blockSize >= size) {
			avail_in = size - pos;
		} else {
			avail_in = blockSize;
		}

		do {
			switch(mode) {
				case 0:
					have = outCapacity;
					read = avail_in;
					ret = LZ4F_decompress(dctx, out, &have, buf + pos, &read, nullptr);
					break;
				case 1:
					read = avail_in;
					have = ret = LZ4F_compressUpdate(cctx, out, outCapacity, buf + pos, avail_in, nullptr);
					break;
			}
			if (LZ4F_isError(ret))
				LOGE("LZ4 coding error: %s\n", LZ4F_getErrorName(ret));

			total += xwrite(fd, out, have);
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
			have = ret = LZ4F_compressEnd(cctx, out, outCapacity, nullptr);
			if (LZ4F_isError(ret))
				LOGE("Failed to end compression: error %s\n", LZ4F_getErrorName(ret));

			total += xwrite(fd, out, have);

			LZ4F_freeCompressionContext(cctx);
			break;
	}

	delete[] out;
	return total;
}

// Mode: 0 = decode; 1 = encode
size_t bzip2(int mode, int fd, const void* buf, size_t size) {
	size_t ret = 0, have, total = 0;
	bz_stream strm;
	char out[CHUNK];

	strm.bzalloc = nullptr;
	strm.bzfree = nullptr;
	strm.opaque = nullptr;

	switch(mode) {
		case 0:
			ret = BZ2_bzDecompressInit(&strm, 0, 0);
			break;
		case 1:
			ret = BZ2_bzCompressInit(&strm, 9, 0, 0);
			break;
	}

	if (ret != BZ_OK)
		LOGE("Unable to init bzlib stream\n");

	strm.next_in = (char *) buf;
	strm.avail_in = size;

	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		switch(mode) {
			case 0:
				ret = BZ2_bzDecompress(&strm);
				break;
			case 1:
				ret = BZ2_bzCompress(&strm, BZ_FINISH);
				break;
		}
		have = CHUNK - strm.avail_out;
		total += xwrite(fd, out, have);
	} while (strm.avail_out == 0);

	switch(mode) {
		case 0:
			BZ2_bzDecompressEnd(&strm);
			break;
		case 1:
			BZ2_bzCompressEnd(&strm);
			break;
	}
	return total;
}

#define LZ4_LEGACY_BLOCKSIZE  0x800000

// Mode: 0 = decode; 1 = encode
size_t lz4_legacy(int mode, int fd, const uint8_t *buf, size_t size) {
	size_t pos = 0;
	int have;
	char *out;
	unsigned block_size, insize, total = 0;

	switch(mode) {
		case 0:
			out = new char[LZ4_LEGACY_BLOCKSIZE];
			// Skip magic
			pos += 4;
			break;
		case 1:
			out = new char[LZ4_COMPRESSBOUND(LZ4_LEGACY_BLOCKSIZE)];
			// Write magic
			total += xwrite(fd, "\x02\x21\x4c\x18", 4);
			break;
	}

	do {
		switch(mode) {
			case 0:
				// Read block size
				block_size = *(unsigned *)(buf + pos);
				pos += 4;
				if (block_size > LZ4_COMPRESSBOUND(LZ4_LEGACY_BLOCKSIZE))
					goto done;
				have = LZ4_decompress_safe((const char *) buf + pos, out, block_size, LZ4_LEGACY_BLOCKSIZE);
				if (have < 0)
					LOGE("Cannot decode lz4_legacy block\n");
				pos += block_size;
				break;
			case 1:
				if (pos + LZ4_LEGACY_BLOCKSIZE >= size)
					insize = size - pos;
				else
					insize = LZ4_LEGACY_BLOCKSIZE;
				have = LZ4_compress_HC((const char *) buf + pos, out, insize, LZ4_COMPRESSBOUND(LZ4_LEGACY_BLOCKSIZE), 9);
				if (have == 0)
					LOGE("lz4_legacy compression error\n");
				pos += insize;
				// Write block size
				total += xwrite(fd, &have, sizeof(have));
				break;
		}
		// Write main data
		total += xwrite(fd, out, have);
	} while(pos < size);

done:
	if (mode == 1) {
		// Append original size to output
		unsigned uncomp = size;
		xwrite(fd, &uncomp, sizeof(uncomp));
	}
	delete[] out;
	return total;
}

long long decompress(format_t type, int fd, const void *from, size_t size) {
	const uint8_t *buf = (uint8_t *) from;
	switch (type) {
		case GZIP:
			return gzip(0, fd, buf, size);
		case XZ:
			return lzma(0, fd, buf, size);
		case LZMA:
			return lzma(0, fd, buf, size);
		case BZIP2:
			return bzip2(0, fd, buf, size);
		case LZ4:
			return lz4(0, fd, buf, size);
		case LZ4_LEGACY:
			return lz4_legacy(0, fd, buf, size);
		default:
			// Unsupported
			return -1;
	}
}

long long compress(format_t type, int fd, const void *from, size_t size) {
	const uint8_t *buf = (uint8_t *) from;
	switch (type) {
		case GZIP:
			return gzip(1, fd, buf, size);
		case XZ:
			return lzma(1, fd, buf, size);
		case LZMA:
			return lzma(2, fd, buf, size);
		case BZIP2:
			return bzip2(1, fd, buf, size);
		case LZ4:
			return lz4(1, fd, buf, size);
		case LZ4_LEGACY:
			return lz4_legacy(1, fd, buf, size);
		default:
			// Unsupported
			return -1;
	}
}

/*
 * Below are utility functions for commandline
 */

void decompress(char *from, const char *to) {
	int strip = 1;
	void *file;
	size_t size = 0;
	if (strcmp(from, "-") == 0)
		stream_full_read(STDIN_FILENO, &file, &size);
	else
		mmap_ro(from, &file, &size);
	format_t type = check_fmt(file, size);
	char *ext;
	ext = strrchr(from, '.');
	if (to == nullptr)
		to = from;
	if (ext != nullptr) {
		// Strip out a matched file extension
		switch (type) {
		case GZIP:
			if (strcmp(ext, ".gz") != 0)
				strip = 0;
			break;
		case XZ:
			if (strcmp(ext, ".xz") != 0)
				strip = 0;
			break;
		case LZMA:
			if (strcmp(ext, ".lzma") != 0)
				strip = 0;
			break;
		case BZIP2:
			if (strcmp(ext, ".bz2") != 0)
				strip = 0;
			break;
		case LZ4_LEGACY:
		case LZ4:
			if (strcmp(ext, ".lz4") != 0)
				strip = 0;
			break;
		default:
			LOGE("Provided file \'%s\' is not a supported archive format\n", from);
		}
		if (strip)
			*ext = '\0';
	}

	int fd;

	if (strcmp(to, "-") == 0) {
		fd = STDOUT_FILENO;
	} else {
		fd = creat(to, 0644);
		fprintf(stderr, "Decompressing to [%s]\n", to);
	}

	decompress(type, fd, file, size);
	close(fd);
	if (to == from && ext != nullptr) {
		*ext = '.';
		unlink(from);
	}
	if (strcmp(from, "-") == 0)
		free(file);
	else
		munmap(file, size);
}

void compress(const char *method, const char *from, const char *to) {
	format_t type;
	const char *ext;
	char dest[PATH_MAX];
	if (strcmp(method, "gzip") == 0) {
		type = GZIP;
		ext = "gz";
	} else if (strcmp(method, "xz") == 0) {
		type = XZ;
		ext = "xz";
	} else if (strcmp(method, "lzma") == 0) {
		type = LZMA;
		ext = "lzma";
	} else if (strcmp(method, "lz4") == 0) {
		type = LZ4;
		ext = "lz4";
	} else if (strcmp(method, "lz4_legacy") == 0) {
		type = LZ4_LEGACY;
		ext = "lz4";
	} else if (strcmp(method, "bzip2") == 0) {
		type = BZIP2;
		ext = "bz2";
	} else {
		fprintf(stderr, "Only support following methods: ");
		for (int i = 0; SUP_LIST[i]; ++i)
			fprintf(stderr, "%s ", SUP_LIST[i]);
		fprintf(stderr, "\n");
		exit(1);
	}
	void *file;
	size_t size;
	if (strcmp(from, "-") == 0)
		stream_full_read(STDIN_FILENO, &file, &size);
	else
		mmap_ro(from, &file, &size);
	if (to == nullptr) {
		if (strcmp(from, "-") == 0)
			strcpy(dest, "-");
		else
			snprintf(dest, sizeof(dest), "%s.%s", from, ext);
	} else
		strcpy(dest, to);
	int fd;
	if (strcmp(dest, "-") == 0) {
		fd = STDOUT_FILENO;
	} else {
		fd = creat(dest, 0644);
		fprintf(stderr, "Compressing to [%s]\n", dest);
	}
	compress(type, fd, file, size);
	close(fd);
	if (strcmp(from, "-") == 0)
		free(file);
	else
		munmap(file, size);
	if (to == nullptr)
		unlink(from);
}

