#pragma once

#include <stdlib.h>
#include <selinux.hpp>

#define ALL NULL

// policydb functions
int load_policydb(const char *file);
int load_split_cil();
int compile_split_cil();
int dump_policydb(const char *file);
void destroy_policydb();

// Handy functions
int sepol_allow(const char *s, const char *t, const char *c, const char *p);
int sepol_deny(const char *s, const char *t, const char *c, const char *p);
int sepol_auditallow(const char *s, const char *t, const char *c, const char *p);
int sepol_dontaudit(const char *s, const char *t, const char *c, const char *p);
int sepol_typetrans(const char *s, const char *t, const char *c, const char *d);
int sepol_typechange(const char *s, const char *t, const char *c, const char *d);
int sepol_typemember(const char *s, const char *t, const char *c, const char *d);
int sepol_nametrans(const char *s, const char *t, const char *c, const char *d, const char *o);
int sepol_allowxperm(const char *s, const char *t, const char *c, const char *range);
int sepol_auditallowxperm(const char *s, const char *t, const char *c, const char *range);
int sepol_dontauditxperm(const char *s, const char *t, const char *c, const char *range);
int sepol_create(const char *s);
int sepol_permissive(const char *s);
int sepol_enforce(const char *s);
int sepol_attradd(const char *s, const char *a);
int sepol_genfscon(const char *name, const char *path, const char *context);
int sepol_exists(const char *source);

// Built in rules
void sepol_magisk_rules();

// Statement parsing
void parse_statement(const char *statement);
void load_rule_file(const char *file);
void statement_help();
