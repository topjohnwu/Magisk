#include <memory>
#include <functional>

#include <zlib.h>
#include <bzlib.h>
#include <lzma.h>
#include <lz4.h>
#include <lz4frame.h>
#include <lz4hc.h>
#include <zopfli/util.h>
#include <zopfli/deflate.h>

#include <base.hpp>

#include "magiskboot.hpp"
#include "compress.hpp"

using namespace std;

#define bwrite this->base->write

constexpr size_t CHUNK = 0x40000;
constexpr size_t LZ4_UNCOMPRESSED = 0x800000;
constexpr size_t LZ4_COMPRESSED = LZ4_COMPRESSBOUND(LZ4_UNCOMPRESSED);

class gz_strm : public filter_out_stream {
public:
    bool write(const void *buf, size_t len) override {
        return len == 0 || do_write(buf, len, Z_NO_FLUSH);
    }

    ~gz_strm() override {
        do_write(nullptr, 0, Z_FINISH);
        switch(mode) {
        case DECODE:
            inflateEnd(&strm);
            break;
        case ENCODE:
            deflateEnd(&strm);
            break;
        default:
            break;
        }
    }

protected:
    enum mode_t {
        DECODE,
        ENCODE,
        WAIT,
        COPY
    } mode;

    gz_strm(mode_t mode, out_strm_ptr &&base) :
            filter_out_stream(std::move(base)), mode(mode), strm{}, outbuf{0} {
        switch(mode) {
        case DECODE:
            inflateInit2(&strm, 15 | 16);
            break;
        case ENCODE:
            deflateInit2(&strm, 9, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
            break;
        default:
            break;
        }
    }

private:
    z_stream strm;
    uint8_t outbuf[CHUNK];

    bool do_write(const void *buf, size_t len, int flush) {
        if (mode == WAIT) {
            if (len == 0) return true;
            Bytef b[1] = {0x1f};
            if (*(Bytef *)buf == 0x8b) {
                mode = DECODE;
                inflateReset(&strm);
                strm.next_in = b;
                strm.avail_in = 1;
                inflate(&strm, flush);
            } else {
                mode = COPY;
                return true;
            }
        }
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
                case COPY:
                    return true;
                default:
                    // should have been handled
                    return false;
            }
            if (code == Z_STREAM_ERROR) {
                LOGW("gzip %s failed (%d)\n", mode ? "encode" : "decode", code);
                return false;
            }
            if (!bwrite(outbuf, sizeof(outbuf) - strm.avail_out))
                return false;
            if (mode == DECODE && code == Z_STREAM_END) {
                if (strm.avail_in > 1) {
                    if (strm.next_in[0] == 0x1f && strm.next_in[1] == 0x8b) {
                        // There is still data in the stream, we need to reset the stream
                        // and continue decoding
                        inflateReset(&strm);
                        strm.avail_out = 0;
                        continue;
                    }
                } else if (strm.avail_in == 1) {
                    if (strm.next_in[0] == 0x1f) {
                        // If there is only one byte left, we need to wait for the next byte
                        // to determine if it is a gzip header
                        mode = WAIT;
                        return true;
                    }
                } else {
                    // The next inflate won't consume any data but fallback
                    // to the previous two conditions
                    return true;
                }
                // There is still data in the stream, we need to copy it
                mode = COPY;
                return true;
            }
        } while (strm.avail_out == 0);
        return true;
    }
};

class gz_decoder : public gz_strm {
public:
    explicit gz_decoder(out_strm_ptr &&base) : gz_strm(DECODE, std::move(base)) {};
};

class gz_encoder : public gz_strm {
public:
    explicit gz_encoder(out_strm_ptr &&base) : gz_strm(ENCODE, std::move(base)) {};
};

class zopfli_encoder : public chunk_out_stream {
public:
    explicit zopfli_encoder(out_strm_ptr &&base) :
        chunk_out_stream(std::move(base), ZOPFLI_MASTER_BLOCK_SIZE),
        zo{}, out(nullptr), outsize(0), crc(crc32_z(0L, Z_NULL, 0)), in_total(0), bp(0) {
        ZopfliInitOptions(&zo);

        // This config is already better than gzip -9
        zo.numiterations = 1;
        zo.blocksplitting = 0;

        ZOPFLI_APPEND_DATA(31, &out, &outsize);  /* ID1 */
        ZOPFLI_APPEND_DATA(139, &out, &outsize); /* ID2 */
        ZOPFLI_APPEND_DATA(8, &out, &outsize);   /* CM */
        ZOPFLI_APPEND_DATA(0, &out, &outsize);   /* FLG */
        /* MTIME */
        ZOPFLI_APPEND_DATA(0, &out, &outsize);
        ZOPFLI_APPEND_DATA(0, &out, &outsize);
        ZOPFLI_APPEND_DATA(0, &out, &outsize);
        ZOPFLI_APPEND_DATA(0, &out, &outsize);

        ZOPFLI_APPEND_DATA(2, &out, &outsize);  /* XFL, 2 indicates best compression. */
        ZOPFLI_APPEND_DATA(3, &out, &outsize);  /* OS follows Unix conventions. */
    }

