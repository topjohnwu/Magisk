#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <memory>
#include <functional>

#include <logging.h>
#include <utils.h>

#include "magiskboot.h"
#include "compress.h"

using namespace std;

static bool read_file(FILE *fp, const function<void (void *, size_t)> &fn) {
	char buf[4096];
	size_t len;
	while ((len = fread(buf, 1, sizeof(buf), fp)))
		fn(buf, len);
	return true;
}

void decompress(char *infile, const char *outfile) {
	bool in_std = strcmp(infile, "-") == 0;
	bool rm_in = false;

	FILE *in_file = in_std ? stdin : xfopen(infile, "re");
	int out_fd = -1;
	unique_ptr<Compression> cmp;

	read_file(in_file, [&](void *buf, size_t len) -> void {
		if (out_fd < 0) {
			format_t type = check_fmt(buf, len);
			if (!COMPRESSED(type))
				LOGE("Input file is not a compressed type!\n");

			cmp.reset(get_decoder(type));
			fprintf(stderr, "Detected format: [%s]\n", fmt2name[type]);

			/* If user does not provide outfile, infile has to be either
	 		* <path>.[ext], or '-'. Outfile will be either <path> or '-'.
	 		* If the input does not have proper format, abort */

			char *ext = nullptr;
			if (outfile == nullptr) {
				outfile = infile;
				if (!in_std) {
					ext = strrchr(infile, '.');
					if (ext == nullptr || strcmp(ext, fmt2ext[type]) != 0)
						LOGE("Input file is not a supported type!\n");

					// Strip out extension and remove input
					*ext = '\0';
					rm_in = true;
					fprintf(stderr, "Decompressing to [%s]\n", outfile);
				}
			}

			out_fd = strcmp(outfile, "-") == 0 ?
					STDOUT_FILENO : xopen(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			cmp->set_out(make_unique<FDOutStream>(out_fd));
			if (ext) *ext = '.';
		}
		if (!cmp->write(buf, len))
			LOGE("Decompression error!\n");
	});

	cmp->finalize();
	fclose(in_file);
	close(out_fd);

	if (rm_in)
		unlink(infile);
}

void compress(const char *method, const char *infile, const char *outfile) {
	auto it = name2fmt.find(method);
	if (it == name2fmt.end())
		LOGE("Unsupported compression method: [%s]\n", method);

	unique_ptr<Compression> cmp(get_encoder(it->second));

	bool in_std = strcmp(infile, "-") == 0;
	bool rm_in = false;

	FILE *in_file = in_std ? stdin : xfopen(infile, "re");
	int out_fd;

	if (outfile == nullptr) {
		if (in_std) {
			out_fd = STDOUT_FILENO;
		} else {
			/* If user does not provide outfile and infile is not
			 * STDIN, output to <infile>.[ext] */
			char *tmp = new char[strlen(infile) + 5];
			sprintf(tmp, "%s%s", infile, fmt2ext[it->second]);
			out_fd = xopen(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			fprintf(stderr, "Compressing to [%s]\n", tmp);
			delete[] tmp;
			rm_in = true;
		}
	} else {
		out_fd = strcmp(outfile, "-") == 0 ?
				 STDOUT_FILENO : xopen(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	}

	cmp->set_out(make_unique<FDOutStream>(out_fd));

	read_file(in_file, [&](void *buf, size_t len) -> void {
		if (!cmp->write(buf, len))
			LOGE("Compression error!\n");
	});

	cmp->finalize();
	fclose(in_file);
	close(out_fd);

	if (rm_in)
		unlink(infile);
}

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

bool GZStream::write(const void *in, size_t size) {
	return size ? write(in, size, Z_NO_FLUSH) : true;
}

uint64_t GZStream::finalize() {
	write(nullptr, 0, Z_FINISH);
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

bool GZStream::write(const void *in, size_t size, int flush) {
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
		FilterOutStream::write(outbuf, sizeof(outbuf) - strm.avail_out);
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

bool BZStream::write(const void *in, size_t size) {
	return size ? write(in, size, BZ_RUN) : true;
}

uint64_t BZStream::finalize() {
	if (mode)
		write(nullptr, 0, BZ_FINISH);
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

bool BZStream::write(const void *in, size_t size, int flush) {
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
		FilterOutStream::write(outbuf, sizeof(outbuf) - strm.avail_out);
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

bool LZMAStream::write(const void *in, size_t size) {
	return size ? write(in, size, LZMA_RUN) : true;
}

uint64_t LZMAStream::finalize() {
	write(nullptr, 0, LZMA_FINISH);
	uint64_t total = strm.total_out;
	lzma_end(&strm);
	return total;
}

bool LZMAStream::write(const void *in, size_t size, lzma_action flush) {
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
		FilterOutStream::write(outbuf, sizeof(outbuf) - strm.avail_out);
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

bool LZ4FDecoder::write(const void *in, size_t size) {
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
		FilterOutStream::write(outbuf, write);
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

LZ4FEncoder::LZ4FEncoder() : outbuf(nullptr), outCapacity(0), total(0) {
	LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
}

LZ4FEncoder::~LZ4FEncoder() {
	LZ4F_freeCompressionContext(ctx);
	delete[] outbuf;
}

bool LZ4FEncoder::write(const void *in, size_t size) {
	if (!outbuf)
		write_header();
	if (size == 0)
		return true;
	auto inbuf = (const uint8_t *) in;
	size_t read, write;
	do {
		read = size > BLOCK_SZ ? BLOCK_SZ : size;
		write = LZ4F_compressUpdate(ctx, outbuf, outCapacity, inbuf, read, nullptr);
		if (LZ4F_isError(write)) {
			LOGW("LZ4 encode error: %s\n", LZ4F_getErrorName(write));
			return false;
		}
		size -= read;
		inbuf += read;
		total += write;
		FilterOutStream::write(outbuf, write);
	} while (size != 0);
	return true;
}

uint64_t LZ4FEncoder::finalize() {
	size_t write = LZ4F_compressEnd(ctx, outbuf, outCapacity, nullptr);
	total += write;
	FilterOutStream::write(outbuf, write);
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
	outCapacity = LZ4F_compressBound(BLOCK_SZ, &prefs);
	outbuf = new uint8_t[outCapacity];
	size_t write = LZ4F_compressBegin(ctx, outbuf, outCapacity, &prefs);
	total += write;
	FilterOutStream::write(outbuf, write);
}

LZ4Decoder::LZ4Decoder() : init(false), buf_off(0), total(0), block_sz(0),
outbuf(new char[LZ4_UNCOMPRESSED]), buf(new char[LZ4_COMPRESSED]) {}

LZ4Decoder::~LZ4Decoder() {
	delete[] outbuf;
	delete[] buf;
}

bool LZ4Decoder::write(const void *in, size_t size) {
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
			FilterOutStream::write(outbuf, write);
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

LZ4Encoder::LZ4Encoder() : init(false), buf_off(0), out_total(0), in_total(0),
outbuf(new char[LZ4_COMPRESSED]), buf(new char[LZ4_UNCOMPRESSED]) {}

LZ4Encoder::~LZ4Encoder() {
	delete[] outbuf;
	delete[] buf;
}

bool LZ4Encoder::write(const void *in, size_t size) {
	if (!init) {
		FilterOutStream::write("\x02\x21\x4c\x18", 4);
		init = true;
	}
	if (size == 0)
		return true;
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
			FilterOutStream::write(&write, sizeof(write));
			FilterOutStream::write(outbuf, write);
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
		FilterOutStream::write(&write, sizeof(write));
		FilterOutStream::write(outbuf, write);
		out_total += write + sizeof(write);
	}
	FilterOutStream::write(&in_total, sizeof(in_total));
	return out_total + sizeof(in_total);
}
