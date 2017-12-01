#ifndef _CPIO_H_
#define _CPIO_H_

#include <stdint.h>

#include "vector.h"

typedef struct cpio_entry {
	// uint32_t ino;
	uint32_t mode;
	uint32_t uid;
	uint32_t gid;
	// uint32_t nlink;
	// uint32_t mtime;
	uint32_t filesize;
	// uint32_t devmajor;
	// uint32_t devminor;
	// uint32_t rdevmajor;
	// uint32_t rdevminor;
	uint32_t namesize;
	// uint32_t check;
	char *filename;
	char *data;
	int remove;
} cpio_entry;

typedef struct cpio_newc_header {
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
} cpio_newc_header;

void parse_cpio(const char *filename, struct vector *v);
void dump_cpio(const char *filename, struct vector *v);
void cpio_vec_destroy(struct vector *v);
void cpio_rm(int recursive, const char *entry, struct vector *v);
void cpio_mkdir(mode_t mode, const char *entry, struct vector *v);
void cpio_add(mode_t mode, const char *entry, const char *filename, struct vector *v);
void cpio_mv(struct vector *v, const char *from, const char *to);
void cpio_test(struct vector *v);
void cpio_extract(const char *entry, const char *filename, struct vector *v);
void cpio_backup(const char *orig, const char *sha1, struct vector *v);
int cpio_restore(struct vector *v);
void cpio_stocksha1(struct vector *v);

#endif
