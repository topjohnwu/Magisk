#include "magiskboot.h"
#include "cpio.h"

static uint32_t x8u(char *hex) {
  uint32_t val, inpos = 8, outpos;
  char pattern[6];

  while (*hex == '0') {
    hex++;
    if (!--inpos) return 0;
  }
  // Because scanf gratuitously treats %*X differently than printf does.
  sprintf(pattern, "%%%dx%%n", inpos);
  sscanf(hex, pattern, &val, &outpos);
  if (inpos != outpos) error(1, "bad cpio header");

  return val;
}

static void cpio_vec_insert(vector *v, cpio_file *n) {
	cpio_file *f, *t;
	int shift = 0;
	// Insert in alphabet order
	vec_for_each(v, f) {
		if (shift) {
			vec_entry(v)[_i] = t;
			t = f;
			continue;
		}
		t = f;
		if (strcmp(f->filename, n->filename) == 0) {
			// Replace, then all is done
			vec_entry(v)[_i] = n;
			return;
		} else if (strcmp(f->filename, n->filename) > 0) {
			// Insert, then start shifting
			vec_entry(v)[_i] = n;
			t = f;
			shift = 1;
		}
	}
	vec_push_back(v, t);
}

// Parse cpio file to a vector of cpio_file
void parse_cpio(const char *filename, vector *v) {
	printf("\nLoading cpio: [%s]\n\n", filename);
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		error(1, "Cannot open %s", filename);
	cpio_newc_header header;
	cpio_file *f;
	while(read(fd, &header, 110) == 110) {
		f = calloc(sizeof(*f), 1);
		// f->ino = x8u(header.ino);
		f->mode = x8u(header.mode);
		f->uid = x8u(header.uid);
		f->gid = x8u(header.gid);
		// f->nlink = x8u(header.nlink);
		// f->mtime = x8u(header.mtime);
		f->filesize = x8u(header.filesize);
		// f->devmajor = x8u(header.devmajor);
		// f->devminor = x8u(header.devminor);
		// f->rdevmajor = x8u(header.rdevmajor);
		// f->rdevminor = x8u(header.rdevminor);
		f->namesize = x8u(header.namesize);
		// f->check = x8u(header.check);
		f->filename = malloc(f->namesize);
		read(fd, f->filename, f->namesize);
		file_align(fd, 4, 0);
		if (f->filesize) {
			f->data = malloc(f->filesize);
			read(fd, f->data, f->filesize);
			file_align(fd, 4, 0);
		}
		vec_push_back(v, f);
	}
	close(fd);
}

void dump_cpio(const char *filename, vector *v) {
	printf("\nDump cpio: [%s]\n\n", filename);
	int fd = open_new(filename);
	unsigned inode = 300000;
	char header[111];
	cpio_file *f;
	vec_for_each(v, f) {
		if (f->remove) continue;
		sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
			inode++,	// f->ino
			f->mode,
			f->uid,
			f->gid,
			1,			// f->nlink
			0,			// f->mtime
			f->filesize,
			0,			// f->devmajor
			0,			// f->devminor
			0,			// f->rdevmajor
			0,			// f->rdevminor
			f->namesize,
			0			// f->check
		);
		write(fd, header, 110);
		write(fd, f->filename, f->namesize);
		file_align(fd, 4, 1);
		if (f->filesize) {
			write(fd, f->data, f->filesize);
			file_align(fd, 4, 1);
		}
	}
}

void cpio_vec_destroy(vector *v) {
	// Free each cpio_file
	cpio_file *f;
	vec_for_each(v, f) {
		free(f->filename);
		free(f->data);
		free(f);
	}
	vec_destroy(v);
}

void cpio_rm(int recursive, const char *entry, vector *v) {
	cpio_file *f;
	vec_for_each(v, f) {
		if ((recursive && strncmp(f->filename, entry, strlen(entry)) == 0)
			|| (strcmp(f->filename, entry) == 0) ) {
			f->remove = 1;
		}
	}
}

void cpio_mkdir(mode_t mode, const char *entry, vector *v) {
	cpio_file *f = calloc(sizeof(*f), 1);
	f->mode = S_IFDIR | mode;
	f->namesize = strlen(entry) + 1;
	f->filename = malloc(f->namesize);
	memcpy(f->filename, entry, f->namesize);
	cpio_vec_insert(v, f);
}

void cpio_add(mode_t mode, const char *entry, const char *filename, vector *v) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		error(1, "Cannot open %s", filename);
	cpio_file *f = calloc(sizeof(*f), 1);
	f->mode = S_IFREG | mode;
	f->namesize = strlen(entry) + 1;
	f->filename = malloc(f->namesize);
	memcpy(f->filename, entry, f->namesize);
	f->filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	f->data = malloc(f->filesize);
	read(fd, f->data, f->filesize);
	cpio_vec_insert(v, f);
}
