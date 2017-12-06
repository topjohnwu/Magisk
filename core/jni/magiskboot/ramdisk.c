#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utils.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "magiskboot.h"
#include "cpio.h"

static void cpio_patch(struct vector *v, int keepverity, int keepforceencrypt) {
	cpio_entry *f;
	vec_for_each(v, f) {
		if (!keepverity) {
			if (strstr(f->filename, "fstab") != NULL && S_ISREG(f->mode)) {
				patch_verity(&f->data, &f->filesize, 1);
			} else if (strcmp(f->filename, "verity_key") == 0) {
				fprintf(stderr, "Remove [verity_key]\n");
				f->remove = 1;
			}
		}
		if (!keepforceencrypt) {
			if (strstr(f->filename, "fstab") != NULL && S_ISREG(f->mode)) {
				patch_encryption(&f->data, &f->filesize);
			}
		}
	}
}

#define STOCK_BOOT      0x0
#define MAGISK_PATCH    0x1
#define OTHER_PATCH     0x2

static int cpio_test(struct vector *v) {
	int ret = STOCK_BOOT;
	cpio_entry *f;
	const char *OTHER_LIST[] = { "sbin/launch_daemonsu.sh", "sbin/su", "init.xposed.rc", "boot/sbin/launch_daemonsu.sh", NULL };
	const char *MAGISK_LIST[] = { ".backup/.magisk", "init.magisk.rc", "overlay/init.magisk.rc", NULL };
	vec_for_each(v, f) {
		for (int i = 0; OTHER_LIST[i]; ++i) {
			if (strcmp(f->filename, OTHER_LIST[i]) == 0) {
				// Already find other files, abort
				return OTHER_PATCH;
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

static char *cpio_stocksha1(struct vector *v) {
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

static struct vector *cpio_backup(struct vector *v, const char *orig, const char *keepverity,
								 const char *keepforceencrypt, const char *sha1) {
	struct vector o_body, *o = &o_body, *ret;
	cpio_entry *m, *n, *rem, *cksm;
	char buf[PATH_MAX];
	int res, backup;

	ret = xcalloc(sizeof(*ret), 1);

	vec_init(o);
	vec_init(ret);

	m = xcalloc(sizeof(*m), 1);
	m->filename = strdup(".backup");
	m->mode = S_IFDIR;
	vec_push_back(ret, m);

	m = xcalloc(sizeof(*m), 1);
	m->filename = strdup(".backup/.magisk");
	m->mode = S_IFREG;
	m->data = xmalloc(50);
	snprintf(m->data, 50, "KEEPVERITY=%s\nKEEPFORCEENCRYPT=%s\n", keepverity, keepforceencrypt);
	m->filesize = strlen(m->data) + 1;
	vec_push_back(ret, m);

	rem = xcalloc(sizeof(*rem), 1);
	rem->filename = strdup(".backup/.rmlist");
	rem->mode = S_IFREG;
	vec_push_back(ret, rem);

	if (sha1) {
		fprintf(stderr, "Save SHA1: [%s] -> [.backup/.sha1]\n", sha1);
		cksm = xcalloc(sizeof(*cksm), 1);
		vec_push_back(ret, cksm);
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
		backup = 0;
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
			backup = 1;
			fprintf(stderr, "Backup missing entry: ");
		} else if (res == 0) {
			++i; ++j;
			if (m->filesize == n->filesize && memcmp(m->data, n->data, m->filesize) == 0)
				continue;
			// Not the same!
			backup = 1;
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
		if (backup) {
			sprintf(buf, ".backup/%s", m->filename);
			free(m->filename);
			m->filename = strdup(buf);
			fprintf(stderr, "[%s] -> [%s]\n", buf, m->filename);
			vec_push_back(ret, m);
			// NULL the original entry, so it won't be freed
			vec_entry(o)[i - 1] = NULL;
		}
	}

	if (rem->filesize == 0)
		rem->remove = 1;

	// Cleanup
	cpio_vec_destroy(o);

	return ret;
}

static void cpio_restore(struct vector *v) {
	cpio_entry *f, *n;
	vec_for_each(v, f) {
		if (strncmp(f->filename, ".backup", 7) == 0) {
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
		if (strncmp(f->filename, "overlay", 7) == 0)
			f->remove = 1;
	}
	// Some known stuff we can remove
	cpio_rm(v, 0, "sbin/magic_mask.sh");
	cpio_rm(v, 0, "init.magisk.rc");
	cpio_rm(v, 0, "magisk");
	cpio_rm(v, 0, "ramdisk-recovery.xz");
}

static void restore_high_compress(struct vector *v, const char *incpio) {
	// Check if the ramdisk is in high compression mode
	if (cpio_extract(v, "ramdisk.cpio.xz", incpio) == 0) {
		void *xz;
		size_t size;
		full_read(incpio, &xz, &size);
		int fd = creat(incpio, 0644);
		lzma(0, fd, xz, size);
		close(fd);
		free(xz);
		cpio_rm(v, 0, "ramdisk.cpio.xz");
		cpio_rm(v, 0, "init");
		struct vector vv;
		vec_init(&vv);
		parse_cpio(&vv, incpio);
		cpio_entry *e;
		vec_for_each(&vv, e)
			vec_push_back(v, e);
		vec_destroy(&vv);
	}
}

static void enable_high_compress(struct vector *v, struct vector *b, const char *incpio) {
	cpio_entry *e, *magiskinit, *init;

	// Swap magiskinit with original init
	vec_for_each(b, e) {
		if (strcmp(e->filename, ".backup/init") == 0) {
			free(e->filename);
			e->filename = strdup("init");
			init = e;
			vec_for_each(v, e) {
				if (strcmp(e->filename, "init") == 0) {
					magiskinit = e;
					vec_cur(v) = init;
					break;
				}
			}
			vec_cur(b) = NULL;
			break;
		}
	}

	dump_cpio(v, incpio);
	cpio_vec_destroy(v);
	void *cpio;
	size_t size;
	full_read(incpio, &cpio, &size);
	int fd = creat(incpio, 0644);
	lzma(1, fd, cpio, size);
	close(fd);
	free(cpio);
	vec_init(v);
	vec_push_back(v, magiskinit);
	cpio_add(v, 0, "ramdisk.cpio.xz", incpio);
}

int cpio_commands(const char *command, int argc, char *argv[]) {
	char *incpio = argv[0];
	++argv;
	--argc;
	struct vector v;
	vec_init(&v);
	parse_cpio(&v, incpio);

	if (strcmp(command, "test") == 0) {
		exit(cpio_test(&v));
	} else if (strcmp(command, "restore") == 0) {
		restore_high_compress(&v, incpio);
		cpio_restore(&v);
	} else if (strcmp(command, "stocksha1") == 0) {
		printf("%s\n", cpio_stocksha1(&v));
		return 0;
	} else if (argc >= 4 && strcmp(command, "backup") == 0) {
		struct vector *back;
		cpio_entry *e;
		back = cpio_backup(&v, argv[0], argv[2], argv[3], argc > 4 ? argv[4] : NULL);

		// Enable high compression mode
		if (strcmp(argv[1], "true") == 0)
			enable_high_compress(&v, back, incpio);

		vec_for_each(back, e)
			if (e) vec_push_back(&v, e);

	} else if (argc > 0 && strcmp(command, "rm") == 0) {
		int recursive = 0;
		if (argc == 2 && strcmp(argv[0], "-r") == 0) {
			recursive = 1;
			++argv;
		}
		cpio_rm(&v, recursive, argv[0]);
	} else if (argc == 2 && strcmp(command, "mv") == 0) {
		if (cpio_mv(&v, argv[0], argv[1]))
			return 1;
	} else if (argc == 2 && strcmp(command, "patch") == 0) {
		cpio_patch(&v, strcmp(argv[0], "true") == 0, strcmp(argv[1], "true") == 0);
	} else if (strcmp(command, "extract") == 0) {
		if (argc == 2) {
			return cpio_extract(&v, argv[0], argv[1]);
		} else {
			cpio_extract_all(&v);
			return 0;
		}
	} else if (argc == 2 && strcmp(command, "mkdir") == 0) {
		cpio_mkdir(&v, strtoul(argv[0], NULL, 8), argv[1]);
	} else if (argc == 2 && strcmp(command, "ln") == 0) {
		cpio_ln(&v, argv[0], argv[1]);
	} else if (argc == 3 && strcmp(command, "add") == 0) {
		cpio_add(&v, strtoul(argv[0], NULL, 8), argv[1], argv[2]);
	} else {
		return 1;
	}

	dump_cpio(&v, incpio);
	cpio_vec_destroy(&v);
	exit(0);
}
