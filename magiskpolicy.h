/* magiskpolicy.h - Public API for policy patching
 */

#ifndef _MAGISKPOLICY_H
#define _MAGISKPOLICY_H

#include <stdlib.h>

#define ALL NULL

// policydb functions
int load_policydb(const char *filename);
int dump_policydb(const char *filename);
void destroy_policydb();

// Handy functions
int sepol_allow(char *s, char *t, char *c, char *p);
int sepol_deny(char *s, char *t, char *c, char *p);
int sepol_auditallow(char *s, char *t, char *c, char *p);
int sepol_auditdeny(char *s, char *t, char *c, char *p);
int sepol_typetrans(char *s, char *t, char *c, char *d, char *o);
int sepol_allowxperm(char *s, char *t, char *c, char *range);
int sepol_auditallowxperm(char *s, char *t, char *c, char *range);
int sepol_dontauditxperm(char *s, char *t, char *c, char *range);
int sepol_create(char *s);
int sepol_permissive(char *s);
int sepol_enforce(char *s);
int sepol_attradd(char *s, char *a);
int sepol_exists(char *source);

// Built in rules
void sepol_magisk_rules();

#endif