    ~zopfli_encoder() override {
        finalize();

        /* CRC */
        ZOPFLI_APPEND_DATA(crc % 256, &out, &outsize);
        ZOPFLI_APPEND_DATA((crc >> 8) % 256, &out, &outsize);
        ZOPFLI_APPEND_DATA((crc >> 16) % 256, &out, &outsize);
        ZOPFLI_APPEND_DATA((crc >> 24) % 256, &out, &outsize);

        /* ISIZE */
        ZOPFLI_APPEND_DATA(in_total % 256, &out, &outsize);
        ZOPFLI_APPEND_DATA((in_total >> 8) % 256, &out, &outsize);
        ZOPFLI_APPEND_DATA((in_total >> 16) % 256, &out, &outsize);
        ZOPFLI_APPEND_DATA((in_total >> 24) % 256, &out, &outsize);

        bwrite(out, outsize);
        free(out);
    }

protected:
    bool write_chunk(const void *buf, size_t len, bool final) override {
        auto in = static_cast<const unsigned char *>(buf);

        in_total += len;
        crc = crc32_z(crc, in, len);

        ZopfliDeflatePart(&zo, 2, final, in, 0, len, &bp, &out, &outsize);

        // ZOPFLI_APPEND_DATA is extremely dumb, so we always preserve the
        // last byte to make sure that realloc is used instead of malloc
        if (!bwrite(out, outsize - 1))
            return false;
        out[0] = out[outsize - 1];
        outsize = 1;

        return true;
    }

private:
    ZopfliOptions zo;
    unsigned char *out;
    size_t outsize;
    unsigned long crc;
    uint32_t in_total;
    unsigned char bp;
};

class bz_strm : public filter_out_stream {
public:
    bool write(const void *buf, size_t len) override {
        return len == 0 || do_write(buf, len, BZ_RUN);
    }

    ~bz_strm() override {
        switch(mode) {
            case DECODE:
                BZ2_bzDecompressEnd(&strm);
                break;
            case ENCODE:
                do_write(nullptr, 0, BZ_FINISH);
                BZ2_bzCompressEnd(&strm);
                break;
        }
    }

protected:
    enum mode_t {
        DECODE,
        ENCODE
    } mode;

    bz_strm(mode_t mode, out_strm_ptr &&base) :
            filter_out_stream(std::move(base)), mode(mode), strm{}, outbuf{0} {
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

    bool do_write(const void *buf, size_t len, int flush) {
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
                return false;
            }
            if (!bwrite(outbuf, sizeof(outbuf) - strm.avail_out))
                return false;
            if (code == BZ_STREAM_END)
                return true;
        } while (strm.avail_out == 0);
        return true;
    }
};

class bz_decoder : public bz_strm {
public:
    explicit bz_decoder(out_strm_ptr &&base) : bz_strm(DECODE, std::move(base)) {};
};

class bz_encoder : public bz_strm {
public:
    explicit bz_encoder(out_strm_ptr &&base) : bz_strm(ENCODE, std::move(base)) {};
};

class lzma_strm : public filter_out_stream {
public:
    bool write(const void *buf, size_t len) override {
        return len == 0 || do_write(buf, len, LZMA_RUN);
    }

    ~lzma_strm() override {
        do_write(nullptr, 0, LZMA_FINISH);
        lzma_end(&strm);
    }

protected:
    enum mode_t {
        DECODE,
        ENCODE_XZ,
        ENCODE_LZMA
    } mode;

