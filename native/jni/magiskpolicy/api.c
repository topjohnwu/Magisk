#include "magiskpolicy.h"
#include "sepolicy.h"

int sepol_allow(char *s, char *t, char *c, char *p) {
	// printf("allow %s %s %s %s\n", s, t, c, p);
	return add_rule(s, t, c, p, AVTAB_ALLOWED, 0);
}

int sepol_deny(char *s, char *t, char *c, char *p) {
	// printf("deny %s %s %s %s\n", s, t, c, p);
	return add_rule(s, t, c, p, AVTAB_ALLOWED, 1);
}

int sepol_auditallow(char *s, char *t, char *c, char *p) {
	// printf("auditallow %s %s %s %s\n", s, t, c, p);
	return add_rule(s, t, c, p, AVTAB_AUDITALLOW, 0);
}

int sepol_auditdeny(char *s, char *t, char *c, char *p) {
	// printf("auditdeny %s %s %s %s\n", s, t, c, p);
	return add_rule(s, t, c, p, AVTAB_AUDITDENY, 0);
}

int sepol_typetrans(char *s, char *t, char *c, char *d, char *o) {
	if (o == NULL) {
		// printf("add_trans %s %s %s %s\n", s, t, c ,d);
		return add_transition(s, t, c, d);
	} else {
		// printf("add_file_trans %s %s %s %s %s\n", s, t, c ,d, o);
		return add_file_transition(s, t, c, d, o);
	}
}

int sepol_allowxperm(char *s, char *t, char *c, char *range) {
	// printf("allowxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(s, t, c, range, AVTAB_XPERMS_ALLOWED, 0);
}

int sepol_auditallowxperm(char *s, char *t, char *c, char *range) {
	// printf("auditallowxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(s, t, c, range, AVTAB_XPERMS_AUDITALLOW, 0);
}

int sepol_dontauditxperm(char *s, char *t, char *c, char *range) {
	// printf("dontauditxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(s, t, c, range, AVTAB_XPERMS_DONTAUDIT, 0);
}

int sepol_permissive(char *s) {
	// printf("permissive %s\n", s);
	return set_domain_state(s, 1);
}

int sepol_enforce(char *s) {
	// printf("enforce %s\n", s);
	return set_domain_state(s, 0);
}

int sepol_create(char *s) {
	// printf("create %s\n", s);
	return create_domain(s);
}

int sepol_attradd(char *s, char *a) {
	// printf("attradd %s %s\n", s, a);
	return add_typeattribute(s, a);
}

int sepol_exists(char* source) {
	return !! hashtab_search(policydb->p_types.table, source);
}
