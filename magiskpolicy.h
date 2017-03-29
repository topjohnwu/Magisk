#ifndef MAGISKPOLICY_H
#define MAGISKPOLICY_H

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
int load_policy(const char *filename);
int dump_policy(const char *filename);
int create_domain(char *d);
int set_domain_state(char* s, int state);
int add_transition(char *s, char *t, char *c, char *d);
int add_file_transition(char *s, char *t, char *c, char *d, char* filename);
int add_typeattribute(char *domainS, char *attr);
int add_rule(char *s, char *t, char *c, char *p, int effect, int not);

// Handy functions
int allow(char *s, char *t, char *c, char *p);
int deny(char *s, char *t, char *c, char *p);
int auditallow(char *s, char *t, char *c, char *p);
int auditdeny(char *s, char *t, char *c, char *p);
int typetrans(char *s, char *t, char *c, char *d, char *o);
int create(char *s);
int permissive(char *s);
int enforce(char *s);
int attradd(char *s, char *a);
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
void full_rules();
void min_rules();

#endif
