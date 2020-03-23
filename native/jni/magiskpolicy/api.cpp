#include <magiskpolicy.hpp>

#include "sepolicy.h"

//#define vprint(fmt, ...) printf(fmt, __VA_ARGS__)
#define vprint(...)

int sepol_allow(const char *s, const char *t, const char *c, const char *p) {
	vprint("allow %s %s %s %s\n", s, t, c, p);
	return add_rule(s, t, c, p, AVTAB_ALLOWED, 0);
}

int sepol_deny(const char *s, const char *t, const char *c, const char *p) {
	vprint("deny %s %s %s %s\n", s, t, c, p);
	return add_rule(s, t, c, p, AVTAB_ALLOWED, 1);
}

int sepol_auditallow(const char *s, const char *t, const char *c, const char *p) {
	vprint("auditallow %s %s %s %s\n", s, t, c, p);
	return add_rule(s, t, c, p, AVTAB_AUDITALLOW, 0);
}

int sepol_dontaudit(const char *s, const char *t, const char *c, const char *p) {
	vprint("dontaudit %s %s %s %s\n", s, t, c, p);
	return add_rule(s, t, c, p, AVTAB_AUDITDENY, 1);
}

int sepol_allowxperm(const char *s, const char *t, const char *c, const char *range) {
	vprint("allowxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(s, t, c, range, AVTAB_XPERMS_ALLOWED, 0);
}

int sepol_auditallowxperm(const char *s, const char *t, const char *c, const char *range) {
	vprint("auditallowxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(s, t, c, range, AVTAB_XPERMS_AUDITALLOW, 0);
}

int sepol_dontauditxperm(const char *s, const char *t, const char *c, const char *range) {
	vprint("dontauditxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(s, t, c, range, AVTAB_XPERMS_DONTAUDIT, 0);
}

int sepol_typetrans(const char *s, const char *t, const char *c, const char *d) {
	vprint("type_transition %s %s %s %s\n", s, t, c, d);
	return add_type_rule(s, t, c, d, AVTAB_TRANSITION);
}

int sepol_typechange(const char *s, const char *t, const char *c, const char *d) {
	vprint("type_change %s %s %s %s\n", s, t, c, d);
	return add_type_rule(s, t, c, d, AVTAB_CHANGE);
}

int sepol_typemember(const char *s, const char *t, const char *c, const char *d) {
	vprint("type_member %s %s %s %s\n", s, t, c, d);
	return add_type_rule(s, t, c, d, AVTAB_MEMBER);
}

int sepol_nametrans(const char *s, const char *t, const char *c, const char *d, const char *o) {
	vprint("name_trans %s %s %s %s %s\n", s, t, c, d, o);
	return add_filename_trans(s, t, c, d, o);
}

int sepol_permissive(const char *s) {
	vprint("permissive %s\n", s);
	return set_domain_state(s, 1);
}

int sepol_enforce(const char *s) {
	vprint("enforce %s\n", s);
	return set_domain_state(s, 0);
}

int sepol_create(const char *s) {
	vprint("create %s\n", s);
	return create_domain(s);
}

int sepol_attradd(const char *s, const char *a) {
	vprint("attradd %s %s\n", s, a);
	return add_typeattribute(s, a);
}

int sepol_genfscon(const char *name, const char *path, const char *context) {
	vprint("genfscon %s %s %s\n", name, path, context);
	return add_genfscon(name, path, context);
}

int sepol_exists(const char *source) {
	return hashtab_search(magisk_policydb->p_types.table, source) != nullptr;
}
