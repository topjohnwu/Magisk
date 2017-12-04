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

static void cpio_vec_insert(struct vector *v, cpio_entry *n) {
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

static int cpio_cmp(const void *a, const void *b) {
	return strcmp((*(cpio_entry **) a)->filename, (*(cpio_entry **) b)->filename);
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
			unlink(filename);
			if (S_ISREG(f->mode)) {
				int fd = creat(filename, f->mode & 0777);
				xwrite(fd, f->data, f->filesize);
				fchown(fd, f->uid, f->gid);
				close(fd);
			} else if (S_ISLNK(f->mode)) {
				symlink(f->data, filename);
			}
			return 0;
		}
	}
	fprintf(stderr, "Cannot find the file entry [%s]\n", entry);
	return 1;
}

int cpio_test(struct vector *v) {
	#define STOCK_BOOT      0x0
	#define MAGISK_PATCH    0x1
	#define OTHER_PATCH     0x2
	int ret = STOCK_BOOT;
	cpio_entry *f;
	const char *OTHER_LIST[] = { "sbin/launch_daemonsu.sh", "sbin/su", "init.xposed.rc", "boot/sbin/launch_daemonsu.sh", NULL };
	const char *MAGISK_LIST[] = { ".backup/.magisk", "init.magisk.rc", "overlay/init.magisk.rc", NULL };
	vec_for_each(v, f) {
		for (int i = 0; OTHER_LIST[i]; ++i) {
			if (strcmp(f->filename, OTHER_LIST[i]) == 0) {
				ret |= OTHER_PATCH;
				// Already find other files, abort
				exit(OTHER_PATCH);
			}
		}
		for (int i = 0; MAGISK_LIST[i]; ++i) {
			if (strcmp(f->filename, MAGISK_LIST[i]) == 0)
				ret = MAGISK_PATCH;
		}
	}
	cpio_vec_destroy(v);
	return ret;
}

void cpio_backup(struct vector *v, const char *orig, const char *sha1) {
	struct vector o_body, *o = &o_body, bak;
	cpio_entry *m, *n, *rem, *cksm;
	char buf[PATH_MAX];
	int res, doBak;

	vec_init(o);
	vec_init(&bak);

	m = xcalloc(sizeof(*m), 1);
	m->filename = strdup(".backup");
	m->mode = S_IFDIR;
	vec_push_back(&bak, m);

	m = xcalloc(sizeof(*m), 1);
	m->filename = strdup(".backup/.magisk");
	m->mode = S_IFREG;
	vec_push_back(&bak, m);

	rem = xcalloc(sizeof(*rem), 1);
	rem->filename = strdup(".backup/.rmlist");
	rem->mode = S_IFREG;
	vec_push_back(&bak, rem);

	if (sha1) {
		fprintf(stderr, "Save SHA1: [%s] -> [.backup/.sha1]\n", sha1);
		cksm = xcalloc(sizeof(*cksm), 1);
		vec_push_back(&bak, cksm);
		cksm->filename = strdup(".backup/.sha1");
		cksm->mode = S_IFREG;
		cksm->data = strdup(sha1);
		cksm->filesize = strlen(sha1) + 1;
	}

	parse_cpio(o, orig);
	// Remove possible backups in original ramdisk
	cpio_rm(o, 1, ".backup");
	cpio_rm(v, 1, ".backup");

	// Sort both vectors before comparing
	vec_sort(v, cpio_cmp);
	vec_sort(o, cpio_cmp);

	// Start comparing
	size_t i = 0, j = 0;
	while(i != vec_size(o) || j != vec_size(v)) {
		doBak = 0;
		if (i != vec_size(o) && j != vec_size(v)) {
			m = vec_entry(o)[i];
			n = vec_entry(v)[j];
			res = strcmp(m->filename, n->filename);
		} else if (i == vec_size(o)) {
			n = vec_entry(v)[j];
			res = 1;
		} else if (j == vec_size(v)) {
			m = vec_entry(o)[i];
			res = -1;
		}

		if (res < 0) {
			// Something is missing in new ramdisk, backup!
			++i;
			doBak = 1;
			fprintf(stderr, "Backup missing entry: ");
		} else if (res == 0) {
			++i; ++j;
			if (m->filesize == n->filesize && memcmp(m->data, n->data, m->filesize) == 0)
				continue;
			// Not the same!
			doBak = 1;
			fprintf(stderr, "Backup mismatch entry: ");
		} else {
			// Someting new in ramdisk, record in rem
			++j;
			if (n->remove) continue;
			rem->data = xrealloc(rem->data, rem->filesize + strlen(n->filename) + 1);
			memcpy(rem->data + rem->filesize, n->filename, strlen(n->filename) + 1);
			rem->filesize += strlen(n->filename) + 1;
			fprintf(stderr, "Record new entry: [%s] -> [.backup/.rmlist]\n", n->filename);
		}
		if (doBak) {
			sprintf(buf, ".backup/%s", m->filename);
			free(m->filename);
			m->filename = strdup(buf);
			fprintf(stderr, "[%s] -> [%s]\n", buf, m->filename);
			vec_push_back(&bak, m);
			// NULL the original entry, so it won't be freed
			vec_entry(o)[i - 1] = NULL;
		}
	}

	// Add the backup files to the original ramdisk
	vec_for_each(&bak, m) {
		vec_push_back(v, m);
	}

	if (rem->filesize == 0)
		rem->remove = 1;

	// Cleanup
	cpio_vec_destroy(o);
}

int cpio_restore(struct vector *v) {
	cpio_entry *f, *n;
	int ret = 1;
	vec_for_each(v, f) {
		if (strstr(f->filename, ".backup") != NULL) {
			ret = 0;
			f->remove = 1;
			if (f->filename[7] == '\0') continue;
			if (f->filename[8] == '.') {
				if (strcmp(f->filename, ".backup/.rmlist") == 0) {
					for (int pos = 0; pos < f->filesize; pos += strlen(f->data + pos) + 1)
						cpio_rm(v, 0, f->data + pos);
				}
				continue;
			} else {
				n = xcalloc(sizeof(*n), 1);
				memcpy(n, f, sizeof(*f));
				n->filename = strdup(f->filename + 8);
				n->data = f->data;
				f->data = NULL;
				n->remove = 0;
				fprintf(stderr, "Restore [%s] -> [%s]\n", f->filename, n->filename);
				cpio_vec_insert(v, n);
			}
		}
	}
	// Some known stuff we can remove
	cpio_rm(v, 0, "sbin/magic_mask.sh");
	cpio_rm(v, 0, "init.magisk.rc");
	cpio_rm(v, 0, "magisk");
	return ret;
}

char *cpio_stocksha1(struct vector *v) {
	cpio_entry *f;
	char sha1[41];
	vec_for_each(v, f) {
		if (strcmp(f->filename, "init.magisk.rc") == 0
			|| strcmp(f->filename, "overlay/init.magisk.rc") == 0) {
			for (char *pos = f->data; pos < f->data + f->filesize; pos = strchr(pos + 1, '\n') + 1) {
				if (memcmp(pos, "# STOCKSHA1=", 12) == 0) {
					pos += 12;
					memcpy(sha1, pos, 40);
					sha1[40] = '\0';
					return strdup(sha1);
				}
			}
		} else if (strcmp(f->filename, ".backup/.sha1") == 0) {
			return f->data;
		}
	}
	return NULL;
}
