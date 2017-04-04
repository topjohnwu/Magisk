/* vector.h - A simple vector implementation in c
 */

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <sys/types.h>

struct vector {
	size_t size;
	size_t cap;
	void **data;
};
void vec_init(struct vector *v);
void vec_push_back(struct vector *v, void *p);
void vec_sort(struct vector *v, int (*compar)(const void *, const void *));
void vec_destroy(struct vector *v);
#define vec_size(v) (v)->size
#define vec_cap(v) (v)->cap
#define vec_entry(v) (v)->data
/* Usage: vec_for_each(vector *v, void *e) */
#define vec_for_each(v, e) \
	e = (v)->data[0]; \
	for (size_t _ = 0; _ < (v)->size; ++_, e = (v)->data[_])

#endif