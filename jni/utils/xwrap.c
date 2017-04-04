/* xwrap.c - wrappers around existing library functions.
 *
 * Functions with the x prefix are wrappers that either succeed or kill the
 * program with an error message, but never return failure. They usually have
 * the same arguments and return value as the function they wrap.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

FILE *xfopen(const char *pathname, const char *mode) {
	FILE *fp = fopen(pathname, mode);
	if (fp == NULL) {
		PLOGE("fopen");
		exit(1);
	}
	return fp;
}