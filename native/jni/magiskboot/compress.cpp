#include <memory>
#include <functional>

#include <zlib.h>
#include <bzlib.h>
#include <lzma.h>
#include <lz4.h>
#include <lz4frame.h>
#include <lz4hc.h>

#include <utils.hpp>

#include "magiskboot.hpp"
#include "compress.hpp"

using namespace std;

#define bwrite filter_stream::write

constexpr size_t CHUNK = 0x40000;
constexpr size_t LZ4_UNCOMPRESSED = 0x800000;
constexpr size_t LZ4_COMPRESSED = LZ4_COMPRESSBOUND(LZ4_UNCOMPRESSED);

class cpr_stream : public filter_stream {
public:
    using filter_stream::filter_stream;
    using stream::read;
};

class gz_strm : public cpr_stream {
public:
    int write(const void *buf, size_t len) override {
        return len ? write(buf, len, Z_NO_FLUSH) : 0;
    }

    ~gz_strm() override {
        write(nullptr, 0, Z_FINISH);
        switch(mode) {
            case DECODE:
                inflateEnd(&strm);
                break;
            case ENCODE:
                deflateEnd(&strm);
                break;
        }
    }

protected:
    enum mode_t {
        DECODE,
        ENCODE
    } mode;

    gz_strm(mode_t mode, stream_ptr &&base) :
        cpr_stream(std::move(base)), mode(mode), strm{}, outbuf{0} {
        switch(mode) {
            case DECODE:
                inflateInit2(&strm, 15 | 16);
                break;
            case ENCODE:
                deflateInit2(&strm, 9, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
                break;
        }
    }

private:
    z_stream strm;
    uint8_t outbuf[CHUNK];

    int write(const void *buf, size_t len, int flush) {
        int ret = 0;
        strm.next_in = (Bytef *) buf;
        strm.avail_in = len;
        do {
            int code;
            strm.next_out = outbuf;
            strm.avail_out = sizeof(outbuf);
            switch(mode) {
                case DECODE:
                    code = inflate(&strm, flush);
                    break;
                case ENCODE:
                    code = deflate(&strm, flush);
                    break;
            }
            if (code == Z_STREAM_ERROR) {
                LOGW("gzip %s failed (%d)\n", mode ? "encode" : "decode", code);
                return -1;
            }
            ret += bwrite(outbuf, sizeof(outbuf) - strm.avail_out);
        } while (strm.avail_out == 0);
        return ret;
    }
};

class gz_decoder : public gz_strm {
public:
    explicit gz_decoder(stream_ptr &&base) : gz_strm(DECODE, std::move(base)) {};
};

class gz_encoder : public gz_strm {
public:
    explicit gz_encoder(stream_ptr &&base) : gz_strm(ENCODE, std::move(base)) {};
};

class bz_strm : public cpr_stream {
public:
    int write(const void *buf, size_t len) override {
        return len ? write(buf, len, BZ_RUN) : 0;
    }

    ~bz_strm() override {
        switch(mode) {
            case DECODE:
                BZ2_bzDecompressEnd(&strm);
                break;
            case ENCODE:
                write(nullptr, 0, BZ_FINISH);
                BZ2_bzCompressEnd(&strm);
                break;
        }
    }

protected:
    enum mode_t {
        DECODE,
        ENCODE
    } mode;

    bz_strm(mode_t mode, stream_ptr &&base) :
        cpr_stream(std::move(base)), mode(mode), strm{}, outbuf{0} {
        switch(mode) {
            case DECODE:
                BZ2_bzDecompressInit(&strm, 0, 0);
                break;
            case ENCODE:
                BZ2_bzCompressInit(&strm, 9, 0, 0);
                break;
        }
    }

private:
    bz_stream strm;
    char outbuf[CHUNK];

    int write(const void *buf, size_t len, int flush) {
        int ret = 0;
        strm.next_in = (char *) buf;
        strm.avail_in = len;
        do {
            int code;
            strm.avail_out = sizeof(outbuf);
            strm.next_out = outbuf;
            switch(mode) {
                case DECODE:
                    code = BZ2_bzDecompress(&strm);
                    break;
                case ENCODE:
                    code = BZ2_bzCompress(&strm, flush);
                    break;
            }
            if (code < 0) {
                LOGW("bzip2 %s failed (%d)\n", mode ? "encode" : "decode", code);
                return -1;
            }
            ret += bwrite(outbuf, sizeof(outbuf) - strm.avail_out);
        } while (strm.avail_out == 0);
        return ret;
    }
};

class bz_decoder : public bz_strm {
public:
    explicit bz_decoder(stream_ptr &&base) : bz_strm(DECODE, std::move(base)) {};
};

class bz_encoder : public bz_strm {
public:
    explicit bz_encoder(stream_ptr &&base) : bz_strm(ENCODE, std::move(base)) {};
};

class lzma_strm : public cpr_stream {
public:
    int write(const void *buf, size_t len) override {
        return len ? write(buf, len, LZMA_RUN) : 0;
    }

