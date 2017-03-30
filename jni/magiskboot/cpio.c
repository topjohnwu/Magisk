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

static void cpio_free(cpio_file *f) {
	if (f) {
		free(f->filename);
		free(f->data);
		free(f);
	}
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
			cpio_free(f);
			vec_entry(v)[_i] = n;
			return;
		} else if (strcmp(f->filename, n->filename) > 0) {
			// Insert, then start shifting
			vec_entry(v)[_i] = n;
			t = f;
			shift = 1;
		}
	}
	if (shift)
		vec_push_back(v, t);
	else
		vec_push_back(v, n);
}

static int cpio_compare(const void *a, const void *b) {
	return strcmp((*(cpio_file **) a)->filename, (*(cpio_file **) b)->filename);
}

// Parse cpio file to a vector of cpio_file
static void parse_cpio(const char *filename, vector *v) {
	printf("Loading cpio: [%s]\n\n", filename);
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
			read(fd, f->data, f->filesize);
			file_align(fd, 4, 0);
		}
		vec_push_back(v, f);
	}
	close(fd);
	// Sort by name
	vec_sort(v, cpio_compare);
}

static void dump_cpio(const char *filename, vector *v) {
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
	// Write trailer
	sprintf(header, "070701%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x", inode++, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 11, 0);
	write(fd, header, 110);
	write(fd, "TRAILER!!!\0", 11);
	file_align(fd, 4, 1);
	close(fd);
}

static void cpio_vec_destroy(vector *v) {
	// Free each cpio_file
	cpio_file *f;
	vec_for_each(v, f) {
		cpio_free(f);
	}
	vec_destroy(v);
}

static void cpio_rm(int recursive, const char *entry, vector *v) {
	cpio_file *f;
	vec_for_each(v, f) {
		if ((recursive && strncmp(f->filename, entry, strlen(entry)) == 0)
			|| (strcmp(f->filename, entry) == 0) ) {
			if (!f->remove) {
				printf("Remove [%s]\n", entry);
				f->remove = 1;
			}
			if (!recursive) return;
		}
	}
}

static void cpio_mkdir(mode_t mode, const char *entry, vector *v) {
	cpio_file *f = calloc(sizeof(*f), 1);
	f->mode = S_IFDIR | mode;
	f->namesize = strlen(entry) + 1;
	f->filename = malloc(f->namesize);
	memcpy(f->filename, entry, f->namesize);
	cpio_vec_insert(v, f);
	printf("Create directory [%s] (%04o)\n",entry, mode);
}

static void cpio_add(mode_t mode, const char *entry, const char *filename, vector *v) {
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
	close(fd);
	cpio_vec_insert(v, f);
	printf("Add entry [%s] (%04o)\n", entry, mode);
}

static void cpio_test(vector *v) {
	#define MAGISK_PATCH  0x1
	#define SUPERSU_PATCH 0x2
	int ret = 0;
	cpio_file *f;
	vec_for_each(v, f) {
		if (strcmp(f->filename, "sbin/launch_daemonsu.sh") == 0) {
			ret |= SUPERSU_PATCH;
		} else if (strcmp(f->filename, "init.magisk.rc") == 0) {
			ret |= MAGISK_PATCH;
		}
	}
	cpio_vec_destroy(v);
	exit((ret & SUPERSU_PATCH) ? SUPERSU_PATCH : (ret & MAGISK_PATCH));
}

static int check_verity_pattern(const char *s) {
	int pos = 0;
	if (s[0] == ',') ++pos;
	if (strncmp(s + pos, "verify", 6) != 0) return -1;
	pos += 6;
	if (s[pos] == '=') {
		while (s[pos] != ' ' && s[pos] != '\n' && s[pos] != ',') ++pos;
	}
	return pos;
}

static void cpio_dmverity(vector *v) {
	cpio_file *f;
	size_t read, write;
	int skip;
	vec_for_each(v, f) {
		if (strstr(f->filename, "fstab") != NULL && S_ISREG(f->mode)) {
			for (read = 0, write = 0; read < f->filesize; ++read, ++write) {
				skip = check_verity_pattern(f->data + read);
				if (skip > 0) {
					printf("Remove pattern [%.*s] in [%s]\n", (int) skip, f->data + read, f->filename);
					read += skip;
				}
				f->data[write] = f->data[read];
			}
			f->filesize = write;
		} else if (strcmp(f->filename, "verity_key") == 0) {
			f->remove = 1;
			break;
		}
	}
}

static void cpio_forceencrypt(vector *v) {
	cpio_file *f;
	size_t read, write;
	#define ENCRYPT_LIST_SIZE 2
	const char *ENCRYPT_LIST[ENCRYPT_LIST_SIZE] = { "forceencrypt", "forcefdeorfbe" };
	vec_for_each(v, f) {
		if (strstr(f->filename, "fstab") != NULL && S_ISREG(f->mode)) {
			for (read = 0, write = 0; read < f->filesize; ++read, ++write) {
				for (int i = 0 ; i < ENCRYPT_LIST_SIZE; ++i) {
					if (strncmp(f->data + read, ENCRYPT_LIST[i], strlen(ENCRYPT_LIST[i])) == 0) {
						memcpy(f->data + write, "encryptable", 11);
						printf("Replace [%s] with [%s] in [%s]\n", ENCRYPT_LIST[i], "encryptable", f->filename);
						write += 11;
						read += strlen(ENCRYPT_LIST[i]);
						break;
					}
				}
				f->data[write] = f->data[read];
			}
			f->filesize = write;
		}
	}
}

