#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <logging.h>
#include <utils.h>

#include "magiskboot.h"
#include "compress.h"

int64_t decompress(format_t type, int fd, const void *from, size_t size) {
	auto cmp = get_decoder(type);
	int64_t ret = cmp->one_step(fd, from, size);
	delete cmp;
	return ret;
}

int64_t compress(format_t type, int fd, const void *from, size_t size) {
	auto cmp = get_encoder(type);
	int64_t ret = cmp->one_step(fd, from, size);
	delete cmp;
	return ret;
}

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


/* Compression Streams */

Compression *get_encoder(format_t type) {
	switch (type) {
		case XZ:
			return new XZEncoder();
		case LZMA:
			return new LZMAEncoder();
		case BZIP2:
			return new BZEncoder();
		case LZ4:
			return new LZ4FEncoder();
		case LZ4_LEGACY:
			return new LZ4Encoder();
		case GZIP:
		default:
			return new GZEncoder();
	}
}

Compression *get_decoder(format_t type) {
	switch (type) {
		case XZ:
		case LZMA:
			return new LZMADecoder();
		case BZIP2:
			return new BZDecoder();
		case LZ4:
			return new LZ4FDecoder();
		case LZ4_LEGACY:
			return new LZ4Decoder();
		case GZIP:
		default:
			return new GZDecoder();
	}
}

Compression::Compression() : fn([](auto, auto) -> void {}) {}

void Compression::set_outfn(std::function<void(const void *, size_t)> &&fn) {
	this->fn = std::move(fn);
}

void Compression::set_outfd(int fd) {
	fn = [=](const void *out, size_t len) -> void {
		xwrite(fd, out, len);
	};
}

int64_t Compression::one_step(int outfd, const void *in, size_t size) {
	set_outfd(outfd);
	if (!update(in, size))
		return -1;
	return finalize();
}

