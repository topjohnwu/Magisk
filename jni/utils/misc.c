/* misc.c - Store all functions that are unable to be catagorized clearly
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

int check_data() {
	FILE *fp = xfopen("/proc/mounts", "r");
	while (fgets(magiskbuf, BUF_SIZE, fp)) {
		if (strstr(magiskbuf, " /data ")) {
			if (strstr(magiskbuf, "tmpfs"))
				return 0;
			else
				return 1;
		}
	}
	return 0;
}

/* All the string should be freed manually!! */
void file_to_vector(struct vector *v, FILE *fp) {
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != -1) {
		// Remove end newline
		if (line[read - 1] == '\n')
			line[read - 1] = '\0';
		vec_push_back(v, line);
		line = NULL;
	}
}
