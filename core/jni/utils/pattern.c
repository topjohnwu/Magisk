#include <malloc.h>
#include <string.h>

#include "utils.h"

static int check_verity_pattern(const char *s) {
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

int patch_verity(char **buf, uint32_t *size, int patch) {
	int skip, found = 0;
	for (int pos = 0; pos < *size; ++pos) {
		if ((skip = check_verity_pattern(*buf + pos)) > 0) {
			found = 1;
			fprintf(stderr, "%s pattern [%.*s]\n", patch ? "Remove" : "Found", skip, *buf + pos);
			if (patch) {
				memcpy(*buf + pos, *buf + pos + skip, *size - pos - skip);
				memset(*buf + *size - skip, '\0', skip);
				*size -= skip;
			} else {
				pos += skip - 1;
			}
		}
	}
	return found;
}

void patch_encryption(char **buf, uint32_t *size) {
	int skip;
	for (int pos = 0; pos < *size; ++pos) {
		if ((skip = check_encryption_pattern(*buf + pos)) > 0) {
			fprintf(stderr, "Replace pattern [%.*s] with [encryptable]\n", skip, *buf + pos);
			memcpy(*buf + pos, "encryptable", 11);
			memcpy(*buf + pos + 11, *buf + pos + skip, *size - pos - skip);
			memset(*buf + *size - skip + 11, '\0', skip - 11);
			*size -= (skip - 11);
		}
	}
}


