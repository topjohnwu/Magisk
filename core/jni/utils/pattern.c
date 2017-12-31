#include <malloc.h>
#include <string.h>

#include "utils.h"

static int check_verity_pattern(const char *s) {
	int skip = 0;
	if (s[0] == ',') ++skip;
	if (strncmp(s + skip, "verify", 6) == 0)
		skip += 6;
	else if (strncmp(s + skip, "avb", 3) == 0)
		skip += 3;
	else
		return -1;

	if (s[skip] == '=') {
		while (s[skip] != '\0' && s[skip] != ' ' && s[skip] != '\n' && s[skip] != ',') ++skip;
	}
	return skip;
}

static int check_encryption_pattern(const char *s) {
	const char *encrypt_list[] = { "forceencrypt", "forcefdeorfbe", NULL };
	for (int i = 0 ; encrypt_list[i]; ++i) {
		int len = strlen(encrypt_list[i]);
		if (strncmp(s, encrypt_list[i], len) == 0)
			return len;
	}
	return -1;
}

void patch_init_rc(void **buf, size_t *size) {
	int injected = 0;
	char *new_data = malloc(*size + 23);
	char *old_data = *buf;
	size_t pos = 0;

	for (char *tok = strsep(&old_data, "\n"); tok; tok = strsep(&old_data, "\n")) {
		if (!injected && strncmp(tok, "import", 6) == 0) {
			if (strstr(tok, "init.magisk.rc")) {
				injected = 1;
			} else {
				strcpy(new_data + pos, "import /init.magisk.rc\n");
				pos += 23;
				injected = 1;
			}
		} else if (strstr(tok, "selinux.reload_policy")) {
			continue;
		}
		// Copy the line
		strcpy(new_data + pos, tok);
		pos += strlen(tok);
		new_data[pos++] = '\n';
	}

	free(*buf);
	*size = pos;
	*buf = new_data;
}

int patch_verity(void **buf, uint32_t *size, int patch) {
	int skip, src_size = *size;
	char *src = *buf, *patched = patch ? xcalloc(src_size, 1) : NULL;
	for (int read = 0, write = 0; read < src_size; ++read, ++write) {
		if ((skip = check_verity_pattern(src + read)) > 0) {
			if (!patch)
				return 1;
			fprintf(stderr, "Remove pattern [%.*s]\n", skip, src + read);
			read += skip;
			*size -= skip;
		}
		if (patch)
			patched[write] = src[read];
	}
	if (patch) {
		free(*buf);
		*buf = patched;
	}
	return 0;
}

void patch_encryption(void **buf, uint32_t *size) {
	int skip, src_size = *size;
	char *src = *buf, *patched = xcalloc(src_size, 1);
	for (int read = 0, write = 0; read < src_size; ++read, ++write) {
		if ((skip = check_encryption_pattern(src + read)) > 0) {
			fprintf(stderr, "Replace pattern [%.*s] with [encryptable]\n", skip, src + read);
			memcpy(patched + read, "encryptable", 11);
			read += skip;
			write += 11;
			*size -= (skip - 11);
		}
		patched[write] = src[read];
	}
	free(*buf);
	*buf = patched;
}


