#include <stdio.h>
#include <unistd.h>

#include "magiskboot.h"
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
static void parse_cpio(const char *filename, struct vector *v) {
	fprintf(stderr, "Loading cpio: [%s]\n\n", filename);
	int fd = xopen(filename, O_RDONLY);
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
		f->namesize = x8u(header.namesize);
		// f->check = x8u(header.check);
		f->filename = xmalloc(f->namesize);
		xxread(fd, f->filename, f->namesize);
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
			f->data = malloc(f->filesize);
			xxread(fd, f->data, f->filesize);
			file_align(fd, 4, 0);
		}
		vec_push_back(v, f);
	}
	close(fd);
}

static void dump_cpio(const char *filename, struct vector *v) {
	fprintf(stderr, "\nDump cpio: [%s]\n\n", filename);
	int fd = open_new(filename);
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
			f->namesize,
			0			// f->check
		);
		xwrite(fd, header, 110);
		xwrite(fd, f->filename, f->namesize);
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

static void cpio_vec_destroy(struct vector *v) {
	// Free each cpio_entry
	cpio_entry *f;
	vec_for_each(v, f) {
		cpio_free(f);
	}
	vec_destroy(v);
}

static void cpio_rm(int recursive, const char *entry, struct vector *v) {
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

static void cpio_mkdir(mode_t mode, const char *entry, struct vector *v) {
	cpio_entry *f = xcalloc(sizeof(*f), 1);
	f->mode = S_IFDIR | mode;
	f->namesize = strlen(entry) + 1;
	f->filename = strdup(entry);
	cpio_vec_insert(v, f);
	fprintf(stderr, "Create directory [%s] (%04o)\n",entry, mode);
}

static void cpio_add(mode_t mode, const char *entry, const char *filename, struct vector *v) {
	int fd = xopen(filename, O_RDONLY);
	cpio_entry *f = xcalloc(sizeof(*f), 1);
	f->mode = S_IFREG | mode;
	f->namesize = strlen(entry) + 1;
	f->filename = strdup(entry);
	f->filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	f->data = xmalloc(f->filesize);
	xxread(fd, f->data, f->filesize);
	close(fd);
	cpio_vec_insert(v, f);
	fprintf(stderr, "Add entry [%s] (%04o)\n", entry, mode);
}

static void cpio_test(struct vector *v) {
	#define STOCK_BOOT      0x0
	#define MAGISK_PATCH    0x1
	#define OTHER_PATCH     0x2
	int ret = STOCK_BOOT;
	cpio_entry *f;
	const char *OTHER_LIST[] = { "sbin/launch_daemonsu.sh", "sbin/su", "init.xposed.rc", "boot/sbin/launch_daemonsu.sh", NULL };
	const char *MAGISK_LIST[] = { "init.magisk.rc", "overlay/init.magisk.rc", NULL };
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
	exit(ret);
}

static void cpio_patch(struct vector *v, int keepverity, int keepforceencrypt) {
	cpio_entry *f;
	int skip, write;
	vec_for_each(v, f) {
		if (strcmp(f->filename, "init.rc") == 0) {
			void *new_data = patch_init_rc(f->data, &f->filesize);
			free(f->data);
			f->data = new_data;
		} else {
			if (!keepverity) {
				if (strstr(f->filename, "fstab") != NULL && S_ISREG(f->mode)) {
					write = 0;
					for (int read = 0; read < f->filesize; ++write, ++read) {
						if ((skip = check_verity_pattern(f->data + read)) > 0) {
							fprintf(stderr, "Remove pattern [%.*s] in [%s]\n", skip, f->data + read, f->filename);
							read += skip;
						}
						f->data[write] = f->data[read];
					}
					f->filesize = write;
				} else if (strcmp(f->filename, "verity_key") == 0) {
					fprintf(stderr, "Remove [verity_key]\n");
					f->remove = 1;
				}
			}
			if (!keepforceencrypt) {
				if (strstr(f->filename, "fstab") != NULL && S_ISREG(f->mode)) {
					write = 0;
					for (int read = 0; read < f->filesize; ++write, ++read) {
						if ((skip = check_encryption_pattern(f->data + read)) > 0) {
							// assert(skip > 11)!
							fprintf(stderr, "Replace pattern [%.*s] with [encryptable] in [%s]\n", skip, f->data + read, f->filename);
							memcpy(f->data + write, "encryptable", 11);
							write += 11;
							read += skip;
						}
						f->data[write] = f->data[read];
					}
					f->filesize = write;
				}
			}
		}
	}
}

static void cpio_extract(const char *entry, const char *filename, struct vector *v) {
	cpio_entry *f;
	vec_for_each(v, f) {
		if (strcmp(f->filename, entry) == 0 && S_ISREG(f->mode)) {
			fprintf(stderr, "Extracting [%s] to [%s]\n\n", entry, filename);
			int fd = open_new(filename);
			xwrite(fd, f->data, f->filesize);
			fchmod(fd, f->mode);
			fchown(fd, f->uid, f->gid);
			close(fd);
			exit(0);
		}
	}
	LOGE("Cannot find the file entry [%s]\n", entry);
}

static void cpio_backup(const char *orig, struct vector *v) {
	struct vector o_body, *o = &o_body, bak;
	cpio_entry *m, *n, *dir, *rem;
	char buf[PATH_MAX];
	int res, doBak;

	dir = xcalloc(sizeof(*dir), 1);
	rem = xcalloc(sizeof(*rem), 1);
	vec_init(o);
	vec_init(&bak);
	// First push back the directory and the rmlist
	vec_push_back(&bak, dir);
	vec_push_back(&bak, rem);
	parse_cpio(orig, o);
	// Remove possible backups in original ramdisk
	cpio_rm(1, ".backup", o);
	cpio_rm(1, ".backup", v);

	// Sort both vectors before comparing
	vec_sort(v, cpio_cmp);
	vec_sort(o, cpio_cmp);

	// Init the directory and rmlist
	dir->filename = strdup(".backup");
	dir->namesize = strlen(dir->filename) + 1;
	dir->mode = S_IFDIR;
	rem->filename = strdup(".backup/.rmlist");
	rem->namesize = strlen(rem->filename) + 1;
	rem->mode = S_IFREG;

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
			rem->data = xrealloc(rem->data, rem->filesize + n->namesize);
			memcpy(rem->data + rem->filesize, n->filename, n->namesize);
			rem->filesize += n->namesize;
			fprintf(stderr, "Record new entry: [%s] -> [.backup/.rmlist]\n", n->filename);
		}
		if (doBak) {
			m->namesize += 8;
			m->filename = realloc(m->filename, m->namesize);
			strcpy(buf, m->filename);
			sprintf(m->filename, ".backup/%s", buf);
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

	// Don't include if empty
	if (rem->filesize == 0) {
		rem->remove = 1;
		if (bak.size == 2)
			dir->remove = 1;
	}

	// Cleanup
	cpio_vec_destroy(o);
}

static int cpio_restore(struct vector *v) {
	cpio_entry *f, *n;
	int ret = 1;
	vec_for_each(v, f) {
		if (strstr(f->filename, ".backup") != NULL) {
			ret = 0;
			f->remove = 1;
			if (strcmp(f->filename, ".backup") == 0) continue;
			if (strcmp(f->filename, ".backup/.rmlist") == 0) {
				for (int pos = 0; pos < f->filesize; pos += strlen(f->data + pos) + 1)
					cpio_rm(0, f->data + pos, v);
				continue;
			}
			n = xcalloc(sizeof(*n), 1);
			memcpy(n, f, sizeof(*f));
			n->namesize -= 8;
			n->filename = strdup(f->filename + 8);
			n->data = f->data;
			f->data = NULL;
			n->remove = 0;
			fprintf(stderr, "Restoring [%s] -> [%s]\n", f->filename, n->filename);
			cpio_vec_insert(v, n);
		}
	}
	// Some known stuff we can remove
	cpio_rm(0, "sbin/magic_mask.sh", v);
	cpio_rm(0, "init.magisk.rc", v);
	cpio_rm(0, "magisk", v);
	return ret;
}

static void cpio_stocksha1(struct vector *v) {
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
					printf("%s\n", sha1);
					return;
				}
			}
		}
	}
}

