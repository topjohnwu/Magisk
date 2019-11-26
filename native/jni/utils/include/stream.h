#pragma once

#include <stdio.h>
#include <memory>

#include "../files.h"

class stream;

using stream_ptr = std::unique_ptr<stream>;

sFILE make_stream(stream_ptr &&strm);

template <class T, class... Args>
sFILE make_stream(Args &&... args) {
	return make_stream(stream_ptr(new T(std::forward<Args>(args)...)));
}

class stream {
public:
	virtual int read(void *buf, size_t len);
	virtual int write(const void *buf, size_t len);
	virtual off_t seek(off_t off, int whence);
	virtual ~stream() = default;
};

// Delegates all operations to the base FILE pointer
class filter_stream : public stream {
public:
	filter_stream(sFILE &&fp = make_sFILE()) : fp(std::move(fp)) {}

	int read(void *buf, size_t len) override;
	int write(const void *buf, size_t len) override;

	void set_base(sFILE &&f);
	template <class T, class... Args >
	void set_base(Args&&... args) {
		set_base(make_stream<T>(std::forward<Args>(args)...));
	}

protected:
	sFILE fp;
};

// Handy interface for classes that need custom seek logic
class seekable_stream : public stream {
protected:
	size_t _pos = 0;

	off_t seek_pos(off_t off, int whence);
	virtual size_t end_pos() = 0;
};

// Byte stream that dynamically allocates memory
class byte_stream : public seekable_stream {
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
	size_t _cap = 0;

	void resize(size_t new_pos, bool zero = false);
	size_t end_pos() final { return _len; }
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
