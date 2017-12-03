#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "magiskboot.h"
#include "cpio.h"

int check_verity_pattern(const char *s) {
	int pos = 0;
	if (s[0] == ',') ++pos;
	if (strncmp(s + pos, "verify", 6) == 0)
		pos += 6;
	else if (strncmp(s + pos, "avb", 3) == 0)
		pos += 3;
	else
		return -1;

	if (s[pos] == '=') {
		while (s[pos] != '\0' && s[pos] != ' ' && s[pos] != '\n' && s[pos] != ',') ++pos;
	}
	return pos;
}

int check_encryption_pattern(const char *s) {
	const char *encrypt_list[] = { "forceencrypt", "forcefdeorfbe", NULL };
	for (int i = 0 ; encrypt_list[i]; ++i) {
		int len = strlen(encrypt_list[i]);
		if (strncmp(s, encrypt_list[i], len) == 0)
			return len;
	}
	return -1;
}

void cpio_patch(struct vector *v, int keepverity, int keepforceencrypt) {
	cpio_entry *f;
	int skip, write;
	vec_for_each(v, f) {
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

int cpio_commands(const char *command, int argc, char *argv[]) {
	int ret = 0;
	char *incpio = argv[0];
	++argv;
	--argc;
	struct vector v;
	vec_init(&v);
	parse_cpio(&v, incpio);

	if (strcmp(command, "test") == 0) {
		return cpio_test(&v);
	} else if (strcmp(command, "restore") == 0) {
		ret = cpio_restore(&v);
	} else if (strcmp(command, "stocksha1") == 0) {
		printf("%s\n", cpio_stocksha1(&v));
		return 0;
	} else if (argc >= 1 && strcmp(command, "backup") == 0) {
		cpio_backup(&v, argv[0], argc > 1 ? argv[1] : NULL);
	} else if (argc > 0 && strcmp(command, "rm") == 0) {
		int recursive = 0;
		if (argc == 2 && strcmp(argv[0], "-r") == 0) {
			recursive = 1;
			++argv;
			--argc;
		}
		cpio_rm(&v, recursive, argv[0]);
	} else if (argc == 2 && strcmp(command, "mv") == 0) {
		if (cpio_mv(&v, argv[0], argv[1]))
			return 1;
	} else if (argc == 2 && strcmp(command, "patch") == 0) {
		cpio_patch(&v, strcmp(argv[0], "true") == 0, strcmp(argv[1], "true") == 0);
	} else if (argc == 2 && strcmp(command, "extract") == 0) {
		return cpio_extract(&v, argv[0], argv[1]);
	} else if (argc == 2 && strcmp(command, "mkdir") == 0) {
		cpio_mkdir(&v, strtoul(argv[0], NULL, 8), argv[1]);
	} else if (argc == 3 && strcmp(command, "add") == 0) {
		cpio_add(&v, strtoul(argv[0], NULL, 8), argv[1], argv[2]);
	} else {
		return 1;
	}

	dump_cpio(&v, incpio);
	cpio_vec_destroy(&v);
	exit(ret);
}
