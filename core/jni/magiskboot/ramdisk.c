#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utils.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "magiskboot.h"
#include "cpio.h"

static void cpio_patch(struct vector *v, int keepverity, int keepforceencrypt) {
	cpio_entry *e;
	vec_for_each(v, e) {
		if (!e) continue;
		if (!keepverity) {
			if (strncmp(e->filename, ".backup", 7) && strstr(e->filename, "fstab") && S_ISREG(e->mode)) {
				patch_verity(&e->data, &e->filesize, 1);
			} else if (strcmp(e->filename, "verity_key") == 0) {
				fprintf(stderr, "Remove [verity_key]\n");
				cpio_free(e);
				vec_cur(v) = NULL;
			}
		}
		if (!keepforceencrypt) {
			if (strstr(e->filename, "fstab") != NULL && S_ISREG(e->mode)) {
				patch_encryption(&e->data, &e->filesize);
			}
		}
	}
}

#define STOCK_BOOT      0x0
#define MAGISK_PATCH    0x1
#define OTHER_PATCH     0x2

static int cpio_test(struct vector *v) {
	const char *OTHER_LIST[] = { "sbin/launch_daemonsu.sh", "sbin/su", "init.xposed.rc", "boot/sbin/launch_daemonsu.sh", NULL };
	const char *MAGISK_LIST[] = { ".backup/.magisk", "init.magisk.rc", "overlay/init.magisk.rc", NULL };

	for (int i = 0; OTHER_LIST[i]; ++i)
		if (cpio_find(v, OTHER_LIST[i]) > 0)
			return OTHER_PATCH;

	for (int i = 0; MAGISK_LIST[i]; ++i)
		if (cpio_find(v, MAGISK_LIST[i]) > 0)
			return MAGISK_PATCH;

	return STOCK_BOOT;
}

static char *cpio_sha1(struct vector *v) {
	cpio_entry *e;
	char sha1[41];
	vec_for_each(v, e) {
		if (!e) continue;
		if (strcmp(e->filename, "init.magisk.rc") == 0
			|| strcmp(e->filename, "overlay/init.magisk.rc") == 0) {
			for (void *pos = e->data; pos < e->data + e->filesize; pos = strchr(pos + 1, '\n') + 1) {
				if (memcmp(pos, "# STOCKSHA1=", 12) == 0) {
					pos += 12;
					memcpy(sha1, pos, 40);
					sha1[40] = '\0';
					return strdup(sha1);
				}
			}
		} else if (strcmp(e->filename, ".backup/.sha1") == 0) {
			return e->data;
		}
	}
	return NULL;
}

static void cpio_backup(struct vector *v, struct vector *bak, const char *orig, const char *sha1) {
	struct vector o_body, *o = &o_body;
	cpio_entry *m, *n, *rem, *cksm;
	char buf[PATH_MAX];
	int res, backup;

	m = xcalloc(sizeof(*m), 1);
	m->filename = strdup(".backup");
	m->mode = S_IFDIR;
	vec_push_back(bak, m);

	rem = xcalloc(sizeof(*rem), 1);
	rem->filename = strdup(".backup/.rmlist");
	rem->mode = S_IFREG;

	if (sha1) {
		fprintf(stderr, "Save SHA1: [%s] -> [.backup/.sha1]\n", sha1);
		cksm = xcalloc(sizeof(*cksm), 1);
		vec_push_back(bak, cksm);
		cksm->filename = strdup(".backup/.sha1");
		cksm->mode = S_IFREG;
		cksm->data = strdup(sha1);
		cksm->filesize = strlen(sha1) + 1;
	}

	vec_init(o);
	parse_cpio(o, orig);
	// Remove possible backups in original ramdisk
	cpio_rm(o, 1, ".backup");
	cpio_rm(v, 1, ".backup");

	// Sort both vectors before comparing
	vec_sort(o, cpio_cmp);
	vec_sort(v, cpio_cmp);

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
			rem->data = xrealloc(rem->data, rem->filesize + strlen(n->filename) + 1);
			memcpy(rem->data + rem->filesize, n->filename, strlen(n->filename) + 1);
			rem->filesize += strlen(n->filename) + 1;
			fprintf(stderr, "Record new entry: [%s] -> [.backup/.rmlist]\n", n->filename);
		}
		if (backup) {
			sprintf(buf, ".backup/%s", m->filename);
			fprintf(stderr, "[%s] -> [%s]\n", m->filename, buf);
			free(m->filename);
			m->filename = strdup(buf);
			vec_push_back(bak, m);
			// NULL the original entry, so it won't be freed
			vec_entry(o)[i - 1] = NULL;
		}
	}

	if (rem->filesize)
		vec_push_back(bak, rem);
	else
		cpio_free(rem);

	// Cleanup
	cpio_vec_destroy(o);
}