    lzma_strm(mode_t mode, out_strm_ptr &&base) :
            filter_out_stream(std::move(base)), mode(mode), strm(LZMA_STREAM_INIT), outbuf{0} {
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

    bool do_write(const void *buf, size_t len, lzma_action flush) {
        strm.next_in = (uint8_t *) buf;
        strm.avail_in = len;
        do {
            strm.avail_out = sizeof(outbuf);
            strm.next_out = outbuf;
            int code = lzma_code(&strm, flush);
            if (code != LZMA_OK && code != LZMA_STREAM_END) {
                LOGW("LZMA %s failed (%d)\n", mode ? "encode" : "decode", code);
                return false;
            }
            if (!bwrite(outbuf, sizeof(outbuf) - strm.avail_out))
                return false;
        } while (strm.avail_out == 0);
        return true;
    }
};

class lzma_decoder : public lzma_strm {
public:
    explicit lzma_decoder(out_strm_ptr &&base) : lzma_strm(DECODE, std::move(base)) {}
};

class xz_encoder : public lzma_strm {
public:
    explicit xz_encoder(out_strm_ptr &&base) : lzma_strm(ENCODE_XZ, std::move(base)) {}
};

class lzma_encoder : public lzma_strm {
public:
    explicit lzma_encoder(out_strm_ptr &&base) : lzma_strm(ENCODE_LZMA, std::move(base)) {}
};

class LZ4F_decoder : public filter_out_stream {
public:
    explicit LZ4F_decoder(out_strm_ptr &&base) :
            filter_out_stream(std::move(base)), ctx(nullptr), outbuf(nullptr), outCapacity(0) {
        LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
    }

    ~LZ4F_decoder() override {
        LZ4F_freeDecompressionContext(ctx);
        delete[] outbuf;
    }

    bool write(const void *buf, size_t len) override {
        auto in = reinterpret_cast<const uint8_t *>(buf);
        if (!outbuf) {
            size_t read = len;
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
            len -= read;
        }
        size_t read, write;
        LZ4F_errorCode_t code;
        do {
            read = len;
            write = outCapacity;
            code = LZ4F_decompress(ctx, outbuf, &write, in, &read, nullptr);
            if (LZ4F_isError(code)) {
                LOGW("LZ4F decode error: %s\n", LZ4F_getErrorName(code));
                return false;
            }
            len -= read;
            in += read;
            if (!bwrite(outbuf, write))
                return false;
        } while (len != 0 || write != 0);
        return true;
    }

private:
    LZ4F_decompressionContext_t ctx;
    uint8_t *outbuf;
    size_t outCapacity;
};

class LZ4F_encoder : public filter_out_stream {
public:
    explicit LZ4F_encoder(out_strm_ptr &&base) :
            filter_out_stream(std::move(base)), ctx(nullptr), out_buf(nullptr), outCapacity(0) {
        LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
    }

    bool write(const void *buf, size_t len) override {
        if (!out_buf) {
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
            out_buf = new uint8_t[outCapacity];
            size_t write = LZ4F_compressBegin(ctx, out_buf, outCapacity, &prefs);
            if (!bwrite(out_buf, write))
                return false;
        }
        if (len == 0)
            return true;

        auto in = reinterpret_cast<const uint8_t *>(buf);
        size_t read, write;
        do {
            read = len > BLOCK_SZ ? BLOCK_SZ : len;
            write = LZ4F_compressUpdate(ctx, out_buf, outCapacity, in, read, nullptr);
            if (LZ4F_isError(write)) {
                LOGW("LZ4F encode error: %s\n", LZ4F_getErrorName(write));
                return false;
            }
            len -= read;
            in += read;
            if (!bwrite(out_buf, write))
                return false;
        } while (len != 0);
        return true;
    }

    ~LZ4F_encoder() override {
        size_t len = LZ4F_compressEnd(ctx, out_buf, outCapacity, nullptr);
        if (LZ4F_isError(len)) {
            LOGE("LZ4F end of frame error: %s\n", LZ4F_getErrorName(len));
        } else if (!bwrite(out_buf, len)) {
            LOGE("LZ4F end of frame error: I/O error\n");
        }
        LZ4F_freeCompressionContext(ctx);
        delete[] out_buf;
    }

private:
    LZ4F_compressionContext_t ctx;
    uint8_t *out_buf;
    size_t outCapacity;

    static constexpr size_t BLOCK_SZ = 1 << 22;
};

class LZ4_decoder : public chunk_out_stream {
public:
    explicit LZ4_decoder(out_strm_ptr &&base) :
        chunk_out_stream(std::move(base), LZ4_COMPRESSED, sizeof(block_sz)),
        out_buf(new char[LZ4_UNCOMPRESSED]), block_sz(0) {}

