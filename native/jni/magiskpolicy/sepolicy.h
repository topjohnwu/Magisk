#pragma once

#include <sepol/policydb/policydb.h>

__BEGIN_DECLS

// Global policydb
extern policydb_t *magisk_policydb;

int create_domain(const char *d);
int set_domain_state(const char *s, int state);
int add_typeattribute(const char *type, const char *attr);
int add_rule(const char *s, const char *t, const char *c, const char *p, int effect, int n);
int add_xperm_rule(const char *s, const char *t, const char *c, const char *range, int effect, int n);
int add_type_rule(const char *s, const char *t, const char *c, const char *d, int effect);
int add_filename_trans(const char *s, const char *t, const char *c, const char *d, const char *o);
int add_genfscon(const char *name, const char *path, const char *context);
void strip_dontaudit();

__END_DECLS