static void cpio_restore(struct vector *v) {
	cpio_entry *e;
	vec_for_each(v, e) {
		if (!e) continue;
		if (strncmp(e->filename, ".backup", 7) == 0) {
			if (e->filename[7] == '\0') continue;
			if (e->filename[8] == '.') {
				if (strcmp(e->filename, ".backup/.rmlist") == 0) {
					for (int pos = 0; pos < e->filesize; pos += strlen(e->data + pos) + 1)
						cpio_rm(v, 0, e->data + pos);
				}
				continue;
			} else {
				fprintf(stderr, "Restore [%s] -> [%s]\n", e->filename, e->filename + 8);
				vec_cur(v) = NULL;
				char *new_name = strdup(e->filename + 8);
				free(e->filename);
				e->filename = new_name;
				cpio_vec_insert(v, e);
			}
		}
	}
	// Some known stuff we can remove
	cpio_rm(v, 1, ".backup");
	cpio_rm(v, 1, "overlay");
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
	cpio_entry *init, *magiskinit;

	// Swap magiskinit with original init
	int i = cpio_find(b, ".backup/init"), j = cpio_find(v, "init");
	init = vec_entry(b)[i];
	magiskinit = vec_entry(v)[j];
	free(init->filename);
	init->filename = strdup("init");
	vec_entry(v)[j] = init;
	vec_entry(b)[i] = NULL;

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

int cpio_commands(int argc, char *argv[]) {
	char *incpio = argv[0];
	++argv;
	--argc;

	struct vector v;
	vec_init(&v);
	parse_cpio(&v, incpio);

	int cmdc;
	char *cmdv[6];

	while (argc) {
		cmdc = 0;
		for (char *tok = strtok(argv[0], " "); tok; tok = strtok(NULL, " "))
			cmdv[cmdc++] = tok;

		if (strcmp(cmdv[0], "test") == 0) {
			exit(cpio_test(&v));
		} else if (strcmp(cmdv[0], "restore") == 0) {
			restore_high_compress(&v, incpio);
			cpio_restore(&v);
		} else if (strcmp(cmdv[0], "sha1") == 0) {
			char *sha1 = cpio_sha1(&v);
			if (sha1)
				printf("%s\n", sha1);
			return 0;
		} else if (cmdc >= 2 && strcmp(cmdv[0], "backup") == 0) {
			struct vector back;
			vec_init(&back);
			cpio_backup(&v, &back, cmdv[1], cmdc > 2 ? cmdv[2] : NULL);
			cpio_entry *e;
			vec_for_each(&back, e)
				if (e) vec_push_back(&v, e);
			vec_destroy(&back);
		} else if (cmdc >= 5 && strcmp(cmdv[0], "magisk") == 0) {
			cpio_patch(&v, strcmp(cmdv[3], "true") == 0, strcmp(cmdv[4], "true") == 0);

			struct vector back;
			vec_init(&back);
			cpio_backup(&v, &back, cmdv[1], cmdc > 5 ? cmdv[5] : NULL);

			cpio_entry *e;
			e = xcalloc(sizeof(*e), 1);
			e->filename = strdup(".backup/.magisk");
			e->mode = S_IFREG;
			e->data = xmalloc(50);
			snprintf(e->data, 50, "KEEPVERITY=%s\nKEEPFORCEENCRYPT=%s\n", cmdv[3], cmdv[4]);
			e->filesize = strlen(e->data) + 1;
			vec_push_back(&back, e);

			// Enable high compression mode
			if (strcmp(cmdv[2], "true") == 0)
				enable_high_compress(&v, &back, incpio);

			vec_for_each(&back, e)
				if (e) vec_push_back(&v, e);
			vec_destroy(&back);
		} else if (cmdc >= 2 && strcmp(cmdv[0], "rm") == 0) {
			int recur = cmdc > 2 && strcmp(cmdv[1], "-r") == 0;
			cpio_rm(&v, recur, cmdv[1 + recur]);
		} else if (cmdc == 3 && strcmp(cmdv[0], "mv") == 0) {
			cpio_mv(&v, cmdv[1], cmdv[2]);
		} else if (cmdc == 3 && strcmp(cmdv[0], "patch") == 0) {
			cpio_patch(&v, strcmp(cmdv[1], "true") == 0, strcmp(cmdv[2], "true") == 0);
		} else if (strcmp(cmdv[0], "extract") == 0) {
			if (cmdc == 3) {
				return cpio_extract(&v, cmdv[1], cmdv[2]);
			} else {
				cpio_extract_all(&v);
				return 0;
			}
		} else if (cmdc == 3 && strcmp(cmdv[0], "mkdir") == 0) {
			cpio_mkdir(&v, strtoul(cmdv[1], NULL, 8), cmdv[2]);
		} else if (cmdc == 3 && strcmp(cmdv[0], "ln") == 0) {
			cpio_ln(&v, cmdv[1], cmdv[2]);
		} else if (cmdc == 4 && strcmp(cmdv[0], "add") == 0) {
			cpio_add(&v, strtoul(cmdv[1], NULL, 8), cmdv[2], cmdv[3]);
		} else {
			return 1;
		}

		--argc;
		++argv;
	}

	dump_cpio(&v, incpio);
	cpio_vec_destroy(&v);
	return 0;
}
