#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <string_view>

#include <OutStream.h>

struct cpio_newc_header;

struct cpio_entry_base {
	uint32_t mode = 0;
	uint32_t uid = 0;
	uint32_t gid = 0;
	uint32_t filesize = 0;

	void *data = nullptr;

	cpio_entry_base() : mode(0), uid(0), gid(0), filesize(0) {};
	explicit cpio_entry_base(const cpio_newc_header *h);
	virtual ~cpio_entry_base() = default;
};

struct cpio_entry : public cpio_entry_base {
	std::string filename;

	cpio_entry() = default;
	explicit cpio_entry(const char *name) : filename(name) {}
	cpio_entry(int fd, cpio_newc_header &header);
	~cpio_entry() override;
};

typedef std::map<std::string_view, std::unique_ptr<cpio_entry_base>> entry_map;

class cpio {
public:
	void dump(const char *file);
	void rm(const char *name, bool r = false);
	void extract();
	bool extract(const char *name, const char *file);
	bool exists(const char *name);

protected:
	entry_map entries;
	void rm(entry_map::iterator &it);
	void output(OutStream &out);
};

class cpio_rw : public cpio {
public:
	explicit cpio_rw(const char *filename);
	void insert(cpio_entry *e);
	void add(mode_t mode, const char *name, const char *file);
	void makedir(mode_t mode, const char *name);
	void ln(const char *target, const char *name);
	bool mv(const char *from, const char *to);

protected:
	void mv(entry_map::iterator &it, const char *to);
};

class cpio_mmap : public cpio {
public:
	explicit cpio_mmap(const char *filename);
	~cpio_mmap();
private:
	char *buf;
	size_t sz;
};
