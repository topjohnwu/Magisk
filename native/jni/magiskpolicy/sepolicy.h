#pragma once

#include <sepol/policydb/policydb.h>

__BEGIN_DECLS

// Internal C APIs, do not use directly
int create_domain(policydb_t *db, const char *d);
int set_domain_state(policydb_t *db, const char *s, int state);
int add_typeattribute(policydb_t *db, const char *type, const char *attr);
int add_rule(policydb_t *db, const char *s, const char *t, const char *c, const char *p, int effect,
			 int n);
int add_xperm_rule(policydb_t *db, const char *s, const char *t, const char *c, const char *range,
				   int effect, int n);
int add_type_rule(policydb_t *db, const char *s, const char *t, const char *c, const char *d,
				  int effect);
int add_filename_trans(policydb_t *db, const char *s, const char *t, const char *c, const char *d,
					   const char *o);
int add_genfscon(policydb_t *db, const char *name, const char *path, const char *context);
void strip_dontaudit(policydb_t *db);

void statement_help();

__END_DECLS
