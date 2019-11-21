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

class stream {
public:
	virtual int read(void *buf, size_t len);
	virtual int write(const void *buf, size_t len);
	virtual off_t seek(off_t off, int whence);
	virtual int close();
	virtual ~stream() = default;
};

// Delegates all operations to the base FILE pointer
class filter_stream : public stream {
public:
	filter_stream(FILE *fp) : fp(fp) {}
	~filter_stream() override { if (fp) close(); }

	int read(void *buf, size_t len) override;
	int write(const void *buf, size_t len) override;
	int close() override;

	void set_base(FILE *f);
	template <class T, class... Args >
	void set_base(Args&&... args) {
		set_base(open_stream<T>(args...));
	}

protected:
	FILE *fp;
};

// Handy interface for classes that need custom seek logic
class seekable_stream : public stream {
protected:
	size_t _pos = 0;

	off_t new_pos(off_t off, int whence);
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
	size_t end_pos() override { return _len; }
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
