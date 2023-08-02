#pragma once

// Internal APIs, do not use directly

#include <map>
#include <string_view>

#include <sepol/policydb/policydb.h>
#include <sepolicy.hpp>

#include "policy-rs.hpp"

struct sepol_impl : public sepolicy {
    avtab_ptr_t find_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms);
    avtab_ptr_t insert_avtab_node(avtab_key_t *key);
    avtab_ptr_t get_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms);
    void print_type(FILE *fp, type_datum_t *type);
    void print_avtab(FILE *fp, avtab_ptr_t node);
    void print_filename_trans(FILE *fp, hashtab_ptr_t node);

    bool add_rule(const char *s, const char *t, const char *c, const char *p, int effect, bool invert);
    void add_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, perm_datum_t *perm, int effect, bool invert);
    void add_xperm_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, const argument &xperm, int effect);
    bool add_xperm_rule(const char *s, const char *t, const char *c, const argument &xperm, int effect);
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

private:
    std::map<std::string_view, std::array<const char *, 32>> class_perm_names;
};

#define impl reinterpret_cast<sepol_impl *>(this)

const char *as_str(const argument &arg);
const char *as_str(const char *arg);

void statement_help();
void test_parse_statements();
