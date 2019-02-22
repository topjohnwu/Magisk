#ifndef _CPIO_H_
#define _CPIO_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <memory>

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

class cpio {
public:
	explicit cpio(const char *filename);
	void dump(const char *file);
	int find(const char *name);
	void insert(cpio_entry *e);
	void insert(std::unique_ptr<cpio_entry> &e);
	void rm(const char *name, bool r = false);
	void makedir(mode_t mode, const char *name);
	void ln(const char *target, const char *name);
	void add(mode_t mode, const char *name, const char *file);
	bool mv(const char *from, const char *to);
	void extract();
	bool extract(const char *name, const char *file);
	void sort();

protected:
	std::vector<std::unique_ptr<cpio_entry> > entries;
};

#endif
