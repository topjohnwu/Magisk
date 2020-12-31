#pragma once

#include <stdlib.h>
#include <selinux.hpp>

#define ALL nullptr

struct policydb;

class sepolicy {
public:
    typedef const char * c_str;
    ~sepolicy();

    // Public static factory functions
    static sepolicy *from_file(c_str file);
    static sepolicy *from_split();
    static sepolicy *compile_split();

    // External APIs
    bool to_file(c_str file);
    void parse_statement(c_str stmt);
    void load_rule_file(c_str file);

    // Operation on types
    bool type(c_str name, c_str attr);
    bool attribute(c_str name);
    bool permissive(c_str type);
    bool enforce(c_str type);
    bool typeattribute(c_str type, c_str attr);
    bool exists(c_str type);

    // Access vector rules
    bool allow(c_str src, c_str tgt, c_str cls, c_str perm);
    bool deny(c_str src, c_str tgt, c_str cls, c_str perm);
    bool auditallow(c_str src, c_str tgt, c_str cls, c_str perm);
    bool dontaudit(c_str src, c_str tgt, c_str cls, c_str perm);

    // Extended permissions access vector rules
    bool allowxperm(c_str src, c_str tgt, c_str cls, c_str range);
    bool auditallowxperm(c_str src, c_str tgt, c_str cls, c_str range);
    bool dontauditxperm(c_str src, c_str tgt, c_str cls, c_str range);

    // Type rules
    bool type_transition(c_str src, c_str tgt, c_str cls, c_str def, c_str obj = nullptr);
    bool type_change(c_str src, c_str tgt, c_str cls, c_str def);
    bool type_member(c_str src, c_str tgt, c_str cls, c_str def);

    // File system labeling
    bool genfscon(c_str fs_name, c_str path, c_str ctx);

    // Magisk
    void magisk_rules();

    // Deprecate
    bool create(c_str name) { return type(name, "domain"); }

protected:
    policydb *db;
};
