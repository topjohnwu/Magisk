#include <malloc.h>
#include <string.h>

#include <utils.h>

#include "magiskboot.h"

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

bool patch_verity(void **buf, uint32_t *size, bool patch) {
	int skip, src_size = *size;
	bool found = false;
	char *src = (char *) *buf, *patched = patch ? (char *) xcalloc(src_size, 1) : nullptr;
	for (int read = 0, write = 0; read < src_size; ++read, ++write) {
		if ((skip = check_verity_pattern(src + read)) > 0) {
			if (patch) {
				fprintf(stderr, "Remove pattern [%.*s]\n", skip, src + read);
				*size -= skip;
			} else {
				fprintf(stderr, "Found pattern [%.*s]\n", skip, src + read);
			}
			read += skip;
			found = true;
		}
		if (patch)
			patched[write] = src[read];
	}
	if (patch) {
		free(*buf);
		*buf = patched;
	}
	return found;
}

void patch_encryption(void **buf, uint32_t *size) {
	int skip, src_size = *size;
	char *src = (char *) *buf, *patched = (char *) xcalloc(src_size, 1);
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


