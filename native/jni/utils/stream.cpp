#include <cstddef>

#include <utils.hpp>
#include <stream.hpp>

using namespace std;

static int strm_read(void *v, char *buf, int len) {
    auto strm = static_cast<stream *>(v);
    return strm->read(buf, len);
}

static int strm_write(void *v, const char *buf, int len) {
    auto strm = static_cast<stream *>(v);
    return strm->write(buf, len);
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

ssize_t stream::write(const void *buf, size_t len) {
    LOGE("This stream does not implement write\n");
    return -1;
}

ssize_t stream::writeFully(void *buf, size_t len) {
    size_t write_sz = 0;
    ssize_t ret;
    do {
        ret = write((byte *) buf + write_sz, len - write_sz);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            return ret;
        }
        write_sz += ret;
    } while (write_sz != len && ret != 0);
    return write_sz;
}

ssize_t stream::writev(const iovec *iov, int iovcnt) {
    size_t write_sz = 0;
    for (int i = 0; i < iovcnt; ++i) {
        auto ret = writeFully(iov[i].iov_base, iov[i].iov_len);
        if (ret < 0)
            return ret;
        write_sz += ret;
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

ssize_t fp_stream::write(const void *buf, size_t len) {
    return fwrite(buf, 1, len, fp.get());
}

off_t fp_stream::seek(off_t off, int whence) {
    return fseek(fp.get(), off, whence);
}

ssize_t filter_stream::read(void *buf, size_t len) {
    return base->read(buf, len);
}

ssize_t filter_stream::write(const void *buf, size_t len) {
    return base->write(buf, len);
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

ssize_t byte_stream::write(const void *buf, size_t len) {
    resize(_pos + len);
    memcpy(_buf + _pos, buf, len);
    _pos += len;
    _len = std::max(_len, _pos);
    return len;
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
        _buf = (uint8_t *) xrealloc(_buf, _cap);
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

ssize_t fd_stream::write(const void *buf, size_t len) {
    return ::write(fd, buf, len);
}

ssize_t fd_stream::writev(const iovec *iov, int iovcnt) {
    return ::writev(fd, iov, iovcnt);
}

off_t fd_stream::seek(off_t off, int whence) {
    return lseek(fd, off, whence);
}
