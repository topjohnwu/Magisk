#include <base.hpp>
#include "flags.h"
#include "policy.hpp"

#if MAGISK_DEBUG
template<typename Arg>
auto as_str(Arg arg) {
    if constexpr (std::is_same_v<Arg, const char *> || std::is_same_v<Arg, char *>) {
        return arg == nullptr ? std::string("*") : std::string(arg);
    } else {
        return std::to_string(arg);
    }
}

// Print out all rules going through public API for debugging
template<typename ...Args>
static void dprint(const char *action, Args ...args) {
    std::string s;
    s = (... + (" " + as_str(args)));
    LOGD("%s%s", action, s.data());
}
#else
#define dprint(...) ((void) 0)
#endif

bool sepolicy::exists(const char *type) {
    return hashtab_search(impl->db->p_types.table, type) != nullptr;
}

void sepolicy::load_rule_file(const char *file) {
    rust::load_rule_file(*this, file);
}

void sepolicy::parse_statement(const char *data) {
    rust::parse_statement(*this, data);
}

void sepolicy::magisk_rules() {
    rust::magisk_rules(*this);
}

void sepolicy::load_rules(const std::string &rules) {
    rust::load_rules(*this, byte_view(rules, false));
}

template<typename F, typename ...T>
requires(std::invocable<F, T...>)
inline void expand(F &&f, T &&...args) {
    f(std::forward<T>(args)...);
}

template<typename ...T>
inline void expand(const rust::Vec<rust::Str> &vec, T &&...args) {
    for (auto i = vec.begin(); i != vec.end() || vec.empty(); ++i) {
        expand(std::forward<T>(args)..., vec.empty() ? nullptr : std::string(*i).data());
        if (vec.empty()) break;
    }
}

template<typename ...T>
inline void expand(const rust::Vec<Xperm> &vec, T &&...args) {
    for (auto &p : vec) {
        expand(std::forward<T>(args)..., p.low, p.high, p.reset);
    }
}

template<typename ...T>
inline void expand(const rust::Str &s, T &&...args) {
    expand(std::forward<T>(args)..., std::string(s).data());
}

void
sepolicy::allow(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls,
                rust::Vec<rust::Str> perm) {
    expand(src, tgt, cls, perm, [this](auto ...args) {
        dprint("allow", args...);
        impl->add_rule(args..., AVTAB_ALLOWED, false);
    });
}

void
sepolicy::deny(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls,
               rust::Vec<rust::Str> perm) {
    expand(src, tgt, cls, perm, [this](auto ...args) {
        dprint("deny", args...);
        impl->add_rule(args..., AVTAB_ALLOWED, true);
    });
}

void sepolicy::auditallow(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt,
                          rust::Vec<rust::Str> cls,
                          rust::Vec<rust::Str> perm) {
    expand(src, tgt, cls, perm, [this](auto ...args) {
        dprint("auditallow", args...);
        impl->add_rule(args..., AVTAB_AUDITALLOW, false);
    });
}

void sepolicy::dontaudit(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt,
                         rust::Vec<rust::Str> cls,
                         rust::Vec<rust::Str> perm) {
    expand(src, tgt, cls, perm, [this](auto ...args) {
        dprint("dontaudit", args...);
        impl->add_rule(args..., AVTAB_AUDITDENY, true);
    });
}

void sepolicy::permissive(rust::Vec<rust::Str> types) {
    expand(types, [this](auto ...args) {
        dprint("permissive", args...);
        impl->set_type_state(args..., true);
    });
}

void sepolicy::enforce(rust::Vec<rust::Str> types) {
    expand(types, [this](auto ...args) {
        dprint("enforce", args...);
        impl->set_type_state(args..., false);
    });
}

void sepolicy::typeattribute(rust::Vec<rust::Str> types, rust::Vec<rust::Str> attrs) {
    expand(types, attrs, [this](auto ...args) {
        dprint("typeattribute", args...);
        impl->add_typeattribute(args...);
    });
}

void sepolicy::type(rust::Str type, rust::Vec<rust::Str> attrs) {
    expand(type, attrs, [this](auto name, auto attr) {
        dprint("type", name, attr);
        impl->add_type(name, TYPE_TYPE) && impl->add_typeattribute(name, attr);
    });
}

void sepolicy::attribute(rust::Str name) {
    expand(name, [this](auto ...args) {
        dprint("name", args...);
        impl->add_type(args..., TYPE_ATTRIB);
    });
}

void sepolicy::type_transition(rust::Str src, rust::Str tgt, rust::Str cls, rust::Str def,
                               rust::Vec<rust::Str> obj) {
    auto obj_str = obj.empty() ? std::string() : std::string(obj[0]);
    auto o = obj.empty() ? nullptr : obj_str.data();
    expand(src, tgt, cls, def, [this, &o](auto ...args) {
        dprint("type_transition", args..., o);
        if (o) {
            impl->add_filename_trans(args..., o);
        } else {
            impl->add_type_rule(args..., AVTAB_TRANSITION);
        }
    });
}

void sepolicy::type_change(rust::Str src, rust::Str tgt, rust::Str cls, rust::Str def) {
    expand(src, tgt, cls, def, [this](auto ...args) {
        dprint("type_change", args...);
        impl->add_type_rule(args..., AVTAB_CHANGE);
    });
}

void sepolicy::type_member(rust::Str src, rust::Str tgt, rust::Str cls, rust::Str def) {
    expand(src, tgt, cls, def, [this](auto ...args) {
        dprint("type_member", args...);
        impl->add_type_rule(args..., AVTAB_MEMBER);
    });
}

void sepolicy::genfscon(rust::Str fs_name, rust::Str path, rust::Str ctx) {
    expand(fs_name, path, ctx, [this](auto ...args) {
        dprint("genfscon", args...);
        impl->add_genfscon(args...);
    });
}

void
sepolicy::allowxperm(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt, rust::Vec<rust::Str> cls,
                     rust::Vec<Xperm> xperm) {
    expand(src, tgt, cls, xperm, [this](auto ...args) {
        dprint("allowxperm", args...);
        impl->add_xperm_rule(args..., AVTAB_XPERMS_ALLOWED);
    });
}

void sepolicy::auditallowxperm(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt,
                               rust::Vec<rust::Str> cls,
                               rust::Vec<Xperm> xperm) {
    expand(src, tgt, cls, xperm, [this](auto ...args) {
        dprint("auditallowxperm", args...);
        impl->add_xperm_rule(args..., AVTAB_XPERMS_AUDITALLOW);
    });
}

void sepolicy::dontauditxperm(rust::Vec<rust::Str> src, rust::Vec<rust::Str> tgt,
                              rust::Vec<rust::Str> cls,
                              rust::Vec<Xperm> xperm) {
    expand(src, tgt, cls, xperm, [this](auto ...args) {
        dprint("dontauditxperm", args...);
        impl->add_xperm_rule(args..., AVTAB_XPERMS_DONTAUDIT);
    });
}

void sepolicy::strip_dontaudit() {
    impl->strip_dontaudit();
}
