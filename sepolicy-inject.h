#ifndef SEPOLICY_INJECT_H
#define SEPOLICY_INJECT_H

#define ALL NULL

#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sepol/debug.h>
#include <sepol/policydb/policydb.h>
#include <sepol/policydb/expand.h>
#include <sepol/policydb/link.h>
#include <sepol/policydb/services.h>
#include <sepol/policydb/avrule_block.h>
#include <sepol/policydb/conditional.h>
#include <sepol/policydb/constraint.h>

// hashtab traversal macro
#define hashtab_for_each(table, ptr) \
	for (int _i = 0; _i < table->size; ++_i) \
		for (*ptr = table->htable[_i]; *ptr != NULL; *ptr = (*ptr)->next)

// Global policydb
policydb_t *policy;

// sepolicy manipulation functions
int load_policy(char *filename, policydb_t *policydb, struct policy_file *pf);
void create_domain(char *d);
int set_domain_state(char* s, int state);
int add_file_transition(char *srcS, char *origS, char *tgtS, char *c, char* filename);
int add_transition(char *srcS, char *origS, char *tgtS, char *c);
int add_typeattribute(char *domainS, char *typeS);
int add_rule(char *s, char *t, char *c, char *p, int effect, int not);
int add_typerule(char *s, char *targetAttribute, char **minusses, char *c, char *p, int effect, int not);

// Handy functions
void allow(char *s, char *t, char *c, char *p);
void deny(char *s, char *t, char *c, char *p);
void auditallow(char *s, char *t, char *c, char *p);
void auditdeny(char *s, char *t, char *c, char *p);
void permissive(char *s);
void enforce(char *s);
void attradd(char *s, char *a);
int exists(char *source);

// Vector of char*
typedef struct vector {
	size_t size;
	size_t cap;
	char **data;
} vector;
void vec_init(vector *v);
void vec_push_back(vector *v, char* s);
void vec_destroy(vector *v);

// Built in rules
void su_rules();
void min_rules();

#endif
