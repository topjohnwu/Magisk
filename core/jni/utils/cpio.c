#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "cpio.h"
#include "logging.h"
#include "utils.h"

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
  if (inpos != outpos) LOGE("bad cpio header\n");

  return val;
}

static void cpio_free(cpio_entry *f) {
	if (f) {
		free(f->filename);
		free(f->data);
		free(f);
	}
}

int cpio_cmp(const void *a, const void *b) {
	return strcmp((*(cpio_entry **) a)->filename, (*(cpio_entry **) b)->filename);
}

void cpio_vec_insert(struct vector *v, cpio_entry *n) {
	cpio_entry *f;
	vec_for_each(v, f) {
		if (strcmp(f->filename, n->filename) == 0) {
			// Replace, then all is done
			cpio_free(f);
			vec_cur(v) = n;
			return;
		}
	}
	vec_push_back(v, n);
}

// Parse cpio file to a vector of cpio_entry
void parse_cpio(struct vector *v, const char *filename) {
	fprintf(stderr, "Loading cpio: [%s]\n\n", filename);
	int fd = open(filename, O_RDONLY);
	if (fd < 0) return;
	cpio_newc_header header;
	cpio_entry *f;
	while(xxread(fd, &header, 110) != -1) {
		f = xcalloc(sizeof(*f), 1);
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
		uint32_t namesize = x8u(header.namesize);
		// f->check = x8u(header.check);
		f->filename = xmalloc(namesize);
		xxread(fd, f->filename, namesize);
		file_align(fd, 4, 0);
		if (strcmp(f->filename, ".") == 0 || strcmp(f->filename, "..") == 0) {
			cpio_free(f);
			continue;
		}
		if (strcmp(f->filename, "TRAILER!!!") == 0) {
			cpio_free(f);
			break;
		}
		if (f->filesize) {
			f->data = xmalloc(f->filesize);
			xxread(fd, f->data, f->filesize);
			file_align(fd, 4, 0);
		}
		vec_push_back(v, f);
	}
	close(fd);
}

void dump_cpio(struct vector *v, const char *filename) {
	fprintf(stderr, "\nDump cpio: [%s]\n\n", filename);
	int fd = creat(filename, 0644);
	unsigned inode = 300000;
	char header[111];
	// Sort by name
	vec_sort(v, cpio_cmp);
	cpio_entry *f;
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
			(uint32_t) strlen(f->filename) + 1,
			0			// f->check
		);
		xwrite(fd, header, 110);
		xwrite(fd, f->filename, strlen(f->filename) + 1);
		file_align(fd, 4, 1);
		if (f->filesize) {
			xwrite(fd, f->data, f->filesize);
			file_align(fd, 4, 1);
		}
	}
	// Write trailer
	sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x", inode++, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 11, 0);
	xwrite(fd, header, 110);
	xwrite(fd, "TRAILER!!!\0", 11);
	file_align(fd, 4, 1);
	close(fd);
}

void cpio_vec_destroy(struct vector *v) {
	// Free each cpio_entry
	cpio_entry *f;
	vec_for_each(v, f) {
		cpio_free(f);
	}
	vec_destroy(v);
}

void cpio_rm(struct vector *v, int recursive, const char *entry) {
	cpio_entry *f;
	vec_for_each(v, f) {
		if (strncmp(f->filename, entry, strlen(entry)) == 0) {
			char next = f->filename[strlen(entry)];
			if ((recursive && next == '/') || next == '\0') {
				if (!f->remove) {
					fprintf(stderr, "Remove [%s]\n", f->filename);
					f->remove = 1;
				}
				if (!recursive) return;
			}
		}
	}
}

void cpio_mkdir(struct vector *v, mode_t mode, const char *entry) {
	cpio_entry *f = xcalloc(sizeof(*f), 1);
	f->mode = S_IFDIR | mode;
	f->filename = strdup(entry);
	cpio_vec_insert(v, f);
	fprintf(stderr, "Create directory [%s] (%04o)\n",entry, mode);
}

void cpio_ln(struct vector *v, const char *target, const char *entry) {
	cpio_entry *f = xcalloc(sizeof(*f), 1);
	f->mode = S_IFLNK;
	f->filename = strdup(entry);
	f->filesize = strlen(target);
	f->data = strdup(target);
	cpio_vec_insert(v, f);
	fprintf(stderr, "Create symlink [%s] -> [%s]\n", entry, target);
}

void cpio_add(struct vector *v, mode_t mode, const char *entry, const char *filename) {
	int fd = xopen(filename, O_RDONLY);
	cpio_entry *f = xcalloc(sizeof(*f), 1);
	f->mode = S_IFREG | mode;
	f->filename = strdup(entry);
	f->filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	f->data = xmalloc(f->filesize);
	xxread(fd, f->data, f->filesize);
	close(fd);
	cpio_vec_insert(v, f);
	fprintf(stderr, "Add entry [%s] (%04o)\n", entry, mode);
}

int cpio_mv(struct vector *v, const char *from, const char *to) {
	struct cpio_entry *f, *t;
	vec_for_each(v, f) {
		if (strcmp(f->filename, from) == 0) {
			fprintf(stderr, "Move [%s] -> [%s]\n", from, to);
			vec_for_each(v, t) {
				if (strcmp(t->filename, to) == 0) {
					t->remove = 1;
					break;
				}
			}
			free(f->filename);
			f->filename = strdup(to);
			return 0;
		}
	}
	fprintf(stderr, "Cannot find entry %s\n", from);
	return 1;
}

int cpio_extract(struct vector *v, const char *entry, const char *filename) {
	cpio_entry *f;
	vec_for_each(v, f) {
		if (strcmp(f->filename, entry) == 0) {
			fprintf(stderr, "Extracting [%s] to [%s]\n\n", entry, filename);
			if (S_ISREG(f->mode)) {
				int fd = creat(filename, f->mode & 0777);
				xwrite(fd, f->data, f->filesize);
				fchown(fd, f->uid, f->gid);
				close(fd);
			} else if (S_ISLNK(f->mode)) {
				char *target = xcalloc(f->filesize + 1, 1);
				memcpy(target, f->data, f->filesize);
				unlink(filename);
				symlink(target, filename);
			}
			return 0;
		}
	}
	fprintf(stderr, "Cannot find the file entry [%s]\n", entry);
	return 1;
}

void cpio_extract_all(struct vector *v) {
	cpio_entry *f;
	vec_for_each(v, f) {
		fprintf(stderr, "Extracting [%s]\n", f->filename);
		unlink(f->filename);
		rmdir(f->filename);
		if (S_ISDIR(f->mode)) {
			mkdir(f->filename, f->mode & 0777);
		} else if (S_ISREG(f->mode)) {
			int fd = creat(f->filename, f->mode & 0777);
			xwrite(fd, f->data, f->filesize);
			fchown(fd, f->uid, f->gid);
			close(fd);
		} else if (S_ISLNK(f->mode)) {
			char *target = xcalloc(f->filesize + 1, 1);
			memcpy(target, f->data, f->filesize);
			symlink(target, f->filename);
		}
	}
}
