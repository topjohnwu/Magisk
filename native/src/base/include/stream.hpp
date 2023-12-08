#pragma once

#include <sys/uio.h>
#include <cstdio>
#include <memory>

#include "../files.hpp"

struct out_stream {
    virtual bool write(const void *buf, size_t len) = 0;
    virtual ssize_t writev(const iovec *iov, int iovcnt);
    virtual ~out_stream() = default;
};

using out_strm_ptr = std::unique_ptr<out_stream>;

// Delegates all operations to base stream
class filter_out_stream : public out_stream {
public:
    filter_out_stream(out_strm_ptr &&base) : base(std::move(base)) {}
    bool write(const void *buf, size_t len) override;
protected:
    out_strm_ptr base;
};

// Buffered output stream, writing in chunks
class chunk_out_stream : public filter_out_stream {
public:
    chunk_out_stream(out_strm_ptr &&base, size_t buf_sz, size_t chunk_sz)
    : filter_out_stream(std::move(base)), chunk_sz(chunk_sz), data(buf_sz) {}

    chunk_out_stream(out_strm_ptr &&base, size_t buf_sz = 4096)
    : chunk_out_stream(std::move(base), buf_sz, buf_sz) {}

    bool write(const void *buf, size_t len) final;

protected:
    // Classes inheriting this class has to call finalize() in its destructor
    void finalize();
    virtual bool write_chunk(const void *buf, size_t len, bool final);

    size_t chunk_sz;

private:
    size_t buf_off = 0;
    heap_data data;
};

struct in_stream {
    virtual ssize_t read(void *buf, size_t len) = 0;
    virtual ssize_t readFully(void *buf, size_t len);
    virtual ssize_t readv(const iovec *iov, int iovcnt);
    virtual ~in_stream() = default;
};

// A channel is something that is writable, readable, and seekable
struct channel : public out_stream, public in_stream {
    virtual off_t seek(off_t off, int whence) = 0;
    virtual ~channel() = default;
};

using channel_ptr = std::unique_ptr<channel>;

// Byte channel that dynamically allocates memory
class byte_channel : public channel {
public:
    byte_channel(heap_data &data) : _data(data) {}

    ssize_t read(void *buf, size_t len) override;
    bool write(const void *buf, size_t len) override;
    off_t seek(off_t off, int whence) override;

private:
    heap_data &_data;
    size_t _pos = 0;
    size_t _cap = 0;

    void resize(size_t new_sz, bool zero = false);
};

class rust_vec_channel : public channel {
public:
    rust_vec_channel(rust::Vec<uint8_t> &data) : _data(data) {}

    ssize_t read(void *buf, size_t len) override;
    bool write(const void *buf, size_t len) override;
    off_t seek(off_t off, int whence) override;

private:
    rust::Vec<uint8_t> &_data;
    size_t _pos = 0;

    void ensure_size(size_t sz, bool zero = false);
};

class file_channel : public channel {
public:
    bool write(const void *buf, size_t len) final;
protected:
    virtual ssize_t do_write(const void *buf, size_t len) = 0;
};

// File channel but does not close the file descriptor at any time
class fd_channel : public file_channel {
public:
    fd_channel(int fd) : fd(fd) {}
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
 * Bridge between channel class and C stdio
 * ****************************************/

// sFILE -> channel_ptr
class fp_channel final : public file_channel {
public:
    fp_channel(FILE *fp = nullptr) : fp(fp, fclose) {}
    fp_channel(sFILE &&fp) : fp(std::move(fp)) {}
    ssize_t read(void *buf, size_t len) override;
    off_t seek(off_t off, int whence) override;
protected:
    ssize_t do_write(const void *buf, size_t len) override;
private:
    sFILE fp;
};

// channel_ptr -> sFILE
sFILE make_channel_fp(channel_ptr &&strm);

template <class T, class... Args>
sFILE make_channel_fp(Args &&... args) {
    return make_channel_fp(channel_ptr(new T(std::forward<Args>(args)...)));
}
