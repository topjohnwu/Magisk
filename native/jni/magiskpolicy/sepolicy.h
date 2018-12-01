/* sepolicy.h - Header for magiskpolicy non-public APIs
 */

#ifndef _SEPOLICY_H
#define _SEPOLICY_H

#include <sepol/policydb/policydb.h>

#ifdef __cplusplus
extern "C" {
#endif

// Global policydb
extern policydb_t *policydb;

// General hash table traversal
#define hash_for_each(table, slots, tab, cur, block) \
	for (int __i = 0; __i < (tab)->slots; ++__i) { \
		__typeof__(cur) __next; \
		for (cur = (tab)->table[__i]; cur; cur = __next) { \
			__next = cur->next; \
			block \
		} \
	} \

// hashtab traversal
#define hashtab_for_each(hashtab, cur, block) hash_for_each(htable, size, hashtab, cur, block)

// avtab traversal
#define avtab_for_each(avtab, cur, block) hash_for_each(htable, nslot, avtab, cur, block)

int create_domain(const char *d);
int set_domain_state(const char *s, int state);
int add_typeattribute(const char *domainS, const char *attr);
int add_rule(const char *s, const char *t, const char *c, const char *p, int effect, int n);
int add_xperm_rule(const char *s, const char *t, const char *c, const char *range, int effect, int n);
int add_type_rule(const char *s, const char *t, const char *c, const char *d, int effect);

#ifdef __cplusplus
};
#endif

#endif
