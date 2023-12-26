#include <base.hpp>

#include "policy.hpp"

#if 0
// Print out all rules going through public API for debugging
template <typename ...Args>
static void dprint(const char *action, Args ...args) {
    std::string s(action);
    for (int i = 0; i < sizeof...(args); ++i) s += " %s";
    s += "\n";
    LOGD(s.data(), as_str(args)...);
}
#else
#define dprint(...)
#endif

bool sepolicy::allow(const char *s, const char *t, const char *c, const char *p) {
    dprint(__FUNCTION__, s, t, c, p);
    return impl->add_rule(s, t, c, p, AVTAB_ALLOWED, false);
}

bool sepolicy::deny(const char *s, const char *t, const char *c, const char *p) {
    dprint(__FUNCTION__, s, t, c, p);
    return impl->add_rule(s, t, c, p, AVTAB_ALLOWED, true);
}

bool sepolicy::auditallow(const char *s, const char *t, const char *c, const char *p) {
    dprint(__FUNCTION__, s, t, c, p);
    return impl->add_rule(s, t, c, p, AVTAB_AUDITALLOW, false);
}

bool sepolicy::dontaudit(const char *s, const char *t, const char *c, const char *p) {
    dprint(__FUNCTION__, s, t, c, p);
    return impl->add_rule(s, t, c, p, AVTAB_AUDITDENY, true);
}

bool sepolicy::allowxperm(const char *s, const char *t, const char *c, const argument &xperm) {
    dprint(__FUNCTION__, s, t, c, "ioctl", xperm);
    return impl->add_xperm_rule(s, t, c, xperm, AVTAB_XPERMS_ALLOWED);
}

bool sepolicy::auditallowxperm(const char *s, const char *t, const char *c, const argument &xperm) {
    dprint(__FUNCTION__, s, t, c, "ioctl", xperm);
    return impl->add_xperm_rule(s, t, c, xperm, AVTAB_XPERMS_AUDITALLOW);
}

bool sepolicy::dontauditxperm(const char *s, const char *t, const char *c, const argument &xperm) {
    dprint(__FUNCTION__, s, t, c, "ioctl", xperm);
    return impl->add_xperm_rule(s, t, c, xperm, AVTAB_XPERMS_DONTAUDIT);
}

bool sepolicy::type_change(const char *s, const char *t, const char *c, const char *d) {
    dprint(__FUNCTION__, s, t, c, d);
    return impl->add_type_rule(s, t, c, d, AVTAB_CHANGE);
}

bool sepolicy::type_member(const char *s, const char *t, const char *c, const char *d) {
    dprint(__FUNCTION__, s, t, c, d);
    return impl->add_type_rule(s, t, c, d, AVTAB_MEMBER);
}

bool sepolicy::type_transition(const char *s, const char *t, const char *c, const char *d, const char *o) {
    if (o) {
        dprint(__FUNCTION__, s, t, c, d, o);
        return impl->add_filename_trans(s, t, c, d, o);
    } else {
        dprint(__FUNCTION__, s, t, c, d);
        return impl->add_type_rule(s, t, c, d, AVTAB_TRANSITION);
    }
}

bool sepolicy::permissive(const char *s) {
    dprint(__FUNCTION__, s);
    return impl->set_type_state(s, true);
}

bool sepolicy::enforce(const char *s) {
    dprint(__FUNCTION__, s);
    return impl->set_type_state(s, false);
}

bool sepolicy::type(const char *name, const char *attr) {
    dprint(__FUNCTION__, name, attr);
    return impl->add_type(name, TYPE_TYPE) && impl->add_typeattribute(name, attr);
}

bool sepolicy::attribute(const char *name) {
    dprint(__FUNCTION__, name);
    return impl->add_type(name, TYPE_ATTRIB);
}

bool sepolicy::typeattribute(const char *type, const char *attr) {
    dprint(__FUNCTION__, type, attr);
    return impl->add_typeattribute(type, attr);
}

bool sepolicy::genfscon(const char *fs_name, const char *path, const char *ctx) {
    dprint(__FUNCTION__, fs_name, path, ctx);
    return impl->add_genfscon(fs_name, path, ctx);
}

bool sepolicy::exists(const char *type) {
    return hashtab_search(impl->db->p_types.table, type) != nullptr;
}

void sepolicy::load_rule_file(const char *file) {
    rust::load_rule_file(*this, file);
}

void sepolicy::load_rules(const std::string &rules) {
    rust::load_rules(*this, byte_view(rules, false));
}
