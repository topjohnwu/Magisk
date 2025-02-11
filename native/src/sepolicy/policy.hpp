#pragma once

// Internal APIs, do not use directly

#include <map>
#include <string_view>
#include <cxx.h>

#include <sepol/policydb/policydb.h>

using Str = rust::Str;

struct Xperm;

class sepol_impl {
    avtab_ptr_t find_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms);
    avtab_ptr_t insert_avtab_node(avtab_key_t *key);
    avtab_ptr_t get_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms);
    void print_type(FILE *fp, type_datum_t *type);
    void print_avtab(FILE *fp, avtab_ptr_t node);
    void print_filename_trans(FILE *fp, hashtab_ptr_t node);

    bool add_rule(Str s, Str t, Str c, Str p, int effect, bool invert);
    void add_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, perm_datum_t *perm, int effect, bool invert);
    void add_xperm_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, const Xperm &p, int effect);
    bool add_xperm_rule(Str s, Str t, Str c, const Xperm &p, int effect);
    bool add_type_rule(Str s, Str t, Str c, Str d, int effect);
    bool add_filename_trans(Str s, Str t, Str c, Str d, Str o);
    bool add_genfscon(Str fs_name, Str path, Str context);
    bool add_type(Str type_name, uint32_t flavor);
    bool set_type_state(Str type_name, bool permissive);
    void add_typeattribute(type_datum_t *type, type_datum_t *attr);
    bool add_typeattribute(Str type, Str attr);

    policydb *db;

    std::map<std::string_view, std::array<const char *, 32>> class_perm_names;

    friend struct SePolicy;

public:
    sepol_impl(policydb *db) : db(db) {}
    ~sepol_impl();
};
