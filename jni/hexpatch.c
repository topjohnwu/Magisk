#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "bootimgtools.h"

int hex2int(char c) {
	int first = c / 16 - 3;
	int second = c % 16;
	int result = first * 10 + second;
	if(result > 9) result--;
	return result;
}

int hex2ascii(char c, char d) {
	int high = hex2int(c) * 16;
	int low = hex2int(d);
	return high+low;
}

void hexstr2str(char *hex, char *str) {
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
	off_t filesize = lseek(fd, 0, SEEK_END);
	char *file, *pattern, *patch, *start;
	file = malloc(sizeof (char) * filesize);
	pattern = malloc(sizeof (char) * patternsize);
	patch = malloc(sizeof (char) * patchsize);
	lseek(fd, 0, SEEK_SET);
	read(fd, file, filesize);
	hexstr2str(from, pattern);
	hexstr2str(to, patch);
	for (off_t i = 0; i < filesize;) {
		int j;
		for (j = 0; j < patternsize; ++j) {
			if(file[i + j] != pattern[j]) break;
		}
		if (j == patternsize) {
			fprintf(stderr, "Pattern %s found!\nPatching to %s\n", from, to);
			lseek(fd, i, SEEK_SET);
			write(fd, patch, patchsize);
		}
		if(j == 0) j = 1;
		i += j;
	}
	free(file);
	free(pattern);
	free(patch);
	close(fd);
	return 0;
}