#include <unistd.h>
#include <cstddef>

#include <base.hpp>
#include <stream.hpp>

using namespace std;

static int strm_read(void *v, char *buf, int len) {
    auto strm = static_cast<channel *>(v);
    return strm->read(buf, len);
}

static int strm_write(void *v, const char *buf, int len) {
    auto strm = static_cast<channel *>(v);
    if (!strm->write(buf, len))
        return -1;
    return len;
}

static fpos_t strm_seek(void *v, fpos_t off, int whence) {
    auto strm = static_cast<channel *>(v);
    return strm->seek(off, whence);
}

static int strm_close(void *v) {
    auto strm = static_cast<channel *>(v);
    delete strm;
    return 0;
}

sFILE make_channel_fp(channel_ptr &&strm) {
    auto fp = make_file(funopen(strm.release(), strm_read, strm_write, strm_seek, strm_close));
    setbuf(fp.get(), nullptr);
    return fp;
}

ssize_t in_stream::readFully(void *buf, size_t len) {
    size_t read_sz = 0;
    ssize_t ret;
    do {
        ret = read((byte *) buf + read_sz, len - read_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            return ret;
        }
        read_sz += ret;
    } while (read_sz != len && ret != 0);
    return read_sz;
}

ssize_t in_stream::readv(const iovec *iov, int iovcnt) {
    size_t read_sz = 0;
    for (int i = 0; i < iovcnt; ++i) {
        auto ret = readFully(iov[i].iov_base, iov[i].iov_len);
        if (ret < 0)
            return ret;
        read_sz += ret;
    }
    return read_sz;
}

ssize_t out_stream::writev(const iovec *iov, int iovcnt) {
    size_t write_sz = 0;
    for (int i = 0; i < iovcnt; ++i) {
        if (!write(iov[i].iov_base, iov[i].iov_len))
            return write_sz;
        write_sz += iov[i].iov_len;
    }
    return write_sz;
}

ssize_t fp_channel::read(void *buf, size_t len) {
    auto ret = fread(buf, 1, len, fp.get());
    return ret ? ret : (ferror(fp.get()) ? -1 : 0);
}

ssize_t fp_channel::do_write(const void *buf, size_t len) {
    return fwrite(buf, 1, len, fp.get());
}

off_t fp_channel::seek(off_t off, int whence) {
    return fseek(fp.get(), off, whence);
}

bool filter_out_stream::write(const void *buf, size_t len) {
    return base->write(buf, len);
}

bool chunk_out_stream::write(const void *_in, size_t len) {
    auto in = static_cast<const uint8_t *>(_in);
    while (len) {
        if (buf_off + len >= chunk_sz) {
            // Enough input for a chunk
            const uint8_t *src;
            if (buf_off) {
                src = data.buf();
                auto copy = chunk_sz - buf_off;
                memcpy(data.buf() + buf_off, in, copy);
                in += copy;
                len -= copy;
                buf_off = 0;
            } else {
                src = in;
                in += chunk_sz;
                len -= chunk_sz;
            }
            if (!write_chunk(src, chunk_sz, false))
                return false;
        } else {
            // Buffer internally
            memcpy(data.buf() + buf_off, in, len);
            buf_off += len;
            break;
        }
    }
    return true;
}

bool chunk_out_stream::write_chunk(const void *buf, size_t len, bool) {
    return base->write(buf, len);
}

void chunk_out_stream::finalize() {
    if (buf_off) {
        if (!write_chunk(data.buf(), buf_off, true)) {
            LOGE("Error in finalize, file truncated\n");
        }
        buf_off = 0;
    }
}

ssize_t byte_channel::read(void *buf, size_t len) {
    len = std::min((size_t) len, _data._sz- _pos);
    memcpy(buf, _data.buf() + _pos, len);
    _pos += len;
    return len;
}

bool byte_channel::write(const void *buf, size_t len) {
    resize(_pos + len);
    memcpy(_data.buf() + _pos, buf, len);
    _pos += len;
    _data._sz= std::max(_data.sz(), _pos);
    return true;
}

off_t byte_channel::seek(off_t off, int whence) {
    off_t np;
    switch (whence) {
        case SEEK_CUR:
            np = _pos + off;
            break;
        case SEEK_END:
            np = _data._sz+ off;
            break;
        case SEEK_SET:
            np = off;
            break;
        default:
            return -1;
    }
    resize(np, true);
    _pos = np;
    return np;
}

void byte_channel::resize(size_t new_sz, bool zero) {
    bool resize = false;
    size_t old_cap = _cap;
    while (new_sz > _cap) {
        _cap = _cap ? (_cap << 1) - (_cap >> 1) : 1 << 12;
        resize = true;
    }
    if (resize) {
        _data._buf = static_cast<uint8_t *>(::realloc(_data._buf, _cap));
        if (zero)
            memset(_data.buf() + old_cap, 0, _cap - old_cap);
    }
}

ssize_t rust_vec_channel::read(void *buf, size_t len) {
    len = std::min<size_t>(len, _data.size() - _pos);
    memcpy(buf, _data.data() + _pos, len);
    _pos += len;
    return len;
}

bool rust_vec_channel::write(const void *buf, size_t len) {
    ensure_size(_pos + len);
    memcpy(_data.data() + _pos, buf, len);
    _pos += len;
    return true;
}

off_t rust_vec_channel::seek(off_t off, int whence) {
    off_t np;
    switch (whence) {
        case SEEK_CUR:
            np = _pos + off;
            break;
        case SEEK_END:
            np = _data.size() + off;
            break;
        case SEEK_SET:
            np = off;
            break;
        default:
            return -1;
    }
    ensure_size(np, true);
    _pos = np;
    return np;
}

void rust_vec_channel::ensure_size(size_t sz, bool zero) {
    size_t old_sz = _data.size();
    if (sz > old_sz) {
        resize_vec(_data, sz);
        if (zero)
            memset(_data.data() + old_sz, 0, sz - old_sz);
    }
}

ssize_t fd_channel::read(void *buf, size_t len) {
    return ::read(fd, buf, len);
}

ssize_t fd_channel::readv(const iovec *iov, int iovcnt) {
    return ::readv(fd, iov, iovcnt);
}

ssize_t fd_channel::do_write(const void *buf, size_t len) {
    return ::write(fd, buf, len);
}

ssize_t fd_channel::writev(const iovec *iov, int iovcnt) {
    return ::writev(fd, iov, iovcnt);
}

off_t fd_channel::seek(off_t off, int whence) {
    return lseek(fd, off, whence);
}

bool file_channel::write(const void *buf, size_t len) {
    size_t write_sz = 0;
    ssize_t ret;
    do {
        ret = do_write((byte *) buf + write_sz, len - write_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            return false;
        }
        write_sz += ret;
    } while (write_sz != len && ret != 0);
    return true;
}
