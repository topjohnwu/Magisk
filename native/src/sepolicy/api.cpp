#include <base.hpp>

#include "include/sepolicy.hpp"

using Str = rust::Str;
using StrVec = rust::Vec<rust::Str>;
using Xperms = rust::Vec<Xperm>;

#if 0
template<typename Arg>
std::string as_str(const Arg &arg) {
    if constexpr (std::is_same_v<Arg, const char *> || std::is_same_v<Arg, char *>) {
        return arg == nullptr ? "*" : arg;
    } else if constexpr (std::is_same_v<Arg, Xperm>) {
        return std::string(rust::xperm_to_string(arg));
    } else {
        return std::to_string(arg);
    }
}

// Print out all rules going through public API for debugging
template<typename ...Args>
static void print_rule(const char *action, Args ...args) {
    std::string s;
    s = (... + (" " + as_str(args)));
    LOGD("%s%s\n", action, s.data());
}
#else
#define print_rule(...) ((void) 0)
#endif

template<typename F, typename ...T>
requires(std::invocable<F, T...>)
static inline void expand(F &&f, T &&...args) {
    f(std::forward<T>(args)...);
}

template<typename ...T>
static inline void expand(const Str &s, T &&...args) {
    if (s.empty()) {
        expand(std::forward<T>(args)..., (char *) nullptr);
    } else {
        expand(std::forward<T>(args)..., std::string(s).data());
    }
}

template<typename ...T>
static inline void expand(const StrVec &vec, T &&...args) {
    if (vec.empty()) {
        expand(std::forward<T>(args)..., (char *) nullptr);
    } else {
        for (auto &s : vec) {
            expand(s, std::forward<T>(args)...);
        }
    }
}

template<typename ...T>
static inline void expand(const Xperms &vec, T &&...args) {
    for (auto &p : vec) {
        expand(std::forward<T>(args)..., p);
    }
}

void sepolicy::allow(StrVec src, StrVec tgt, StrVec cls, StrVec perm) noexcept {
    expand(src, tgt, cls, perm, [this](auto ...args) {
        print_rule("allow", args...);
        impl->add_rule(args..., AVTAB_ALLOWED, false);
    });
}

void sepolicy::deny(StrVec src, StrVec tgt, StrVec cls, StrVec perm) noexcept {
    expand(src, tgt, cls, perm, [this](auto ...args) {
        print_rule("deny", args...);
        impl->add_rule(args..., AVTAB_ALLOWED, true);
    });
}

void sepolicy::auditallow(StrVec src, StrVec tgt, StrVec cls, StrVec perm) noexcept {
    expand(src, tgt, cls, perm, [this](auto ...args) {
        print_rule("auditallow", args...);
        impl->add_rule(args..., AVTAB_AUDITALLOW, false);
    });
}

void sepolicy::dontaudit(StrVec src, StrVec tgt, StrVec cls, StrVec perm) noexcept {
    expand(src, tgt, cls, perm, [this](auto ...args) {
        print_rule("dontaudit", args...);
        impl->add_rule(args..., AVTAB_AUDITDENY, true);
    });
}

void sepolicy::permissive(StrVec types) noexcept {
    expand(types, [this](auto ...args) {
        print_rule("permissive", args...);
        impl->set_type_state(args..., true);
    });
}

void sepolicy::enforce(StrVec types) noexcept {
    expand(types, [this](auto ...args) {
        print_rule("enforce", args...);
        impl->set_type_state(args..., false);
    });
}

void sepolicy::typeattribute(StrVec types, StrVec attrs) noexcept {
    expand(types, attrs, [this](auto ...args) {
        print_rule("typeattribute", args...);
        impl->add_typeattribute(args...);
    });
}

void sepolicy::type(Str type, StrVec attrs) noexcept {
    expand(type, attrs, [this](auto name, auto attr) {
        print_rule("type", name, attr);
        impl->add_type(name, TYPE_TYPE) && impl->add_typeattribute(name, attr);
    });
}

void sepolicy::attribute(Str name) noexcept {
    expand(name, [this](auto ...args) {
        print_rule("attribute", args...);
        impl->add_type(args..., TYPE_ATTRIB);
    });
}

void sepolicy::type_transition(Str src, Str tgt, Str cls, Str def, Str obj) noexcept {
    expand(src, tgt, cls, def, obj, [this](auto s, auto t, auto c, auto d, auto o) {
        if (o) {
            print_rule("type_transition", s, t, c, d, o);
            impl->add_filename_trans(s, t, c, d, o);
        } else {
            print_rule("type_transition", s, t, c, d);
            impl->add_type_rule(s, t, c, d, AVTAB_TRANSITION);
        }
    });
}

void sepolicy::type_change(Str src, Str tgt, Str cls, Str def) noexcept {
    expand(src, tgt, cls, def, [this](auto ...args) {
        print_rule("type_change", args...);
        impl->add_type_rule(args..., AVTAB_CHANGE);
    });
}

void sepolicy::type_member(Str src, Str tgt, Str cls, Str def) noexcept {
    expand(src, tgt, cls, def, [this](auto ...args) {
        print_rule("type_member", args...);
        impl->add_type_rule(args..., AVTAB_MEMBER);
    });
}

void sepolicy::genfscon(Str fs_name, Str path, Str ctx) noexcept {
    expand(fs_name, path, ctx, [this](auto ...args) {
        print_rule("genfscon", args...);
        impl->add_genfscon(args...);
    });
}

void sepolicy::allowxperm(StrVec src, StrVec tgt, StrVec cls, Xperms xperm) noexcept {
    expand(src, tgt, cls, xperm, [this](auto ...args) {
        print_rule("allowxperm", args...);
        impl->add_xperm_rule(args..., AVTAB_XPERMS_ALLOWED);
    });
}

void sepolicy::auditallowxperm(StrVec src, StrVec tgt, StrVec cls, Xperms xperm) noexcept {
    expand(src, tgt, cls, xperm, [this](auto ...args) {
        print_rule("auditallowxperm", args...);
        impl->add_xperm_rule(args..., AVTAB_XPERMS_AUDITALLOW);
    });
}

void sepolicy::dontauditxperm(StrVec src, StrVec tgt, StrVec cls, Xperms xperm) noexcept {
    expand(src, tgt, cls, xperm, [this](auto ...args) {
        print_rule("dontauditxperm", args...);
        impl->add_xperm_rule(args..., AVTAB_XPERMS_DONTAUDIT);
    });
}
