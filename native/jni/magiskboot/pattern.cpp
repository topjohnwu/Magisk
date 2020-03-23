#include <malloc.h>
#include <string.h>

#include <utils.hpp>

#include "magiskboot.hpp"

#define MATCH(p) else if (strncmp(s + skip, p, sizeof(p) - 1) == 0) skip += (sizeof(p) - 1)

static int check_verity_pattern(const char *s) {
	int skip = s[0] == ',';

	if (0) {}
	MATCH("verifyatboot");
	MATCH("verify");
	MATCH("avb_keys");
	MATCH("avb");
	MATCH("support_scfs");
	MATCH("fsverity");
	else return -1;

	if (s[skip] == '=') {
		while (s[skip] != '\0' && s[skip] != ' ' && s[skip] != '\n' && s[skip] != ',')
			++skip;
	}
	return skip;
}

#undef MATCH
#define MATCH(p) else if (strncmp(s, p, sizeof(p) - 1) == 0) return (sizeof(p) - 1)

static int check_encryption_pattern(const char *s) {
	if (0) {}
	MATCH("forceencrypt");
	MATCH("forcefdeorfbe");
	MATCH("fileencryption");
	else return -1;
}

char *patch_verity(const void *buf, uint32_t &size, bool inplace) {
	auto src = static_cast<const char *>(buf);
	auto dest = (char *)(inplace ? buf : xmalloc(size));
	int src_size = size;
	bool found = false;
	int write = 0;
	for (int read = 0; read < src_size;) {
		if (int skip; (skip = check_verity_pattern(src + read)) > 0) {
			fprintf(stderr, "Found pattern [%.*s]\n", skip, src + read);
			size -= skip;
			read += skip;
			found = true;
		} else {
			dest[write++] = src[read++];
		}
	}
	dest[write] = '\0';
	if (!found) {
		if (!inplace)
			free(dest);
		return nullptr;
	}
	return dest;
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
