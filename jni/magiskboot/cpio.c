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
			free(f->filename);
			free(f->data);
			free(f);
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
		if (strcmp(f->filename, "TRAILER!!!") == 0 || strcmp(f->filename, ".") == 0) {
			free(f->filename);
			free(f);
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
	printf("Dump cpio: [%s]\n\n", filename);
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
}

static void cpio_vec_destroy(vector *v) {
	// Free each cpio_file
	cpio_file *f;
	vec_for_each(v, f) {
		free(f->filename);
		free(f->data);
		free(f);
	}
	vec_destroy(v);
}

static void cpio_rm(int recursive, const char *entry, vector *v) {
	cpio_file *f;
	vec_for_each(v, f) {
		if ((recursive && strncmp(f->filename, entry, strlen(entry)) == 0)
			|| (strcmp(f->filename, entry) == 0) ) {
			f->remove = 1;
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
	cpio_vec_insert(v, f);
}

static void cpio_test(vector *v) {
	int ret = 0;
	cpio_file *f;
	vec_for_each(v, f) {
		if (strcmp(f->filename, "sbin/launch_daemonsu.sh") == 0) {
			if (!ret) ret = 2;
		} else if (strcmp(f->filename, "magisk") == 0) {
			ret = 1;
			break;
		}
	}
	cpio_vec_destroy(v);
	exit(ret);
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
	printf("\n");
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
	printf("\n");
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
			return;
		}
	}
	error(1, "Cannot find the file entry [%s]", entry);
}

int cpio_commands(const char *command, int argc, char *argv[]) {
	int recursive = 0;
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
	} else if (argc == 2 && strcmp(command, "extract") == 0) {
		cmd = EXTRACT;
	} else if (argc > 0 && strcmp(command, "rm") == 0) {
		cmd = RM;
		if (argc == 2 && strcmp(argv[0], "-r") == 0) {
			recursive = 1;
			++argv;
			--argc;
		}
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
		case EXTRACT:
			cpio_extract(argv[0], argv[1], &v);
			break;
		case RM:
			cpio_rm(recursive, argv[0], &v);
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
	return 0;
}
