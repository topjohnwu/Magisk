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

void cpio_free(cpio_entry *e) {
	if (e) {
		free(e->filename);
		free(e->data);
		free(e);
	}
}

int cpio_find(struct vector *v, const char *entry) {
	cpio_entry *e;
	vec_for_each(v, e) {
		if (!e) continue;
		if (strcmp(e->filename, entry) == 0)
			return _;
	}
	return -1;
}

int cpio_cmp(const void *a, const void *b) {
	return strcmp(((cpio_entry *) a)->filename, ((cpio_entry *) b)->filename);
}

void cpio_vec_insert(struct vector *v, cpio_entry *n) {
	int i = cpio_find(v, n->filename);
	if (i >= 0) {
		// Replace, then all is done
		cpio_free(vec_entry(v)[i]);
		vec_entry(v)[i] = n;
		return;
	}
	vec_push_back(v, n);
}

// Parse cpio file to a vector of cpio_entry
#define parse_align() lseek(fd, align(lseek(fd, 0, SEEK_CUR), 4), SEEK_SET)
void parse_cpio(struct vector *v, const char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) return;
	fprintf(stderr, "Loading cpio: [%s]\n", filename);
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
		parse_align();
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
			parse_align();
		}
		vec_push_back(v, f);
	}
	close(fd);
}

#define dump_align() write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR), 4))
void dump_cpio(struct vector *v, const char *filename) {
	fprintf(stderr, "Dump cpio: [%s]\n", filename);
	unsigned inode = 300000;
	char header[111];
	// Sort by name
	vec_sort(v, cpio_cmp);
	cpio_entry *e;
	int fd = creat(filename, 0644);
	vec_for_each(v, e) {
		sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x",
			inode++,    // e->ino
			e->mode,
			e->uid,
			e->gid,
			1,          // e->nlink
			0,          // e->mtime
			e->filesize,
			0,          // e->devmajor
			0,          // e->devminor
			0,          // e->rdevmajor
			0,          // e->rdevminor
			(uint32_t) strlen(e->filename) + 1,
			0           // e->check
		);
		xwrite(fd, header, 110);
		xwrite(fd, e->filename, strlen(e->filename) + 1);
		dump_align();
		if (e->filesize) {
			xwrite(fd, e->data, e->filesize);
			dump_align();
		}
	}
	// Write trailer
	sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x", inode++, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 11, 0);
	xwrite(fd, header, 110);
	xwrite(fd, "TRAILER!!!\0", 11);
	dump_align();
	close(fd);
}

void cpio_vec_destroy(struct vector *v) {
	// Free each cpio_entry
	cpio_entry *e;
	vec_for_each(v, e)
		cpio_free(e);
	vec_destroy(v);
}

void cpio_rm(struct vector *v, int recursive, const char *entry) {
	cpio_entry *e;
	size_t len = strlen(entry);
	vec_for_each(v, e) {
		if (!e) continue;
		if (strncmp(e->filename, entry, len) == 0) {
			if ((recursive && e->filename[len] == '/') || e->filename[len] == '\0') {
				fprintf(stderr, "Remove [%s]\n", e->filename);
				cpio_free(e);
				vec_cur(v) = NULL;
				if (!recursive) return;
			}
		}
	}
}

void cpio_mkdir(struct vector *v, mode_t mode, const char *entry) {
	cpio_entry *e = xcalloc(sizeof(*e), 1);
	e->mode = S_IFDIR | mode;
	e->filename = strdup(entry);
	cpio_vec_insert(v, e);
	fprintf(stderr, "Create directory [%s] (%04o)\n",entry, mode);
}

void cpio_ln(struct vector *v, const char *target, const char *entry) {
	cpio_entry *e = xcalloc(sizeof(*e), 1);
	e->mode = S_IFLNK;
	e->filename = strdup(entry);
	e->filesize = strlen(target);
	e->data = strdup(target);
	cpio_vec_insert(v, e);
	fprintf(stderr, "Create symlink [%s] -> [%s]\n", entry, target);
}

void cpio_add(struct vector *v, mode_t mode, const char *entry, const char *filename) {
	int fd = xopen(filename, O_RDONLY);
	cpio_entry *e = xcalloc(sizeof(*e), 1);
	e->mode = S_IFREG | mode;
	e->filename = strdup(entry);
	e->filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	e->data = xmalloc(e->filesize);
	xxread(fd, e->data, e->filesize);
	close(fd);
	cpio_vec_insert(v, e);
	fprintf(stderr, "Add entry [%s] (%04o)\n", entry, mode);
}

int cpio_mv(struct vector *v, const char *from, const char *to) {
	struct cpio_entry *e;
	int f = cpio_find(v, from), t = cpio_find(v, to);
	if (f > 0) {
		if (t > 0) {
			cpio_free(vec_entry(v)[t]);
			vec_entry(v)[t] = NULL;
		}
		e = vec_entry(v)[f];
		free(e->filename);
		e->filename = strdup(to);
		return 0;
	}
	fprintf(stderr, "Cannot find entry %s\n", from);
	return 1;
}

int cpio_extract(struct vector *v, const char *entry, const char *filename) {
	int i = cpio_find(v, entry);
	if (i > 0) {
		cpio_entry *e = vec_entry(v)[i];
		fprintf(stderr, "Extracting [%s] to [%s]\n", entry, filename);
		if (S_ISREG(e->mode)) {
			int fd = creat(filename, e->mode & 0777);
			xwrite(fd, e->data, e->filesize);
			fchown(fd, e->uid, e->gid);
			close(fd);
		} else if (S_ISLNK(e->mode)) {
			char *target = xcalloc(e->filesize + 1, 1);
			memcpy(target, e->data, e->filesize);
			unlink(filename);
			symlink(target, filename);
		}
		return 0;
	}
	fprintf(stderr, "Cannot find the file entry [%s]\n", entry);
	return 1;
}

void cpio_extract_all(struct vector *v) {
	cpio_entry *e;
	vec_for_each(v, e) {
		if (!e) continue;
		fprintf(stderr, "Extracting [%s]\n", e->filename);
		unlink(e->filename);
		rmdir(e->filename);
		if (S_ISDIR(e->mode)) {
			mkdir(e->filename, e->mode & 0777);
		} else if (S_ISREG(e->mode)) {
			int fd = creat(e->filename, e->mode & 0777);
			xwrite(fd, e->data, e->filesize);
			fchown(fd, e->uid, e->gid);
			close(fd);
		} else if (S_ISLNK(e->mode)) {
			char *target = xcalloc(e->filesize + 1, 1);
			memcpy(target, e->data, e->filesize);
			symlink(target, e->filename);
		}
	}
}
