#include "magiskpolicy.h"
#include "sepolicy.h"

int sepol_allow(char *s, char *t, char *c, char *p) {
	return add_rule(s, t, c, p, AVTAB_ALLOWED, 0);
}

int sepol_deny(char *s, char *t, char *c, char *p) {
	return add_rule(s, t, c, p, AVTAB_ALLOWED, 1);
}

int sepol_auditallow(char *s, char *t, char *c, char *p) {
	return add_rule(s, t, c, p, AVTAB_AUDITALLOW, 0);
}

int sepol_auditdeny(char *s, char *t, char *c, char *p) {
	return add_rule(s, t, c, p, AVTAB_AUDITDENY, 0);
}

int sepol_typetrans(char *s, char *t, char *c, char *d, char *o) {
	if (o == NULL)
		return add_transition(s, t, c, d);
	else
		return add_file_transition(s, t, c, d, o);
}

int sepol_permissive(char *s) {
	return set_domain_state(s, 1);
}

int sepol_enforce(char *s) {
	return set_domain_state(s, 0);
}

int sepol_create(char *s) {
	return create_domain(s);
}

int sepol_attradd(char *s, char *a) {
	return add_typeattribute(s, a);
}

int sepol_exists(char* source) {
	return !! hashtab_search(policy->p_types.table, source);
}