static void cpio_extract(const char *entry, const char *filename, vector *v) {
	cpio_file *f;
	vec_for_each(v, f) {
		if (strcmp(f->filename, entry) == 0 && S_ISREG(f->mode)) {
			printf("Extracting [%s] to [%s]\n\n", entry, filename);
			int fd = open_new(filename);
			write(fd, f->data, f->filesize);
			fchmod(fd, f->mode);
			fchown(fd, f->uid, f->gid);
			close(fd);
			exit(0);
		}
	}
	error(1, "Cannot find the file entry [%s]", entry);
}

static void cpio_backup(const char *orig, vector *v) {
	vector o_body, *o = &o_body, bak;
	cpio_file *m, *n, *dir, *rem;
	char chk1[21], chk2[21], buf[PATH_MAX];
	int res, doBak;

	dir = calloc(sizeof(*dir), 1);
	rem = calloc(sizeof(*rem), 1);
	vec_init(o);
	vec_init(&bak);
	// First push back the directory and the rmlist
	vec_push_back(&bak, dir);
	vec_push_back(&bak, rem);
	parse_cpio(orig, o);
	// Remove possible backups in original ramdisk
	cpio_rm(1, ".backup", o);
	cpio_rm(1, ".backup", v);

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
			printf("Entry [%s] is missing\n", m->filename);
		} else if (res == 0) {
			++i; ++j;
			if (m->filesize == n->filesize && memcmp(m->data, n->data, m->filesize) == 0)
				continue;
			// Not the same!
			doBak = 1;
			printf("Entry [%s] missmatch\n", m->filename);
		} else {
			// Someting new in ramdisk, record in rem
			++j;
			if (n->remove) continue;
			rem->data = realloc(rem->data, rem->filesize + n->namesize);
			memcpy(rem->data + rem->filesize, n->filename, n->namesize);
			rem->filesize += n->namesize;
			printf("Entry [%s] is new\n", n->filename);
		}
		if (doBak) {
			m->namesize += 8;
			m->filename = realloc(m->filename, m->namesize);
			strcpy(buf, m->filename);
			sprintf(m->filename, ".backup/%s", buf);
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

	// Sort
	vec_sort(v, cpio_compare);

	// Cleanup
	cpio_vec_destroy(o);
}

static int cpio_restore(vector *v) {
	cpio_file *f, *n;
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
			n = calloc(sizeof(*n), 1);
			memcpy(n, f, sizeof(*f));
			n->namesize -= 8;
			n->filename = malloc(n->namesize);
			memcpy(n->filename, f->filename + 8, n->namesize);
			n->data = malloc(n->filesize);
			memcpy(n->data, f->data, n->filesize);
			n->remove = 0;
			printf("Restoring [%s] -> [%s]\n", f->filename, n->filename);
			cpio_vec_insert(v, n);
		}
	}
	// Some known stuff we can remove
	cpio_rm(0, "sbin/magic_mask.sh", v);
	cpio_rm(0, "init.magisk.rc", v);
	cpio_rm(0, "magisk", v);
	return ret;
}

int cpio_commands(const char *command, int argc, char *argv[]) {
	int recursive = 0, ret = 0;
	command_t cmd;
	char *incpio = argv[0];
	++argv;
	--argc;
	if (strcmp(command, "test") == 0) {
		cmd = TEST;
	} else if (strcmp(command, "patch-dmverity") == 0) {
		cmd = DMVERITY;
	} else if (strcmp(command, "patch-forceencrypt") == 0) {
		cmd = FORCEENCRYPT;
	} else if (strcmp(command, "restore") == 0) {
		cmd = RESTORE;
	} else if (argc == 1 && strcmp(command, "backup") == 0) {
		cmd = BACKUP;
	} else if (argc > 0 && strcmp(command, "rm") == 0) {
		cmd = RM;
		if (argc == 2 && strcmp(argv[0], "-r") == 0) {
			recursive = 1;
			++argv;
			--argc;
		}
	} else if (argc == 2 && strcmp(command, "extract") == 0) {
		cmd = EXTRACT;
	} else if (argc == 2 && strcmp(command, "mkdir") == 0) {
		cmd = MKDIR;
	} else if (argc == 3 && strcmp(command, "add") == 0) {
		cmd = ADD;
	} else {
		cmd = NONE;
		return 1;
	}
	vector v;
	vec_init(&v);
	parse_cpio(incpio, &v);
	switch(cmd) {
		case TEST:
			cpio_test(&v);
			break;
		case DMVERITY:
			cpio_dmverity(&v);
			break;
		case FORCEENCRYPT:
			cpio_forceencrypt(&v);
			break;
		case RESTORE:
			ret = cpio_restore(&v);
			break;
		case BACKUP:
			cpio_backup(argv[0], &v);
		case RM:
			cpio_rm(recursive, argv[0], &v);
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
		default:
			// Never happen
			break;
	}
	dump_cpio(incpio, &v);
	cpio_vec_destroy(&v);
	exit(ret);
}
