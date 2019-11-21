#pragma once

#include <unistd.h>
#include <stdio.h>
#include <memory>

#include <utils.h>

class stream;

FILE *open_stream(stream *strm);

template <class T, class... Args>
FILE *open_stream(Args &&... args) {
	return open_stream(new T(args...));
}

/* Base classes */

class stream {
public:
	virtual int read(void *buf, size_t len);
	virtual int write(const void *buf, size_t len);
	virtual off_t seek(off_t off, int whence);
	virtual int close();
	virtual ~stream() = default;
};

class filter_stream : public stream {
public:
	filter_stream(FILE *fp) : fp(fp) {}
	int close() override { return fclose(fp); }
	virtual ~filter_stream() { close(); }

	void set_base(FILE *f) {
		if (fp) fclose(fp);
		fp = f;
	}

	template <class T, class... Args >
	void set_base(Args&&... args) {
		set_base(open_stream<T>(args...));
	}

protected:
	FILE *fp;
};

class filter_in_stream : public filter_stream {
public:
	filter_in_stream(FILE *fp = nullptr) : filter_stream(fp) {}
	int read(void *buf, size_t len) override { return fread(buf, len, 1, fp); }
};

class filter_out_stream : public filter_stream {
public:
	filter_out_stream(FILE *fp = nullptr) : filter_stream(fp) {}
	int write(const void *buf, size_t len) override { return fwrite(buf, len, 1, fp); }
};

class seekable_stream : public stream {
protected:
	size_t _pos = 0;

	off_t new_pos(off_t off, int whence);
	virtual size_t end_pos() = 0;
};

/* Concrete classes */

class byte_stream : public seekable_stream {
public:
	byte_stream(uint8_t *&buf, size_t &len);
	template <class byte>
	byte_stream(byte *&buf, size_t &len) : byte_stream(reinterpret_cast<uint8_t *&>(buf), len) {}
	int read(void *buf, size_t len) override;
	int write(const void *buf, size_t len) override;
	off_t seek(off_t off, int whence) override;
	virtual ~byte_stream() = default;

private:
	uint8_t *&_buf;
	size_t &_len;
	size_t _cap = 0;

	void resize(size_t new_pos, bool zero = false);
	size_t end_pos() override { return _len; }
};

class fd_stream : stream {
public:
	fd_stream(int fd) : fd(fd) {}
	int read(void *buf, size_t len) override;
	int write(const void *buf, size_t len) override;
	off_t seek(off_t off, int whence) override;
	virtual ~fd_stream() = default;

private:
	int fd;
};

/* TODO: Replace classes below to new implementation */

class OutStream {
public:
	virtual bool write(const void *buf, size_t len) = 0;
	virtual ~OutStream() = default;
};

typedef std::unique_ptr<OutStream> strm_ptr;

class FilterOutStream : public OutStream {
public:
	FilterOutStream() = default;

	FilterOutStream(strm_ptr &&ptr) : out(std::move(ptr)) {}

	void setOut(strm_ptr &&ptr) { out = std::move(ptr); }

	OutStream *getOut() { return out.get(); }

	bool write(const void *buf, size_t len) override {
		return out ? out->write(buf, len) : false;
	}

protected:
	strm_ptr out;
};

class FDOutStream : public OutStream {
public:
	FDOutStream(int fd, bool close = false) : fd(fd), close(close) {}

	bool write(const void *buf, size_t len) override {
		return ::write(fd, buf, len) == len;
	}

	~FDOutStream() override {
		if (close)
			::close(fd);
	}

protected:
	int fd;
	bool close;
};

class BufOutStream : public OutStream {
public:
	BufOutStream() : buf(nullptr), off(0), cap(0) {};

	bool write(const void *b, size_t len) override {
		bool resize = false;
		while (off + len > cap) {
			cap = cap ? cap << 1 : 1 << 19;
			resize = true;
		}
		if (resize)
			buf = (char *) xrealloc(buf, cap);
		memcpy(buf + off, b, len);
		off += len;
		return true;
	}

	template <typename bytes, typename length>
	void release(bytes *&b, length &len) {
		b = buf;
		len = off;
		buf = nullptr;
		off = cap = 0;
	}

	template <typename bytes, typename length>
	void getbuf(bytes *&b, length &len) const {
		b = buf;
		len = off;
	}

	~BufOutStream() override {
		free(buf);
	}

protected:
	char *buf;
	size_t off;
	size_t cap;
};
