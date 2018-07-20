/* sepolicy.h - Header for magiskpolicy non-public APIs
 */

#ifndef _SEPOLICY_H
#define _SEPOLICY_H

#include <sepol/policydb/policydb.h>

// Global policydb
extern policydb_t *policydb;

// hashtab traversal macro
#define hashtab_for_each(table, ptr) \
	for (int _i = 0; _i < table->size; ++_i) \
		for (*ptr = table->htable[_i]; *ptr != NULL; *ptr = (*ptr)->next)

// sepolicy manipulation functions
int create_domain(char *d);
int set_domain_state(char* s, int state);
int add_transition(char *s, char *t, char *c, char *d);
int add_file_transition(char *s, char *t, char *c, char *d, char* filename);
int add_typeattribute(char *domainS, char *attr);
int add_rule(char *s, char *t, char *c, char *p, int effect, int not);
int add_xperm_rule(char *s, char *t, char *c, char *range, int effect, int not);

#endif
