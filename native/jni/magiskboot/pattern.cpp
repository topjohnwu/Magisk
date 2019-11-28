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
	static const char *encrypt_list[] = { "forceencrypt", "forcefdeorfbe" };
	for (auto enc : encrypt_list) {
		int len = strlen(enc);
		if (strncmp(s, enc, len) == 0)
			return len;
	}
	return -1;
}

char *patch_verity(const void *buf, uint32_t &size, bool inplace) {
	auto src = static_cast<const char *>(buf);
	int src_size = size;
	bool found = false;
	auto patched = (char *)(inplace ? buf : xmalloc(size));
	int write = 0;
	for (int read = 0; read < src_size; ++read, ++write) {
		if (int skip; (skip = check_verity_pattern(src + read)) > 0) {
			fprintf(stderr, "Found pattern [%.*s]\n", skip, src + read);
			size -= skip;
			read += skip;
			found = true;
		}
		patched[write] = src[read];
	}
	patched[write] = '\0';
	if (!found) {
		if (!inplace)
			free(patched);
		return nullptr;
	}
	return patched;
}

void patch_encryption(void *buf, uint32_t &size) {
	auto src = static_cast<char *>(buf);
	int src_size = size;
	int write = 0;
	for (int read = 0; read < src_size; ++read, ++write) {
		if (int skip; (skip = check_encryption_pattern(src + read)) > 0) {
			fprintf(stderr, "Found pattern [%.*s]\n", skip, src + read);
			size -= skip;
			read += skip;
		}
		src[write] = src[read];
	}
	src[write] = '\0';
}
