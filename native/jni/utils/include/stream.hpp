#pragma once

#include <stdio.h>
#include <memory>

#include "../files.hpp"

class stream {
public:
    virtual int read(void *buf, size_t len);
    virtual int write(const void *buf, size_t len);
    virtual off_t seek(off_t off, int whence);
    virtual ~stream() = default;
};

using stream_ptr = std::unique_ptr<stream>;

// Delegates all operations to base stream
class filter_stream : public stream {
public:
    filter_stream(stream_ptr &&base) : base(std::move(base)) {}

    int read(void *buf, size_t len) override;
    int write(const void *buf, size_t len) override;

protected:
    stream_ptr base;
};

// Byte stream that dynamically allocates memory
class byte_stream : public stream {
public:
    byte_stream(uint8_t *&buf, size_t &len);
    template <class byte>
    byte_stream(byte *&buf, size_t &len) : byte_stream(reinterpret_cast<uint8_t *&>(buf), len) {}
    int read(void *buf, size_t len) override;
    int write(const void *buf, size_t len) override;
    off_t seek(off_t off, int whence) override;

private:
    uint8_t *&_buf;
    size_t &_len;
    size_t _pos = 0;
    size_t _cap = 0;

    void resize(size_t new_pos, bool zero = false);
};

// File stream but does not close the file descriptor at any time
class fd_stream : public stream {
public:
    fd_stream(int fd) : fd(fd) {}
    int read(void *buf, size_t len) override;
    int write(const void *buf, size_t len) override;
    off_t seek(off_t off, int whence) override;

private:
    int fd;
};

/* ****************************************
 * Bridge between stream class and C stdio
 * ****************************************/

// sFILE -> stream_ptr
class fp_stream final : public stream {
public:
    fp_stream(FILE *fp = nullptr) : fp(fp, fclose) {}
    fp_stream(sFILE &&fp) : fp(std::move(fp)) {}
    int read(void *buf, size_t len) override;
    int write(const void *buf, size_t len) override;
    off_t seek(off_t off, int whence) override;

private:
    sFILE fp;
};

// stream_ptr -> sFILE
sFILE make_stream_fp(stream_ptr &&strm);

template <class T, class... Args>
sFILE make_stream_fp(Args &&... args) {
    return make_stream_fp(stream_ptr(new T(std::forward<Args>(args)...)));
}
