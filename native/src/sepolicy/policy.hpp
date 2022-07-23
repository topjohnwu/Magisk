#pragma once

// Internal APIs, do not use directly

#include <sepol/policydb/policydb.h>
#include <sepolicy.hpp>

struct sepol_impl : public sepolicy {
    avtab_ptr_t get_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms);
    bool add_rule(const char *s, const char *t, const char *c, const char *p, int effect, bool invert);
    void add_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, perm_datum_t *perm, int effect, bool invert);
    void add_xperm_rule(type_datum_t *src, type_datum_t *tgt,
            class_datum_t *cls, uint16_t low, uint16_t high, int effect, bool invert);
    bool add_xperm_rule(const char *s, const char *t, const char *c, const char *range, int effect, bool invert);
    bool add_type_rule(const char *s, const char *t, const char *c, const char *d, int effect);
    bool add_filename_trans(const char *s, const char *t, const char *c, const char *d, const char *o);
    bool add_genfscon(const char *fs_name, const char *path, const char *context);
    bool add_type(const char *type_name, uint32_t flavor);
    bool set_type_state(const char *type_name, bool permissive);
    void add_typeattribute(type_datum_t *type, type_datum_t *attr);
    bool add_typeattribute(const char *type, const char *attr);
    void strip_dontaudit();

    sepol_impl(policydb *db) : db(db) {}
    ~sepol_impl();

    policydb *db;
};

#define impl reinterpret_cast<sepol_impl *>(this)

void statement_help();
