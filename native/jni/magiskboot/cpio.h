#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <string_view>

struct cpio_newc_header {
	char magic[6];
	char ino[8];
	char mode[8];
	char uid[8];
	char gid[8];
	char nlink[8];
	char mtime[8];
	char filesize[8];
	char devmajor[8];
	char devminor[8];
	char rdevmajor[8];
	char rdevminor[8];
	char namesize[8];
	char check[8];
} __attribute__((packed));

struct cpio_entry {
	uint32_t mode = 0;
	uint32_t uid = 0;
	uint32_t gid = 0;
	uint32_t filesize = 0;

	/* Dynamic data */
	std::string filename;
	void *data = nullptr;

	cpio_entry() = default;
	explicit cpio_entry(const char *name) : filename(name) {}
	cpio_entry(int fd, cpio_newc_header &header);
	~cpio_entry();
};

typedef std::map<std::string_view, std::unique_ptr<cpio_entry>> entry_map;

class cpio {
public:
	explicit cpio(const char *filename);
	void dump(const char *file);
	void rm(const char *name, bool r = false);
	void extract();
	bool extract(const char *name, const char *file);
	bool exists(const char *name);

	void insert(cpio_entry *e);
	void add(mode_t mode, const char *name, const char *file);
	void makedir(mode_t mode, const char *name);
	void ln(const char *target, const char *name);
	bool mv(const char *from, const char *to);

protected:
	entry_map entries;

	void rm(entry_map::iterator &it);
	void mv(entry_map::iterator &it, const char *to);
};
