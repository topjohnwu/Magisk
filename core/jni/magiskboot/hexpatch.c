#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>

#include "magiskboot.h"
#include "utils.h"

static void hex2byte(const char *hex, unsigned char *str) {
	char high, low;
	for (int i = 0, length = strlen(hex); i < length; i += 2) {
		high = toupper(hex[i]) - '0';
		low = toupper(hex[i + 1]) - '0';
		str[i / 2] = ((high > 9 ? high - 7 : high) << 4) + (low > 9 ? low - 7 : low);
	}
}

void hexpatch(const char *image, const char *from, const char *to) {
	int patternsize = strlen(from) / 2, patchsize = strlen(to) / 2;
	size_t filesize;
	void *file, *pattern, *patch;
	mmap_rw(image, &file, &filesize);
	pattern = xmalloc(patternsize);
	patch = xmalloc(patchsize);
	hex2byte(from, pattern);
	hex2byte(to, patch);
	for (size_t i = 0; filesize > 0 && i < filesize - patternsize; ++i) {
		if (memcmp(file + i, pattern, patternsize) == 0) {
			fprintf(stderr, "Pattern %s found!\nPatching to %s\n", from, to);
			memset(file + i, 0, patternsize);
			memcpy(file + i, patch, patchsize);
			i += patternsize - 1;
		}
	}
	munmap(file, filesize);
	free(pattern);
	free(patch);
}