    ~lzma_strm() override {
        write(nullptr, 0, LZMA_FINISH);
        lzma_end(&strm);
    }

protected:
    enum mode_t {
        DECODE,
        ENCODE_XZ,
        ENCODE_LZMA
    } mode;

    lzma_strm(mode_t mode, stream_ptr &&base) :
        cpr_stream(std::move(base)), mode(mode), strm(LZMA_STREAM_INIT), outbuf{0} {
        lzma_options_lzma opt;

        // Initialize preset
        lzma_lzma_preset(&opt, 9);
        lzma_filter filters[] = {
            { .id = LZMA_FILTER_LZMA2, .options = &opt },
            { .id = LZMA_VLI_UNKNOWN, .options = nullptr },
        };

        lzma_ret code;
        switch(mode) {
            case DECODE:
                code = lzma_auto_decoder(&strm, UINT64_MAX, 0);
                break;
            case ENCODE_XZ:
                code = lzma_stream_encoder(&strm, filters, LZMA_CHECK_CRC32);
                break;
            case ENCODE_LZMA:
                code = lzma_alone_encoder(&strm, &opt);
                break;
        }
        if (code != LZMA_OK) {
            LOGE("LZMA initialization failed (%d)\n", code);
        }
    }

private:
    lzma_stream strm;
    uint8_t outbuf[CHUNK];

    int write(const void *buf, size_t len, lzma_action flush) {
        int ret = 0;
        strm.next_in = (uint8_t *) buf;
        strm.avail_in = len;
        do {
            strm.avail_out = sizeof(outbuf);
            strm.next_out = outbuf;
            int code = lzma_code(&strm, flush);
            if (code != LZMA_OK && code != LZMA_STREAM_END) {
                LOGW("LZMA %s failed (%d)\n", mode ? "encode" : "decode", code);
                return -1;
            }
            ret += bwrite(outbuf, sizeof(outbuf) - strm.avail_out);
        } while (strm.avail_out == 0);
        return ret;
    }
};

class lzma_decoder : public lzma_strm {
public:
    explicit lzma_decoder(stream_ptr &&base) : lzma_strm(DECODE, std::move(base)) {}
};

class xz_encoder : public lzma_strm {
public:
    explicit xz_encoder(stream_ptr &&base) : lzma_strm(ENCODE_XZ, std::move(base)) {}
};

class lzma_encoder : public lzma_strm {
public:
    explicit lzma_encoder(stream_ptr &&base) : lzma_strm(ENCODE_LZMA, std::move(base)) {}
};

class LZ4F_decoder : public cpr_stream {
public:
    explicit LZ4F_decoder(stream_ptr &&base) :
        cpr_stream(std::move(base)), ctx(nullptr), outbuf(nullptr), outCapacity(0) {
        LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
    }

    ~LZ4F_decoder() override {
        LZ4F_freeDecompressionContext(ctx);
        delete[] outbuf;
    }

    int write(const void *buf, size_t len) override {
        int ret = 0;
        auto inbuf = reinterpret_cast<const uint8_t *>(buf);
        if (!outbuf)
            read_header(inbuf, len);
        size_t read, write;
        LZ4F_errorCode_t code;
        do {
            read = len;
            write = outCapacity;
            code = LZ4F_decompress(ctx, outbuf, &write, inbuf, &read, nullptr);
            if (LZ4F_isError(code)) {
                LOGW("LZ4F decode error: %s\n", LZ4F_getErrorName(code));
                return -1;
            }
            len -= read;
            inbuf += read;
            ret += bwrite(outbuf, write);
        } while (len != 0 || write != 0);
        return ret;
    }

private:
    LZ4F_decompressionContext_t ctx;
    uint8_t *outbuf;
    size_t outCapacity;

    void read_header(const uint8_t *&in, size_t &size) {
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
};

class LZ4F_encoder : public cpr_stream {
public:
    explicit LZ4F_encoder(stream_ptr &&base) :
        cpr_stream(std::move(base)), ctx(nullptr), outbuf(nullptr), outCapacity(0) {
        LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
    }

