#pragma once

#include <stdlib.h>
#include <string>

#include <base.hpp>

// sepolicy paths
#define PLAT_POLICY_DIR     "/system/etc/selinux/"
#define VEND_POLICY_DIR     "/vendor/etc/selinux/"
#define PROD_POLICY_DIR     "/product/etc/selinux/"
#define ODM_POLICY_DIR      "/odm/etc/selinux/"
#define SYSEXT_POLICY_DIR   "/system_ext/etc/selinux/"
#define SPLIT_PLAT_CIL      PLAT_POLICY_DIR "plat_sepolicy.cil"

// selinuxfs paths
#define SELINUX_MNT         "/sys/fs/selinux"
#define SELINUX_ENFORCE     SELINUX_MNT "/enforce"
#define SELINUX_POLICY      SELINUX_MNT "/policy"
#define SELINUX_LOAD        SELINUX_MNT "/load"
#define SELINUX_VERSION     SELINUX_MNT "/policyvers"

using token_list = std::vector<const char *>;
using argument = std::pair<token_list, bool>;
using argument_list = std::vector<argument>;

struct Xperm;

#define ALL       nullptr

struct sepolicy {
    using c_str = const char *;

    // Public static factory functions
    static sepolicy *from_data(char *data, size_t len);
    static sepolicy *from_file(c_str file);
    static sepolicy *from_split();
    static sepolicy *compile_split();
    // External APIs
    bool to_file(c_str file);
    void load_rules(const std::string &rules);
    void load_rule_file(c_str file);
    void print_rules();
    void parse_statement(c_str statement);

    // Operation on types
    void type(rust::Str type, rust::Vec<rust::Str> attrs);
    void attribute(rust::Str names);
    void permissive(rust::Vec<rust::Str> types);
    void enforce(rust::Vec<rust::Str> types);
    void typeattribute(rust::Vec<rust::Str> types, rust::Vec<rust::Str> attrs);
    bool exists(c_str type);

    // Access vector rules
    void allow(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls, rust::Vec<rust::Str> perm);
    void deny(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls, rust::Vec<rust::Str> perm);
    void auditallow(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls, rust::Vec<rust::Str> perm);
    void dontaudit(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls, rust::Vec<rust::Str> perm);

    // Extended permissions access vector rules
    void allowxperm(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls, rust::Vec<Xperm> xperm);
    void auditallowxperm(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls, rust::Vec<Xperm> xperm);
    void dontauditxperm(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls, rust::Vec<Xperm> xperm);

    // Type rules
    void type_transition(rust::Str src, rust::Str tgt, rust::Str cls, rust::Str def, rust::Vec<rust::Str> obj);
    void type_change(rust::Str src, rust::Str tgt, rust::Str cls, rust::Str def);
    void type_member(rust::Str src, rust::Str tgt, rust::Str cls, rust::Str def);

    // File system labeling
    void genfscon(rust::Str fs_name, rust::Str path, rust::Str ctx);

    // Magisk
    void magisk_rules();

    void strip_dontaudit();

protected:
    // Prevent anyone from accidentally creating an instance
    sepolicy() = default;
};