GZStream::GZStream(int mode) : mode(mode), strm({}) {
	switch(mode) {
		case 0:
			inflateInit2(&strm, 15 | 16);
			break;
		case 1:
			deflateInit2(&strm, 9, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
			break;
	}
}

bool GZStream::update(const void *in, size_t size) {
	return update(in, size, Z_NO_FLUSH);
}

uint64_t GZStream::finalize() {
	update(nullptr, 0, Z_FINISH);
	uint64_t total = strm.total_out;
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

bool GZStream::update(const void *in, size_t size, int flush) {
	int ret;
	strm.next_in = (Bytef *) in;
	strm.avail_in = size;
	do {
		strm.next_out = outbuf;
		strm.avail_out = sizeof(outbuf);
		switch(mode) {
			case 0:
				ret = inflate(&strm, flush);
				break;
			case 1:
				ret = deflate(&strm, flush);
				break;
		}
		if (ret == Z_STREAM_ERROR) {
			LOGW("Gzip %s failed (%d)\n", mode ? "encode" : "decode", ret);
			return false;
		}
		fn(outbuf, sizeof(outbuf) - strm.avail_out);
	} while (strm.avail_out == 0);
	return true;
}

BZStream::BZStream(int mode) : mode(mode), strm({}) {
	switch(mode) {
		case 0:
			BZ2_bzDecompressInit(&strm, 0, 0);
			break;
		case 1:
			BZ2_bzCompressInit(&strm, 9, 0, 0);
			break;
	}
}

bool BZStream::update(const void *in, size_t size) {
	return update(in, size, BZ_RUN);
}

uint64_t BZStream::finalize() {
	update(nullptr, 0, BZ_FINISH);
	uint64_t total = ((uint64_t) strm.total_out_hi32 << 32) + strm.total_out_lo32;
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

bool BZStream::update(const void *in, size_t size, int flush) {
	int ret;
	strm.next_in = (char *) in;
	strm.avail_in = size;
	do {
		strm.avail_out = sizeof(outbuf);
		strm.next_out = outbuf;
		switch(mode) {
			case 0:
				ret = BZ2_bzDecompress(&strm);
				break;
			case 1:
				ret = BZ2_bzCompress(&strm, flush);
				break;
		}
		if (ret < 0) {
			LOGW("Bzip2 %s failed (%d)\n", mode ? "encode" : "decode", ret);
			return false;
		}
		fn(outbuf, sizeof(outbuf) - strm.avail_out);
	} while (strm.avail_out == 0);
	return true;
}

LZMAStream::LZMAStream(int mode) : mode(mode), strm(LZMA_STREAM_INIT) {
	lzma_options_lzma opt;
	int ret;

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
}

bool LZMAStream::update(const void *in, size_t size) {
	return update(in, size, LZMA_RUN);
}

uint64_t LZMAStream::finalize() {
	update(nullptr, 0, LZMA_FINISH);
	uint64_t total = strm.total_out;
	lzma_end(&strm);
	return total;
}

bool LZMAStream::update(const void *in, size_t size, lzma_action flush) {
	int ret;
	strm.next_in = (uint8_t *) in;
	strm.avail_in = size;
	do {
		strm.avail_out = sizeof(outbuf);
		strm.next_out = outbuf;
		ret = lzma_code(&strm, flush);
		if (ret != LZMA_OK && ret != LZMA_STREAM_END) {
			LOGW("LZMA %s failed (%d)\n", mode ? "encode" : "decode", ret);
			return false;
		}
		fn(outbuf, sizeof(outbuf) - strm.avail_out);
	} while (strm.avail_out == 0);
	return true;
}

LZ4FDecoder::LZ4FDecoder() : outbuf(nullptr), total(0) {
	LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
}

LZ4FDecoder::~LZ4FDecoder() {
	LZ4F_freeDecompressionContext(ctx);
	delete[] outbuf;
}

bool LZ4FDecoder::update(const void *in, size_t size) {
	auto inbuf = (const uint8_t *) in;
	if (!outbuf)
		read_header(inbuf, size);
	size_t read, write;
	LZ4F_errorCode_t ret;
	do {
		read = size;
		write = outCapacity;
		ret = LZ4F_decompress(ctx, outbuf, &write, inbuf, &read, nullptr);
		if (LZ4F_isError(ret)) {
			LOGW("LZ4 decode error: %s\n", LZ4F_getErrorName(ret));
			return false;
		}
		size -= read;
		inbuf += read;
		total += write;
		fn(outbuf, write);
	} while (size != 0 || write != 0);
	return true;
}

uint64_t LZ4FDecoder::finalize() {
	return total;
}

void LZ4FDecoder::read_header(const uint8_t *&in, size_t &size) {
	size_t read = size;
	LZ4F_frameInfo_t info;
	LZ4F_getFrameInfo(ctx, &info, in, &read);
	switch (info.blockSizeID) {
		case LZ4F_default:
		case LZ4F_max64KB:  outCapacity = 1 << 16; break;
		case LZ4F_max256KB: outCapacity = 1 << 18; break;
		case LZ4F_max1MB:   outCapacity = 1 << 20; break;
		case LZ4F_max4MB:   outCapacity = 1 << 22; break;
	}
	outbuf = new uint8_t[outCapacity];
	in += read;
	size -= read;
}

LZ4FEncoder::LZ4FEncoder() : outbuf(nullptr), outCapacity(0) {
	LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
}

LZ4FEncoder::~LZ4FEncoder() {
	LZ4F_freeCompressionContext(ctx);
	delete[] outbuf;
}

bool LZ4FEncoder::update(const void *in, size_t size) {
	if (!outbuf)
		write_header();
	auto inbuf = (const uint8_t *) in;
	size_t read, write;
	do {
		read = size > CHUNK ? CHUNK : size;
		write = LZ4F_compressUpdate(ctx, outbuf, outCapacity, inbuf, read, nullptr);
		if (LZ4F_isError(write)) {
			LOGW("LZ4 encode error: %s\n", LZ4F_getErrorName(write));
			return false;
		}
		size -= read;
		inbuf += read;
		total += write;
		fn(outbuf, write);
	} while (size != 0);
	return true;
}

uint64_t LZ4FEncoder::finalize() {
	size_t write = LZ4F_compressEnd(ctx, outbuf, outCapacity, nullptr);
	total += write;
	fn(outbuf, write);
	return total;
}

void LZ4FEncoder::write_header() {
	LZ4F_preferences_t prefs {
		.autoFlush = 1,
		.compressionLevel = 9,
		.frameInfo = {
			.blockMode = LZ4F_blockIndependent,
			.blockSizeID = LZ4F_max4MB,
			.blockChecksumFlag = LZ4F_noBlockChecksum,
			.contentChecksumFlag = LZ4F_contentChecksumEnabled
		}
	};
	outCapacity = LZ4F_compressBound(CHUNK, &prefs);
	outbuf = new uint8_t[outCapacity];
	size_t write = LZ4F_compressBegin(ctx, outbuf, outCapacity, &prefs);
	total += write;
	fn(outbuf, write);
}

LZ4Decoder::LZ4Decoder() : init(false), buf_off(0), total(0), block_sz(0) {
	outbuf = new char[LZ4_UNCOMPRESSED];
	buf = new char[LZ4_COMPRESSED];
}

LZ4Decoder::~LZ4Decoder() {
	delete[] outbuf;
	delete[] buf;
}

bool LZ4Decoder::update(const void *in, size_t size) {
	const char *inbuf = (const char *) in;
	if (!init) {
		// Skip magic
		inbuf += 4;
		size -= 4;
		init = true;
	}
	int write;
	size_t consumed;
	do {
		if (block_sz == 0) {
			block_sz = *((unsigned *) inbuf);
			inbuf += sizeof(unsigned);
			size -= sizeof(unsigned);
		} else if (buf_off + size >= block_sz) {
			consumed = block_sz - buf_off;
			memcpy(buf + buf_off, inbuf, consumed);
			inbuf += consumed;
			size -= consumed;

			write = LZ4_decompress_safe(buf, outbuf, block_sz, LZ4_UNCOMPRESSED);
			if (write < 0) {
				LOGW("LZ4HC decompression failure (%d)\n", write);
				return false;
			}
			fn(outbuf, write);
			total += write;

			// Reset
			buf_off = 0;
			block_sz = 0;
		} else {
			// Copy to internal buffer
			memcpy(buf + buf_off, inbuf, size);
			buf_off += size;
			size = 0;
		}
	} while (size != 0);
	return true;
}

uint64_t LZ4Decoder::finalize() {
	return total;
}

LZ4Encoder::LZ4Encoder() : init(false), buf_off(0), out_total(0), in_total(0) {
	outbuf = new char[LZ4_COMPRESSED];
	buf = new char[LZ4_UNCOMPRESSED];
}

LZ4Encoder::~LZ4Encoder() {
	delete[] outbuf;
	delete[] buf;
}

bool LZ4Encoder::update(const void *in, size_t size) {
	if (!init) {
		fn("\x02\x21\x4c\x18", 4);
		init = true;
	}
	in_total += size;
	const char *inbuf = (const char *) in;
	size_t consumed;
	int write;
	do {
		if (buf_off + size >= LZ4_UNCOMPRESSED) {
			consumed = LZ4_UNCOMPRESSED - buf_off;
			memcpy(buf + buf_off, inbuf, consumed);
			inbuf += consumed;
			size -= consumed;

			write = LZ4_compress_HC(buf, outbuf, LZ4_UNCOMPRESSED, LZ4_COMPRESSED, 9);
			if (write == 0) {
				LOGW("LZ4HC compression failure\n");
				return false;
			}
			fn(&write, sizeof(write));
			fn(outbuf, write);
			out_total += write + sizeof(write);

			// Reset buffer
			buf_off = 0;
		} else {
			// Copy to internal buffer
			memcpy(buf + buf_off, inbuf, size);
			buf_off += size;
			size = 0;
		}
	} while (size != 0);
	return true;
}

uint64_t LZ4Encoder::finalize() {
	if (buf_off) {
		int write = LZ4_compress_HC(buf, outbuf, buf_off, LZ4_COMPRESSED, 9);
		fn(&write, sizeof(write));
		fn(outbuf, write);
		out_total += write + sizeof(write);
	}
	fn(&in_total, sizeof(in_total));
	return out_total + sizeof(in_total);
}
