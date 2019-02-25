#pragma once

#include <unistd.h>
#include <memory>

#include "utils.h"

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

	void set_out(strm_ptr &&ptr) { out = std::move(ptr); }

	OutStream *get_out() { return out.get(); }

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