    ~LZ4_decoder() override {
        finalize();
        delete[] out_buf;
    }

protected:
    bool write_chunk(const void *buf, size_t len, bool) override {
        // This is an error
        if (len != chunk_sz)
            return false;

        auto in = reinterpret_cast<const char *>(buf);

        if (block_sz == 0) {
            memcpy(&block_sz, in, sizeof(block_sz));
            if (block_sz == 0x184C2102) {
                // This is actually the lz4 magic, read the next 4 bytes
                block_sz = 0;
                chunk_sz = sizeof(block_sz);
                return true;
            }
            // Read the next block chunk
            chunk_sz = block_sz;
            return true;
        } else {
            int r = LZ4_decompress_safe(in, out_buf, block_sz, LZ4_UNCOMPRESSED);
            chunk_sz = sizeof(block_sz);
            block_sz = 0;
            if (r < 0) {
                LOGW("LZ4HC decompression failure (%d)\n", r);
                return false;
            }
            return bwrite(out_buf, r);
        }
    }

private:
    char *out_buf;
    uint32_t block_sz;
};

class LZ4_encoder : public chunk_out_stream {
public:
    explicit LZ4_encoder(out_strm_ptr &&base, bool lg) :
        chunk_out_stream(std::move(base), LZ4_UNCOMPRESSED),
        out_buf(new char[LZ4_COMPRESSED]), lg(lg), in_total(0) {
        bwrite("\x02\x21\x4c\x18", 4);
    }

    ~LZ4_encoder() override {
        finalize();
        if (lg)
            bwrite(&in_total, sizeof(in_total));
        delete[] out_buf;
    }

protected:
    bool write_chunk(const void *buf, size_t len, bool) override {
        auto in = static_cast<const char *>(buf);
        uint32_t block_sz = LZ4_compress_HC(in, out_buf, len, LZ4_COMPRESSED, LZ4HC_CLEVEL_MAX);
        if (block_sz == 0) {
            LOGW("LZ4HC compression failure\n");
            return false;
        }
        if (bwrite(&block_sz, sizeof(block_sz)) && bwrite(out_buf, block_sz)) {
            in_total += len;
            return true;
        }
        return false;
    }

private:
    char *out_buf;
    bool lg;
    uint32_t in_total;
};

out_strm_ptr get_encoder(format_t type, out_strm_ptr &&base) {
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
        case ZOPFLI:
            return make_unique<zopfli_encoder>(std::move(base));
        case GZIP:
        default:
            return make_unique<gz_encoder>(std::move(base));
    }
}

out_strm_ptr get_decoder(format_t type, out_strm_ptr &&base) {
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
        case ZOPFLI:
        case GZIP:
        default:
            return make_unique<gz_decoder>(std::move(base));
    }
}

void decompress(char *infile, const char *outfile) {
    bool in_std = infile == "-"sv;
    bool rm_in = false;

    FILE *in_fp = in_std ? stdin : xfopen(infile, "re");
    out_strm_ptr strm;

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
            strm = get_decoder(type, make_unique<fp_channel>(out_fp));
            if (ext) *ext = '.';
        }
        if (!strm->write(buf, len))
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

    auto strm = get_encoder(fmt, make_unique<fp_channel>(out_fp));

    char buf[4096];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), in_fp))) {
        if (!strm->write(buf, len))
            LOGE("Compression error!\n");
    }

    strm.reset(nullptr);
    fclose(in_fp);

    if (rm_in)
        unlink(infile);
}

bool decompress(rust::Slice<const uint8_t> buf, int fd) {
    format_t type = check_fmt(buf.data(), buf.length());

    if (!COMPRESSED(type)) {
        LOGE("Input file is not a supported compression format!\n");
        return false;
    }

    auto strm = get_decoder(type, make_unique<fd_channel>(fd));
    if (!strm->write(buf.data(), buf.length())) {
        return false;
    }
    return true;
}

bool xz(rust::Slice<const uint8_t> buf, rust::Vec<uint8_t> &out) {
    auto strm = get_encoder(XZ, make_unique<rust_vec_channel>(out));
    if (!strm->write(buf.data(), buf.length())) {
        return false;
    }
    return true;
}

bool unxz(rust::Slice<const uint8_t> buf, rust::Vec<uint8_t> &out) {
    format_t type = check_fmt(buf.data(), buf.length());
    if (type != XZ) {
        LOGE("Input file is not in xz format!\n");
        return false;
    }
    auto strm = get_decoder(XZ, make_unique<rust_vec_channel>(out));
    if (!strm->write(buf.data(), buf.length())) {
        return false;
    }
    return true;
}
