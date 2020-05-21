#include <magiskpolicy.hpp>

#include "sepolicy.h"

//#define vprint(fmt, ...) printf(fmt, __VA_ARGS__)
#define vprint(...)

int sepolicy::allow(const char *s, const char *t, const char *c, const char *p) {
	vprint("allow %s %s %s %s\n", s, t, c, p);
	return add_rule(db, s, t, c, p, AVTAB_ALLOWED, 0);
}

int sepolicy::deny(const char *s, const char *t, const char *c, const char *p) {
	vprint("deny %s %s %s %s\n", s, t, c, p);
	return add_rule(db, s, t, c, p, AVTAB_ALLOWED, 1);
}

int sepolicy::auditallow(const char *s, const char *t, const char *c, const char *p) {
	vprint("auditallow %s %s %s %s\n", s, t, c, p);
	return add_rule(db, s, t, c, p, AVTAB_AUDITALLOW, 0);
}

int sepolicy::dontaudit(const char *s, const char *t, const char *c, const char *p) {
	vprint("dontaudit %s %s %s %s\n", s, t, c, p);
	return add_rule(db, s, t, c, p, AVTAB_AUDITDENY, 1);
}

int sepolicy::allowxperm(const char *s, const char *t, const char *c, const char *range) {
	vprint("allowxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(db, s, t, c, range, AVTAB_XPERMS_ALLOWED, 0);
}

int sepolicy::auditallowxperm(const char *s, const char *t, const char *c, const char *range) {
	vprint("auditallowxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(db, s, t, c, range, AVTAB_XPERMS_AUDITALLOW, 0);
}

int sepolicy::dontauditxperm(const char *s, const char *t, const char *c, const char *range) {
	vprint("dontauditxperm %s %s %s %s\n", s, t, c, range);
	return add_xperm_rule(db, s, t, c, range, AVTAB_XPERMS_DONTAUDIT, 0);
}

int sepolicy::type_change(const char *s, const char *t, const char *c, const char *d) {
	vprint("type_change %s %s %s %s\n", s, t, c, d);
	return add_type_rule(db, s, t, c, d, AVTAB_CHANGE);
}

int sepolicy::type_member(const char *s, const char *t, const char *c, const char *d) {
	vprint("type_member %s %s %s %s\n", s, t, c, d);
	return add_type_rule(db, s, t, c, d, AVTAB_MEMBER);
}

int sepolicy::type_transition(const char *src, const char *tgt, const char *cls, const char *def, const char *obj) {
	if (obj) {
		vprint("type_transition %s %s %s %s\n", src, tgt, cls, def);
		return add_type_rule(db, src, tgt, cls, def, AVTAB_TRANSITION);
	} else {
		vprint("type_transition %s %s %s %s %s\n", src, tgt, cls, def, obj);
		return add_filename_trans(db, src, tgt, cls, def, obj);
	}
}

int sepolicy::permissive(const char *s) {
	vprint("permissive %s\n", s);
	return set_domain_state(db, s, 1);
}

int sepolicy::enforce(const char *s) {
	vprint("enforce %s\n", s);
	return set_domain_state(db, s, 0);
}

int sepolicy::create(const char *s) {
	vprint("create %s\n", s);
	return create_domain(db, s);
}

int sepolicy::typeattribute(const char *type, const char *attr) {
	vprint("typeattribute %s %s\n", type, attr);
	return add_typeattribute(db, type, attr);
}

int sepolicy::genfscon(const char *fs_name, const char *path, const char *ctx) {
	vprint("genfscon %s %s %s\n", fs_name, path, ctx);
	return add_genfscon(db, fs_name, path, ctx);
}

int sepolicy::exists(const char *source) {
	return hashtab_search(db->p_types.table, source) != nullptr;
}
