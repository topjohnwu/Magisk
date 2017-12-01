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
	} else if (argc >= 1 && strcmp(command, "backup") == 0) {
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
		cpio_backup(argv[0], argc > 1 ? argv[1] : NULL, &v);
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
