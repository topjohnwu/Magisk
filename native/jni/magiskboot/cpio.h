#ifndef _CPIO_H_
#define _CPIO_H_

#include <stdint.h>

#include "array.h"

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
	// uint32_t ino;
	uint32_t mode = 0;
	uint32_t uid = 0;
	uint32_t gid = 0;
	// uint32_t nlink;
	// uint32_t mtime;
	uint32_t filesize = 0;
	// uint32_t devmajor;
	// uint32_t devminor;
	// uint32_t rdevmajor;
	// uint32_t rdevminor;
	// uint32_t namesize;
	// uint32_t check;
	char *filename = nullptr;
	void *data = nullptr;

	cpio_entry() {}
	cpio_entry(int fd, cpio_newc_header &header);
	~cpio_entry();
};

class cpio {
public:
	cpio(const char *filename);
	~cpio();
	void dump(const char *file);
	int find(const char *name);
	void insert(cpio_entry *e);
	void rm(int recur, const char *name);
	void makedir(mode_t mode, const char *name);
	void ln(const char *target, const char *name);
	void add(mode_t mode, const char *name, const char *file);
	void insert(Array<cpio_entry *> &arr);
	bool mv(const char *from, const char *to);
	void extract();
	bool extract(const char *name, const char *file);
	void sort();

protected:
	Array<cpio_entry *> arr;
};

#endif
