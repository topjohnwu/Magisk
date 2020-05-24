#pragma once

#include <sepol/policydb/policydb.h>
#include <magiskpolicy.hpp>

// Internal APIs, do not use directly
struct sepol_impl : public sepolicy {
	int set_attr(const char *attr_name, int type_val);
	void check_avtab_node(avtab_ptr_t node);
	avtab_ptr_t get_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms);
	bool add_rule(const char *s, const char *t, const char *c, const char *p, int effect, bool n);
	void add_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, perm_datum_t *perm, int effect, bool n);
	void add_xperm_rule(type_datum_t *src, type_datum_t *tgt,
			class_datum_t *cls, uint16_t low, uint16_t high, int effect, bool n);
	bool add_xperm_rule(const char *s, const char *t, const char *c, const char *range, int effect, bool n);
	bool add_type_rule(const char *s, const char *t, const char *c, const char *d, int effect);
	bool add_filename_trans(const char *s, const char *t, const char *c, const char *d, const char *o);
	bool add_genfscon(const char *fs_name, const char *path, const char *context);
	bool create_domain(const char *type_name);
	bool set_domain_state(const char *s, bool permissive);
	bool add_typeattribute(const char *type, const char *attr);
	void strip_dontaudit();
	void allow_su_client(const char *type);
};

#define impl static_cast<sepol_impl *>(this)

void statement_help();