    int write(const void *buf, size_t len) override {
        int ret = 0;
        if (!outbuf)
            ret += write_header();
        if (len == 0)
            return 0;
        auto inbuf = reinterpret_cast<const uint8_t *>(buf);
        size_t read, write;
        do {
            read = len > BLOCK_SZ ? BLOCK_SZ : len;
            write = LZ4F_compressUpdate(ctx, outbuf, outCapacity, inbuf, read, nullptr);
            if (LZ4F_isError(write)) {
                LOGW("LZ4F encode error: %s\n", LZ4F_getErrorName(write));
                return -1;
            }
            len -= read;
            inbuf += read;
            ret += bwrite(outbuf, write);
        } while (len != 0);
        return ret;
    }

    ~LZ4F_encoder() override {
        size_t len = LZ4F_compressEnd(ctx, outbuf, outCapacity, nullptr);
        bwrite(outbuf, len);
        LZ4F_freeCompressionContext(ctx);
        delete[] outbuf;
    }

private:
    LZ4F_compressionContext_t ctx;
    uint8_t *outbuf;
    size_t outCapacity;

    static constexpr size_t BLOCK_SZ = 1 << 22;

    int write_header() {
        LZ4F_preferences_t prefs {
            .frameInfo = {
                .blockSizeID = LZ4F_max4MB,
                .blockMode = LZ4F_blockIndependent,
                .contentChecksumFlag = LZ4F_contentChecksumEnabled,
                .blockChecksumFlag = LZ4F_noBlockChecksum,
            },
            .compressionLevel = 9,
            .autoFlush = 1,
        };
        outCapacity = LZ4F_compressBound(BLOCK_SZ, &prefs);
        outbuf = new uint8_t[outCapacity];
        size_t write = LZ4F_compressBegin(ctx, outbuf, outCapacity, &prefs);
        return bwrite(outbuf, write);
    }
};

class LZ4_decoder : public cpr_stream {
public:
    explicit LZ4_decoder(stream_ptr &&base) :
        cpr_stream(std::move(base)), out_buf(new char[LZ4_UNCOMPRESSED]),
        buf(new char[LZ4_COMPRESSED]), init(false), block_sz(0), buf_off(0) {}

    ~LZ4_decoder() override {
        delete[] out_buf;
        delete[] buf;
    }

    int write(const void *in, size_t size) override {
        int ret = 0;
        auto inbuf = static_cast<const char *>(in);
        if (!init) {
            // Skip magic
            inbuf += 4;
            size -= 4;
            init = true;
        }
        for (int consumed; size != 0;) {
            if (block_sz == 0) {
                if (buf_off + size >= sizeof(block_sz)) {
                    consumed = sizeof(block_sz) - buf_off;
                    memcpy(buf + buf_off, inbuf, consumed);
                    memcpy(&block_sz, buf, sizeof(block_sz));
                    buf_off = 0;
                } else {
                    consumed = size;
                    memcpy(buf + buf_off, inbuf, size);
                }
                inbuf += consumed;
                size -= consumed;
            } else if (buf_off + size >= block_sz) {
                consumed = block_sz - buf_off;
                memcpy(buf + buf_off, inbuf, consumed);
                inbuf += consumed;
                size -= consumed;

                int write = LZ4_decompress_safe(buf, out_buf, block_sz, LZ4_UNCOMPRESSED);
                if (write < 0) {
                    LOGW("LZ4HC decompression failure (%d)\n", write);
                    return -1;
                }
                ret += bwrite(out_buf, write);

                // Reset
                buf_off = 0;
                block_sz = 0;
            } else {
                // Copy to internal buffer
                memcpy(buf + buf_off, inbuf, size);
                buf_off += size;
                break;
            }
        }
        return ret;
    }

private:
    char *out_buf;
    char *buf;
    bool init;
    unsigned block_sz;
    int buf_off;
};

class LZ4_encoder : public cpr_stream {
public:
    explicit LZ4_encoder(stream_ptr &&base, bool lg) :
        cpr_stream(std::move(base)), outbuf(new char[LZ4_COMPRESSED]),
        buf(new char[LZ4_UNCOMPRESSED]), init(false), lg(lg), buf_off(0), in_total(0) {}

    int write(const void *in, size_t size) override {
        int ret = 0;
        if (!init) {
            ret += bwrite("\x02\x21\x4c\x18", 4);
            init = true;
        }
        if (size == 0)
            return 0;
        in_total += size;
        const char *inbuf = (const char *) in;
        size_t consumed;
        do {
            if (buf_off + size >= LZ4_UNCOMPRESSED) {
                consumed = LZ4_UNCOMPRESSED - buf_off;
                memcpy(buf + buf_off, inbuf, consumed);
                inbuf += consumed;
                size -= consumed;
                buf_off = LZ4_UNCOMPRESSED;

                if (int written = write_block(); written < 0)
                    return -1;
                else
                    ret += written;

                // Reset buffer
                buf_off = 0;
            } else {
                // Copy to internal buffer
                memcpy(buf + buf_off, inbuf, size);
                buf_off += size;
                size = 0;
            }
        } while (size != 0);
        return ret;
    }

