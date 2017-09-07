#ifndef _CPIO_H_
#define _CPIO_H_

#include <stdint.h>

#include "list.h"

typedef struct cpio_file {
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
} cpio_file;

typedef struct line_list {
    char *line;
    int isNew;
    struct list_head pos;
} line_list;

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

typedef enum {
    NONE,
    RM,
    MKDIR,
    ADD,
    EXTRACT,
    TEST,
    PATCH,
    BACKUP,
    RESTORE,
    STOCKSHA1
} command_t;

#endif
