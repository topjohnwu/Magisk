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

struct Xperm;

using StrVec = rust::Vec<rust::Str>;
using Xperms = rust::Vec<Xperm>;

struct sepolicy {
    using c_str = const char *;
    using Str = rust::Str;

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
    void type(Str type, StrVec attrs);
    void attribute(Str names);
    void permissive(StrVec types);
    void enforce(StrVec types);
    void typeattribute(StrVec types, StrVec attrs);
    bool exists(c_str type);

    // Access vector rules
    void allow(StrVec src, StrVec tgt, StrVec cls, StrVec perm);
    void deny(StrVec src, StrVec tgt, StrVec cls, StrVec perm);
    void auditallow(StrVec src, StrVec tgt, StrVec cls, StrVec perm);
    void dontaudit(StrVec src, StrVec tgt, StrVec cls, StrVec perm);

    // Extended permissions access vector rules
    void allowxperm(StrVec src, StrVec tgt, StrVec cls, Xperms xperm);
    void auditallowxperm(StrVec src, StrVec tgt, StrVec cls, Xperms xperm);
    void dontauditxperm(StrVec src, StrVec tgt, StrVec cls, Xperms xperm);

    // Type rules
    void type_transition(Str src, Str tgt, Str cls, Str def, Str obj);
    void type_change(Str src, Str tgt, Str cls, Str def);
    void type_member(Str src, Str tgt, Str cls, Str def);

    // File system labeling
    void genfscon(Str fs_name, Str path, Str ctx);

    // Magisk
    void magisk_rules();

    void strip_dontaudit();

protected:
    // Prevent anyone from accidentally creating an instance
    sepolicy() = default;
};
