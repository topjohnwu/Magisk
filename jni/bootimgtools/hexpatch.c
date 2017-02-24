#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "bootimg.h"

static int hex2int(char c) {
	int first = c / 16 - 3;
	int second = c % 16;
	int result = first * 10 + second;
	if(result > 9) result--;
	return result;
}

static unsigned hex2ascii(char c, char d) {
	int high = hex2int(c) * 16;
	int low = hex2int(d);
	return high + low;
}

static void hexstr2str(char *hex, unsigned char *str) {
	char buf = 0;
	for(int i = 0, length = strlen(hex); i < length; ++i){
		if(i % 2){
			str[i / 2] = hex2ascii(buf, hex[i]);
		} else{
			buf = hex[i];
		}
	}
}

int hexpatch(char * image, char *from, char *to) {
	int fd = open(image, O_RDWR), patternsize = strlen(from) / 2, patchsize = strlen(to) / 2;
	size_t filesize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	unsigned char *file, *pattern, *patch;
	file = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	pattern = malloc(patternsize);
	patch = malloc(patchsize);
	hexstr2str(from, pattern);
	hexstr2str(to, patch);
	for (size_t i = 0; i < filesize - patternsize; ++i) {
		if (memcmp(file + i, pattern, patternsize) == 0) {
			printf("Pattern %s found!\nPatching to %s\n", from, to);
			memset(file + i, 0, patternsize);
			memcpy(file + i, patch, patchsize);
			i += patternsize - 1;
		}
	}
	munmap(file, filesize);
	free(pattern);
	free(patch);
	close(fd);
	return 0;
}