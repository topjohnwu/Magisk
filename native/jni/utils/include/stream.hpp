#pragma once

#include <sys/uio.h>
#include <cstdio>
#include <memory>

#include "../files.hpp"

class stream {
public:
    virtual ssize_t read(void *buf, size_t len);
    virtual ssize_t readFully(void *buf, size_t len);
    virtual ssize_t readv(const iovec *iov, int iovcnt);
    virtual bool write(const void *buf, size_t len);
    virtual ssize_t writev(const iovec *iov, int iovcnt);
    virtual off_t seek(off_t off, int whence);
    virtual ~stream() = default;
};

using stream_ptr = std::unique_ptr<stream>;

// Delegates all operations to base stream
class filter_stream : public stream {
public:
    filter_stream(stream_ptr &&base) : base(std::move(base)) {}

    ssize_t read(void *buf, size_t len) override;
    bool write(const void *buf, size_t len) override;
    virtual bool write(const void *buf, size_t len, bool final);

    // Seeking while filtering does not make sense
    off_t seek(off_t off, int whence) final { return stream::seek(off, whence); }

protected:
    stream_ptr base;
};

using filter_strm_ptr = std::unique_ptr<filter_stream>;

// Buffered output stream, writing in chunks
class chunk_out_stream : public filter_stream {
public:
    chunk_out_stream(stream_ptr &&base, size_t buf_sz, size_t chunk_sz)
    : filter_stream(std::move(base)), chunk_sz(chunk_sz), buf_sz(buf_sz) {}

    chunk_out_stream(stream_ptr &&base, size_t buf_sz = 4096)
    : chunk_out_stream(std::move(base), buf_sz, buf_sz) {}

    ~chunk_out_stream() override { delete[] _buf; }

    // Reading does not make sense
    ssize_t read(void *buf, size_t len) final { return stream::read(buf, len); }
    bool write(const void *buf, size_t len) final;
    bool write(const void *buf, size_t len, bool final) final;

protected:
    // Classes inheriting this class has to call finalize() in its destructor
    void finalize();
    virtual bool write_chunk(const void *buf, size_t len, bool final) = 0;

    size_t chunk_sz;

private:
    size_t buf_sz;
    size_t buf_off = 0;
    uint8_t *_buf = nullptr;
};

// Byte stream that dynamically allocates memory
class byte_stream : public stream {
public:
    byte_stream(uint8_t *&buf, size_t &len);
    template <class Byte>
    byte_stream(Byte *&buf, size_t &len) : byte_stream(reinterpret_cast<uint8_t *&>(buf), len) {}

    ssize_t read(void *buf, size_t len) override;
    bool write(const void *buf, size_t len) override;
    off_t seek(off_t off, int whence) override;

private:
    uint8_t *&_buf;
    size_t &_len;
    size_t _pos = 0;
    size_t _cap = 0;

    void resize(size_t new_pos, bool zero = false);
};

class file_stream : public stream {
public:
    bool write(const void *buf, size_t len) final;
protected:
    virtual ssize_t do_write(const void *buf, size_t len) = 0;
};

// File stream but does not close the file descriptor at any time
class fd_stream : public file_stream {
public:
    fd_stream(int fd) : fd(fd) {}
    ssize_t read(void *buf, size_t len) override;
    ssize_t readv(const iovec *iov, int iovcnt) override;
    ssize_t writev(const iovec *iov, int iovcnt) override;
    off_t seek(off_t off, int whence) override;
protected:
    ssize_t do_write(const void *buf, size_t len) override;
private:
    int fd;
};

/* ****************************************
 * Bridge between stream class and C stdio
 * ****************************************/

// sFILE -> stream_ptr
class fp_stream final : public file_stream {
public:
    fp_stream(FILE *fp = nullptr) : fp(fp, fclose) {}
    fp_stream(sFILE &&fp) : fp(std::move(fp)) {}
    ssize_t read(void *buf, size_t len) override;
    off_t seek(off_t off, int whence) override;
protected:
    ssize_t do_write(const void *buf, size_t len) override;
private:
    sFILE fp;
};

// stream_ptr -> sFILE
sFILE make_stream_fp(stream_ptr &&strm);

template <class T, class... Args>
sFILE make_stream_fp(Args &&... args) {
    return make_stream_fp(stream_ptr(new T(std::forward<Args>(args)...)));
}