static void cpio_mv(struct vector *v, const char *from, const char *to) {
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
			f->namesize = strlen(to) + 1;
			f->filename = strdup(to);
			return;
		}
	}
	fprintf(stderr, "Cannot find entry %s\n", from);
	exit(1);
}

int cpio_commands(const char *command, int argc, char *argv[]) {
	int recursive = 0, ret = 0;
	command_t cmd;
	char *incpio = argv[0];
	++argv;
	--argc;
	if (strcmp(command, "test") == 0) {
		cmd = TEST;
	} else if (strcmp(command, "restore") == 0) {
		cmd = RESTORE;
	} else if (strcmp(command, "stocksha1") == 0) {
		cmd = STOCKSHA1;
	} else if (argc == 1 && strcmp(command, "backup") == 0) {
		cmd = BACKUP;
	} else if (argc > 0 && strcmp(command, "rm") == 0) {
		cmd = RM;
		if (argc == 2 && strcmp(argv[0], "-r") == 0) {
			recursive = 1;
			++argv;
			--argc;
		}
	} else if (argc == 2 && strcmp(command, "mv") == 0) {
		cmd = MV;
	} else if (argc == 2 && strcmp(command, "patch") == 0) {
		cmd = PATCH;
	} else if (argc == 2 && strcmp(command, "extract") == 0) {
		cmd = EXTRACT;
	} else if (argc == 2 && strcmp(command, "mkdir") == 0) {
		cmd = MKDIR;
	} else if (argc == 3 && strcmp(command, "add") == 0) {
		cmd = ADD;
	} else {
		cmd = NONE;
	}
	struct vector v;
	vec_init(&v);
	parse_cpio(incpio, &v);
	switch(cmd) {
	case TEST:
		cpio_test(&v);
		break;
	case RESTORE:
		ret = cpio_restore(&v);
		break;
	case STOCKSHA1:
		cpio_stocksha1(&v);
		return 0;
	case BACKUP:
		cpio_backup(argv[0], &v);
	case RM:
		cpio_rm(recursive, argv[0], &v);
		break;
	case PATCH:
		cpio_patch(&v, strcmp(argv[0], "true") == 0, strcmp(argv[1], "true") == 0);
		break;
	case EXTRACT:
		cpio_extract(argv[0], argv[1], &v);
		break;
	case MKDIR:
		cpio_mkdir(strtoul(argv[0], NULL, 8), argv[1], &v);
		break;
	case ADD:
		cpio_add(strtoul(argv[0], NULL, 8), argv[1], argv[2], &v);
		break;
	case MV:
		cpio_mv(&v, argv[0], argv[1]);
		break;
	case NONE:
		return 1;
	}
	dump_cpio(incpio, &v);
	cpio_vec_destroy(&v);
	exit(ret);
}
