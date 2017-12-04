/* vector.c - A simple vector implementation in c
 */

#include <stdlib.h>
#include <string.h>

#include "vector.h"

void vec_init(struct vector *v) {
	if (v == NULL) return;
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

void *vec_pop_back(struct vector *v) {
	void *ret = vec_entry(v)[vec_size(v) - 1];
	--vec_size(v);
	return ret;
}

void vec_sort(struct vector *v, int (*compar)(const void *, const void *)) {
	if (v == NULL) return;
	qsort(vec_entry(v), vec_size(v), sizeof(void*), compar);
}

/* Will cleanup only the vector itself
 * use in cases when each element requires special cleanup 
 */
void vec_destroy(struct vector *v) {
	if (v == NULL) return;
	vec_size(v) = 0;
	vec_cap(v) = 0;
	free(vec_entry(v));
	vec_entry(v) = NULL; // Prevent double destroy segfault
}

/* Will cleanup each element AND the vector itself
 * Shall be the general case
 */
void vec_deep_destroy(struct vector *v) {
	if (v == NULL) return;
	void *e;
	vec_for_each(v, e) {
		free(e);
	}
	vec_destroy(v);
}

struct vector *vec_dup(struct vector *v) {
	struct vector *ret = malloc(sizeof(*ret));
	vec_size(ret) = vec_size(v);
	vec_cap(ret) = vec_cap(v);
	vec_entry(v) = malloc(sizeof(void*) * vec_cap(ret));
	memcpy(vec_entry(ret), vec_entry(v), sizeof(void*) * vec_cap(ret));
	return ret;
}
