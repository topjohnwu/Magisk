#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>

#include "magiskboot.h"
#include "utils.h"

static void hex2byte(uint8_t *hex, uint8_t *str) {
	char high, low;
	for (int i = 0, length = strlen((char *) hex); i < length; i += 2) {
		high = toupper(hex[i]) - '0';
		low = toupper(hex[i + 1]) - '0';
		str[i / 2] = ((high > 9 ? high - 7 : high) << 4) + (low > 9 ? low - 7 : low);
	}
}

void hexpatch(const char *image, const char *from, const char *to) {
	int patternsize = strlen(from) / 2, patchsize = strlen(to) / 2;
	size_t filesize;
	uint8_t *file, *pattern, *patch;
	mmap_rw(image, (void **) &file, &filesize);
	pattern = (uint8_t *) xmalloc(patternsize);
	patch = (uint8_t *) xmalloc(patchsize);
	hex2byte((uint8_t *) from, pattern);
	hex2byte((uint8_t *) to, patch);
	for (size_t i = 0; filesize > 0 && i < filesize - patternsize; ++i) {
		if (memcmp(file + i, pattern, patternsize) == 0) {
			fprintf(stderr, "Patch @ %08X [%s]->[%s]\n", (unsigned) i, from, to);
			memset(file + i, 0, patternsize);
			memcpy(file + i, patch, patchsize);
			i += patternsize - 1;
		}
	}
	munmap(file, filesize);
	free(pattern);
	free(patch);
}
