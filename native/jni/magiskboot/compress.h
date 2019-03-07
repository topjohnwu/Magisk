#pragma once

#include <zlib.h>
#include <bzlib.h>
#include <lzma.h>
#include <lz4.h>
#include <lz4frame.h>
#include <lz4hc.h>
#include <OutStream.h>

#include "format.h"

#define CHUNK 0x40000

class Compression : public FilterOutStream {
public:
	virtual uint64_t finalize() = 0;
};

class GZStream : public Compression {
public:
	bool write(const void *in, size_t size) override;
	uint64_t finalize() override;

protected:
	explicit GZStream(int mode);

private:
	int mode;
	z_stream strm;
	uint8_t outbuf[CHUNK];

	bool write(const void *in, size_t size, int flush);
};

class GZDecoder : public GZStream {
public:
	GZDecoder() : GZStream(0) {};
};

class GZEncoder : public GZStream {
public:
	GZEncoder() : GZStream(1) {};
};

class BZStream : public Compression {
public:
	bool write(const void *in, size_t size) override;
	uint64_t finalize() override;

protected:
	explicit BZStream(int mode);

private:
	int mode;
	bz_stream strm;
	char outbuf[CHUNK];

	bool write(const void *in, size_t size, int flush);
};

class BZDecoder : public BZStream {
public:
	BZDecoder() : BZStream(0) {};
};

class BZEncoder : public BZStream {
public:
	BZEncoder() : BZStream(1) {};
};

class LZMAStream : public Compression {
public:
	bool write(const void *in, size_t size) override;
	uint64_t finalize() override;

protected:
	explicit LZMAStream(int mode);

private:
	int mode;
	lzma_stream strm;
	uint8_t outbuf[CHUNK];

	bool write(const void *in, size_t size, lzma_action flush);
};

class LZMADecoder : public LZMAStream {
public:
	LZMADecoder() : LZMAStream(0) {}
};

class XZEncoder : public LZMAStream {
public:
	XZEncoder() : LZMAStream(1) {}
};

class LZMAEncoder : public LZMAStream {
public:
	LZMAEncoder() : LZMAStream(2) {}
};

class LZ4FDecoder : public Compression {
public:
	LZ4FDecoder();
	~LZ4FDecoder() override;
	bool write(const void *in, size_t size) override;
	uint64_t finalize() override;

private:
	LZ4F_decompressionContext_t ctx;
	uint8_t *outbuf;
	size_t outCapacity;
	uint64_t total;

	void read_header(const uint8_t *&in, size_t &size);
};

class LZ4FEncoder : public Compression {
public:
	LZ4FEncoder();
	~LZ4FEncoder() override;
	bool write(const void *in, size_t size) override;
	uint64_t finalize() override;

private:
	static constexpr size_t BLOCK_SZ = 1 << 22;
	LZ4F_compressionContext_t ctx;
	uint8_t *outbuf;
	size_t outCapacity;
	uint64_t total;

	void write_header();
};

#define LZ4_UNCOMPRESSED 0x800000
#define LZ4_COMPRESSED   LZ4_COMPRESSBOUND(LZ4_UNCOMPRESSED)

class LZ4Decoder : public Compression {
public:
	LZ4Decoder();
	~LZ4Decoder() override;
	bool write(const void *in, size_t size) override;
	uint64_t finalize() override;

private:
	char *outbuf;
	char *buf;
	bool init;
	unsigned block_sz;
	int buf_off;
	uint64_t total;
};

class LZ4Encoder : public Compression {
public:
	LZ4Encoder();
	~LZ4Encoder() override;
	bool write(const void *in, size_t size) override;
	uint64_t finalize() override;

private:
	char *outbuf;
	char *buf;
	bool init;
	int buf_off;
	uint64_t out_total;
	unsigned in_total;
};

Compression *get_encoder(format_t type);
Compression *get_decoder(format_t type);
void compress(const char *method, const char *infile, const char *outfile);
void decompress(char *infile, const char *outfile);
