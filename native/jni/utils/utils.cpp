#include <unistd.h>

#include "utils.hpp"

int file_to_array(const char *filename, Array<char *> &arr) {
	if (access(filename, R_OK) != 0)
		return 1;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	FILE *fp = xfopen(filename, "r");
	if (fp == NULL)
		return 1;

	while ((read = getline(&line, &len, fp)) != -1) {
		// Remove end newline
		if (line[read - 1] == '\n')
			line[read - 1] = '\0';
		arr.push_back(line);
		line = NULL;
	}
	fclose(fp);
	return 0;
}
