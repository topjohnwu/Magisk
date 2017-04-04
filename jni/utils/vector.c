/* vector.c - A simple vector implementation in c
 */

#include <stdlib.h>

#include "vector.h"

void vec_init(struct vector *v) {
	vec_size(v) = 0;
	vec_cap(v) = 1;
	vec_entry(v) = malloc(sizeof(void*));
}

void vec_push_back(struct vector *v, void *p) {
	if (v == NULL) return;
	if (vec_size(v) == vec_cap(v)) {
		vec_cap(v) *= 2;
		vec_entry(v) = realloc(vec_entry(v), sizeof(void*) * vec_cap(v));
	}
	vec_entry(v)[vec_size(v)] = p;
	++vec_size(v);
}

void vec_sort(struct vector *v, int (*compar)(const void *, const void *)) {
	qsort(vec_entry(v), vec_size(v), sizeof(void*), compar);
}

void vec_destroy(struct vector *v) {
	// Will not free each entry!
	// Manually free each entry, then call this function
	vec_size(v) = 0;
	vec_cap(v) = 0;
	free(v->data);
}