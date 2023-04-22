#include <unistd.h>
#include <cstddef>

#include <base.hpp>
#include <stream.hpp>

using namespace std;

static int strm_read(void *v, char *buf, int len) {
    auto strm = static_cast<stream *>(v);
    return strm->read(buf, len);
}

static int strm_write(void *v, const char *buf, int len) {
    auto strm = static_cast<stream *>(v);
    if (!strm->write(buf, len))
        return -1;
    return len;
}

static fpos_t strm_seek(void *v, fpos_t off, int whence) {
    auto strm = static_cast<stream *>(v);
    return strm->seek(off, whence);
}

static int strm_close(void *v) {
    auto strm = static_cast<stream *>(v);
    delete strm;
    return 0;
}

sFILE make_stream_fp(stream_ptr &&strm) {
    auto fp = make_file(funopen(strm.release(), strm_read, strm_write, strm_seek, strm_close));
    setbuf(fp.get(), nullptr);
    return fp;
}

ssize_t stream::read(void *buf, size_t len)  {
    LOGE("This stream does not implement read\n");
    return -1;
}

ssize_t stream::readFully(void *buf, size_t len) {
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

ssize_t stream::readv(const iovec *iov, int iovcnt) {
    size_t read_sz = 0;
    for (int i = 0; i < iovcnt; ++i) {
        auto ret = readFully(iov[i].iov_base, iov[i].iov_len);
        if (ret < 0)
            return ret;
        read_sz += ret;
    }
    return read_sz;
}

bool stream::write(const void *buf, size_t len) {
    LOGE("This stream does not implement write\n");
    return false;
}

ssize_t stream::writev(const iovec *iov, int iovcnt) {
    size_t write_sz = 0;
    for (int i = 0; i < iovcnt; ++i) {
        if (!write(iov[i].iov_base, iov[i].iov_len))
            return write_sz;
        write_sz += iov[i].iov_len;
    }
    return write_sz;
}

off_t stream::seek(off_t off, int whence) {
    LOGE("This stream does not implement seek\n");
    return -1;
}

ssize_t fp_stream::read(void *buf, size_t len) {
    auto ret = fread(buf, 1, len, fp.get());
    return ret ? ret : (ferror(fp.get()) ? -1 : 0);
}

ssize_t fp_stream::do_write(const void *buf, size_t len) {
    return fwrite(buf, 1, len, fp.get());
}

off_t fp_stream::seek(off_t off, int whence) {
    return fseek(fp.get(), off, whence);
}

ssize_t filter_stream::read(void *buf, size_t len) {
    return base->read(buf, len);
}

bool filter_stream::write(const void *buf, size_t len) {
    return base->write(buf, len);
}

bool filter_stream::write(const void *buf, size_t len, bool final) {
    return write(buf, len);
}

bool chunk_out_stream::write(const void *buf, size_t len) {
    return write(buf, len, false);
}

bool chunk_out_stream::write(const void *_in, size_t len, bool final) {
    auto in = static_cast<const uint8_t *>(_in);
    while (len) {
        if (buf_off + len >= chunk_sz) {
            // Enough input for a chunk
            const uint8_t *src;
            if (buf_off) {
                src = _buf;
                auto copy = chunk_sz - buf_off;
                memcpy(_buf + buf_off, in, copy);
                in += copy;
                len -= copy;
                buf_off = 0;
            } else {
                src = in;
                in += chunk_sz;
                len -= chunk_sz;
            }
            if (!write_chunk(src, chunk_sz, final && len == 0))
                return false;
        } else if (final) {
            // Final input data, write regardless whether it is chunk sized
            if (buf_off) {
                memcpy(_buf + buf_off, in, len);
                auto avail = buf_off + len;
                buf_off = 0;
                if (!write_chunk(_buf, avail, true))
                    return false;
            } else {
                if (!write_chunk(in, len, true))
                    return false;
            }
            break;
        } else {
            // Buffer internally
            if (!_buf) {
                _buf = new uint8_t[buf_sz];
            }
            memcpy(_buf + buf_off, in, len);
            buf_off += len;
            break;
        }
    }
    return true;
}

void chunk_out_stream::finalize() {
    if (buf_off) {
        if (!write_chunk(_buf, buf_off, true)) {
            LOGE("Error in finalize, file truncated\n");
        }
        delete[] _buf;
        _buf = nullptr;
        buf_off = 0;
    }
}

byte_stream::byte_stream(uint8_t *&buf, size_t &len) : _buf(buf), _len(len) {
    buf = nullptr;
    len = 0;
}

ssize_t byte_stream::read(void *buf, size_t len) {
    len = std::min((size_t) len, _len - _pos);
    memcpy(buf, _buf + _pos, len);
    return len;
}

bool byte_stream::write(const void *buf, size_t len) {
    resize(_pos + len);
    memcpy(_buf + _pos, buf, len);
    _pos += len;
    _len = std::max(_len, _pos);
    return true;
}

off_t byte_stream::seek(off_t off, int whence) {
    off_t np;
    switch (whence) {
        case SEEK_CUR:
            np = _pos + off;
            break;
        case SEEK_END:
            np = _len + off;
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

void byte_stream::resize(size_t new_pos, bool zero) {
    bool resize = false;
    size_t old_cap = _cap;
    while (new_pos > _cap) {
        _cap = _cap ? (_cap << 1) - (_cap >> 1) : 1 << 12;
        resize = true;
    }
    if (resize) {
        _buf = (uint8_t *) realloc(_buf, _cap);
        if (zero)
            memset(_buf + old_cap, 0, _cap - old_cap);
    }
}

ssize_t fd_stream::read(void *buf, size_t len) {
    return ::read(fd, buf, len);
}

ssize_t fd_stream::readv(const iovec *iov, int iovcnt) {
    return ::readv(fd, iov, iovcnt);
}

ssize_t fd_stream::do_write(const void *buf, size_t len) {
    return ::write(fd, buf, len);
}

ssize_t fd_stream::writev(const iovec *iov, int iovcnt) {
    return ::writev(fd, iov, iovcnt);
}

off_t fd_stream::seek(off_t off, int whence) {
    return lseek(fd, off, whence);
}

bool file_stream::write(const void *buf, size_t len) {
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