    ~LZ4_encoder() override {
        if (buf_off)
            write_block();
        if (lg)
            bwrite(&in_total, sizeof(in_total));
        delete[] outbuf;
        delete[] buf;
    }

private:
    char *outbuf;
    char *buf;
    bool init;
    bool lg;
    int buf_off;
    unsigned in_total;

    int write_block() {
        int written = LZ4_compress_HC(buf, outbuf, buf_off, LZ4_COMPRESSED, LZ4HC_CLEVEL_MAX);
        if (written == 0) {
            LOGW("LZ4HC compression failure\n");
            return -1;
        }
        bwrite(&written, sizeof(written));
        bwrite(outbuf, written);
        return written + sizeof(written);
    }
};

stream_ptr get_encoder(format_t type, stream_ptr &&base) {
    switch (type) {
        case XZ:
            return make_unique<xz_encoder>(std::move(base));
        case LZMA:
            return make_unique<lzma_encoder>(std::move(base));
        case BZIP2:
            return make_unique<bz_encoder>(std::move(base));
        case LZ4:
            return make_unique<LZ4F_encoder>(std::move(base));
        case LZ4_LEGACY:
            return make_unique<LZ4_encoder>(std::move(base), false);
        case LZ4_LG:
            return make_unique<LZ4_encoder>(std::move(base), true);
        case GZIP:
        default:
            return make_unique<gz_encoder>(std::move(base));
    }
}

stream_ptr get_decoder(format_t type, stream_ptr &&base) {
    switch (type) {
        case XZ:
        case LZMA:
            return make_unique<lzma_decoder>(std::move(base));
        case BZIP2:
            return make_unique<bz_decoder>(std::move(base));
        case LZ4:
            return make_unique<LZ4F_decoder>(std::move(base));
        case LZ4_LEGACY:
        case LZ4_LG:
            return make_unique<LZ4_decoder>(std::move(base));
        case GZIP:
        default:
            return make_unique<gz_decoder>(std::move(base));
    }
}

void decompress(char *infile, const char *outfile) {
    bool in_std = infile == "-"sv;
    bool rm_in = false;

    FILE *in_fp = in_std ? stdin : xfopen(infile, "re");
    stream_ptr strm;

    char buf[4096];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), in_fp))) {
        if (!strm) {
            format_t type = check_fmt(buf, len);

            fprintf(stderr, "Detected format: [%s]\n", fmt2name[type]);

            if (!COMPRESSED(type))
                LOGE("Input file is not a supported compressed type!\n");

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

            FILE *out_fp = outfile == "-"sv ? stdout : xfopen(outfile, "we");
            strm = get_decoder(type, make_unique<fp_stream>(out_fp));
            if (ext) *ext = '.';
        }
        if (strm->write(buf, len) < 0)
            LOGE("Decompression error!\n");
    }

    strm.reset(nullptr);
    fclose(in_fp);

    if (rm_in)
        unlink(infile);
}

void compress(const char *method, const char *infile, const char *outfile) {
    format_t fmt = name2fmt[method];
    if (fmt == UNKNOWN)
        LOGE("Unknown compression method: [%s]\n", method);

    bool in_std = infile == "-"sv;
    bool rm_in = false;

    FILE *in_fp = in_std ? stdin : xfopen(infile, "re");
    FILE *out_fp;

    if (outfile == nullptr) {
        if (in_std) {
            out_fp = stdout;
        } else {
            /* If user does not provide outfile and infile is not
             * STDIN, output to <infile>.[ext] */
            string tmp(infile);
            tmp += fmt2ext[fmt];
            out_fp = xfopen(tmp.data(), "we");
            fprintf(stderr, "Compressing to [%s]\n", tmp.data());
            rm_in = true;
        }
    } else {
        out_fp = outfile == "-"sv ? stdout : xfopen(outfile, "we");
    }

    auto strm = get_encoder(fmt, make_unique<fp_stream>(out_fp));

    char buf[4096];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), in_fp))) {
        if (strm->write(buf, len) < 0)
            LOGE("Compression error!\n");
    };

    strm.reset(nullptr);
    fclose(in_fp);

    if (rm_in)
        unlink(infile);
}
