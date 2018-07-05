/* vector.c - A simple vector implementation in c
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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

void vec_push_back_all(struct vector *v, void *p, ...) {
	va_list argv;
	va_start(argv, p);
	vec_push_back(v, p);
	for (void *arg = va_arg(argv, char*); arg; arg = va_arg(argv, char*))
		vec_push_back(v, arg);
	va_end(argv);
}

void *vec_pop_back(struct vector *v) {
	void *ret = vec_entry(v)[vec_size(v) - 1];
	--vec_size(v);
	return ret;
}

static int (*cmp)(const void *, const void *);

static int vec_comp(const void *a, const void *b) {
	void *aa = *((void **)a), *bb = *((void **)b);
	if (aa == NULL && bb == NULL) return 0;
	else if (aa == NULL) return 1;
	else if (bb == NULL) return -1;
	else return cmp ? cmp(aa, bb) : 0;
}

void vec_sort(struct vector *v, int (*compar)(const void *, const void *)) {
	if (v == NULL) return;
	cmp = compar;
	qsort(vec_entry(v), vec_size(v), sizeof(void*), vec_comp);
	void *e;
	vec_for_each_r(v, e) {
		if (e) break;
		--vec_size(v);
	}
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

void vec_dup(struct vector *v, struct vector *vv) {
	vec_size(vv) = vec_size(v);
	vec_cap(vv) = vec_cap(v);
	vec_entry(vv) = malloc(sizeof(void*) * vec_cap(v));
	memcpy(vec_entry(vv), vec_entry(v), sizeof(void*) * vec_cap(v));
}
