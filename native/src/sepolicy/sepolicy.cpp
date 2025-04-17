#include <base.hpp>

#include "include/sepolicy.hpp"

using namespace std;

// Invert is adding rules for auditdeny; in other cases, invert is removing rules
#define strip_av(effect, invert) ((effect == AVTAB_AUDITDENY) == !invert)

// libsepol internal APIs
__BEGIN_DECLS
int policydb_index_decls(sepol_handle_t * handle, policydb_t * p);
int avtab_hash(struct avtab_key *keyp, uint32_t mask);
int type_set_expand(type_set_t * set, ebitmap_t * t, policydb_t * p, unsigned char alwaysexpand);
int context_from_string(
        sepol_handle_t * handle,
        const policydb_t * policydb,
        context_struct_t ** cptr,
        const char *con_str, size_t con_str_len);
int context_to_string(
        sepol_handle_t * handle,
        const policydb_t * policydb,
        const context_struct_t * context,
        char **result, size_t * result_len);
__END_DECLS

template <typename T>
struct auto_cast_wrapper {
    auto_cast_wrapper(T *ptr) : ptr(ptr) {}
    template <typename U>
    operator U*() const { return static_cast<U*>(ptr); }
private:
    T *ptr;
};

template <typename T>
static auto_cast_wrapper<T> auto_cast(T *p) {
    return auto_cast_wrapper<T>(p);
}

template <size_t T>
static size_t copy_str(std::array<char, T> &dest, rust::Str src) {
    if (T == 0) return 0;
    size_t len = std::min(T - 1, src.size());
    memcpy(dest.data(), src.data(), len);
    dest[len] = '\0';
    return len;
}

static char *dup_str(rust::Str src) {
    size_t len = src.size();
    char *s = static_cast<char *>(malloc(len + 1));
    memcpy(s, src.data(), len);
    s[len] = '\0';
    return s;
}

static bool str_eq(string_view a, rust::Str b) {
    return a.size() == b.size() && memcmp(a.data(), b.data(), a.size()) == 0;
}

static auto hashtab_find(hashtab_t h, Str key) {
    array<char, 256> buf{};
    copy_str(buf, key);
    return auto_cast(hashtab_search(h, buf.data()));
}

template <class Node, class Func>
static void list_for_each(Node *node_ptr, const Func &fn) {
    auto cur = node_ptr;
    while (cur) {
        auto next = cur->next;
        fn(cur);
        cur = next;
    }
}

template <class Node, class Func>
static Node *list_find(Node *node_ptr, const Func &fn) {
    for (auto cur = node_ptr; cur; cur = cur->next) {
        if (fn(cur)) {
            return cur;
        }
    }
    return nullptr;
}

template <class Node, class Func>
static void hash_for_each(Node **node_ptr, int n_slot, const Func &fn) {
    for (int i = 0; i < n_slot; ++i) {
        list_for_each(node_ptr[i], fn);
    }
}

template <class Func>
static void hashtab_for_each(hashtab_t htab, const Func &fn) {
    hash_for_each(htab->htable, htab->size, fn);
}

template <class Func>
static void avtab_for_each(avtab_t *avtab, const Func &fn) {
    hash_for_each(avtab->htable, avtab->nslot, fn);
}

template <class Func>
static void for_each_attr(hashtab_t htab, const Func &fn) {
    hashtab_for_each(htab, [&](hashtab_ptr_t node) {
        auto type = static_cast<type_datum_t *>(node->datum);
        if (type->flavor == TYPE_ATTRIB)
            fn(type);
    });
}

static int avtab_remove_node(avtab_t *h, avtab_ptr_t node) {
    if (!h || !h->htable)
        return SEPOL_ENOMEM;
    int hvalue = avtab_hash(&node->key, h->mask);
    avtab_ptr_t prev = nullptr;
    avtab_ptr_t cur = h->htable[hvalue];
    while (cur) {
        if (cur == node)
            break;
        prev = cur;
        cur = cur->next;
    }
    if (cur == nullptr)
        return SEPOL_ENOENT;

    // Detach from link list
    if (prev)
        prev->next = node->next;
    else
        h->htable[hvalue] = node->next;
    h->nel--;

    // Free memory
    free(node->datum.xperms);
    free(node);
    return 0;
}

static bool is_redundant(avtab_ptr_t node) {
    switch (node->key.specified) {
    case AVTAB_AUDITDENY:
        return node->datum.data == ~0U;
    case AVTAB_XPERMS:
        return node->datum.xperms == nullptr;
    default:
        return node->datum.data == 0U;
    }
}

avtab_ptr_t sepol_impl::find_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms) {
    avtab_ptr_t node;

    // AVTAB_XPERMS entries are not necessarily unique
    if (key->specified & AVTAB_XPERMS) {
        if (xperms == nullptr)
            return nullptr;
        node = avtab_search_node(&db->te_avtab, key);
        while (node) {
            if ((node->datum.xperms->specified == xperms->specified) &&
                (node->datum.xperms->driver == xperms->driver)) {
                node = nullptr;
                break;
            }
            node = avtab_search_node_next(node, key->specified);
        }
    } else {
        node = avtab_search_node(&db->te_avtab, key);
    }

    return node;
}

avtab_ptr_t sepol_impl::insert_avtab_node(avtab_key_t *key) {
    avtab_datum_t avdatum{};
    // AUDITDENY, aka DONTAUDIT, are &= assigned, versus |= for others.
    // Initialize the data accordingly.
    avdatum.data = key->specified == AVTAB_AUDITDENY ? ~0U : 0U;
    return avtab_insert_nonunique(&db->te_avtab, key, &avdatum);
}

avtab_ptr_t sepol_impl::get_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms) {
    avtab_ptr_t node = find_avtab_node(key, xperms);
    if (!node) {
        node = insert_avtab_node(key);
    }
    return node;
}

void sepol_impl::add_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, perm_datum_t *perm, int effect, bool invert) {
    if (src == nullptr) {
        if (strip_av(effect, invert)) {
            // Stripping av, have to go through all types for correct results
            hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
                add_rule(auto_cast(node->datum), tgt, cls, perm, effect, invert);
            });
        } else {
            // If we are not stripping av, go through all attributes instead of types for optimization
            for_each_attr(db->p_types.table, [&](type_datum_t *type) {
                add_rule(type, tgt, cls, perm, effect, invert);
            });
        }
    } else if (tgt == nullptr) {
        if (strip_av(effect, invert)) {
            hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
                add_rule(src, auto_cast(node->datum), cls, perm, effect, invert);
            });
        } else {
            for_each_attr(db->p_types.table, [&](type_datum_t *type) {
                add_rule(src, type, cls, perm, effect, invert);
            });
        }
    } else if (cls == nullptr) {
        hashtab_for_each(db->p_classes.table, [&](hashtab_ptr_t node) {
            add_rule(src, tgt, auto_cast(node->datum), perm, effect, invert);
        });
    } else {
        avtab_key_t key;
        key.source_type = src->s.value;
        key.target_type = tgt->s.value;
        key.target_class = cls->s.value;
        key.specified = effect;

        avtab_ptr_t node = get_avtab_node(&key, nullptr);
        if (invert) {
            if (perm)
                node->datum.data &= ~(1U << (perm->s.value - 1));
            else
                node->datum.data = 0U;
        } else {
            if (perm)
                node->datum.data |= 1U << (perm->s.value - 1);
            else
                node->datum.data = ~0U;
        }

        if (is_redundant(node))
            avtab_remove_node(&db->te_avtab, node);
    }
}

bool sepol_impl::add_rule(Str s, Str t, Str c, Str p, int effect, bool invert) {
    type_datum_t *src = nullptr, *tgt = nullptr;
    class_datum_t *cls = nullptr;
    perm_datum_t *perm = nullptr;

    if (!s.empty()) {
        src = hashtab_find(db->p_types.table, s);
        if (src == nullptr) {
            LOGW("source type %.*s does not exist\n", (int) s.size(), s.data());
            return false;
        }
    }

    if (!t.empty()) {
        tgt = hashtab_find(db->p_types.table, t);
        if (tgt == nullptr) {
            LOGW("target type %.*s does not exist\n", (int) t.size(), t.data());
            return false;
        }
    }

    if (!c.empty()) {
        cls = hashtab_find(db->p_classes.table, c);
        if (cls == nullptr) {
            LOGW("class %.*s does not exist\n", (int) c.size(), c.data());
            return false;
        }
    }

    if (!p.empty()) {
        if (c.empty()) {
            LOGW("No class is specified, cannot add perm [%.*s] \n", (int) p.size(), p.data());
            return false;
        }

        perm = hashtab_find(cls->permissions.table, p);
        if (perm == nullptr && cls->comdatum != nullptr) {
            perm = hashtab_find(cls->comdatum->permissions.table, p);
        }
        if (perm == nullptr) {
            LOGW("perm %.*s does not exist in class %.*s\n",
                 (int) p.size(), p.data(), (int) c.size(), c.data());
            return false;
        }
    }
    add_rule(src, tgt, cls, perm, effect, invert);
    return true;
}

#define ioctl_driver(x) (x>>8 & 0xFF)
#define ioctl_func(x) (x & 0xFF)

void sepol_impl::add_xperm_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, const Xperm &p, int effect) {
    if (db->policyvers < POLICYDB_VERSION_XPERMS_IOCTL) {
        LOGW("policy version %u does not support ioctl extended permissions rules\n", db->policyvers);
        return;
    }
    if (src == nullptr) {
        for_each_attr(db->p_types.table, [&](type_datum_t *type) {
            add_xperm_rule(type, tgt, cls, p, effect);
        });
    } else if (tgt == nullptr) {
        for_each_attr(db->p_types.table, [&](type_datum_t *type) {
            add_xperm_rule(src, type, cls, p, effect);
        });
    } else if (cls == nullptr) {
        hashtab_for_each(db->p_classes.table, [&](hashtab_ptr_t node) {
            add_xperm_rule(src, tgt, auto_cast(node->datum), p, effect);
        });
    } else {
        avtab_key_t key;
        key.source_type = src->s.value;
        key.target_type = tgt->s.value;
        key.target_class = cls->s.value;
        key.specified = effect;

        // Each key may contain 1 driver node and 256 function nodes
        avtab_ptr_t node_list[257] = { nullptr };
#define driver_node (node_list[256])

        // Find all rules with key
        for (avtab_ptr_t node = avtab_search_node(&db->te_avtab, &key); node;) {
            if (node->datum.xperms->specified == AVTAB_XPERMS_IOCTLDRIVER) {
                driver_node = node;
            } else if (node->datum.xperms->specified == AVTAB_XPERMS_IOCTLFUNCTION) {
                node_list[node->datum.xperms->driver] = node;
            }
            node = avtab_search_node_next(node, key.specified);
        }

        if (p.reset) {
            for (int i = 0; i <= 0xFF; ++i) {
                if (node_list[i]) {
                    avtab_remove_node(&db->te_avtab, node_list[i]);
                    node_list[i] = nullptr;
                }
            }
            if (driver_node) {
                memset(driver_node->datum.xperms->perms, 0, sizeof(avtab_extended_perms_t::perms));
            }
        }

        auto new_driver_node = [&]() -> avtab_ptr_t {
            auto node = insert_avtab_node(&key);
            node->datum.xperms = auto_cast(calloc(1, sizeof(avtab_extended_perms_t)));
            node->datum.xperms->specified = AVTAB_XPERMS_IOCTLDRIVER;
            node->datum.xperms->driver = 0;
            return node;
        };

        auto new_func_node = [&](uint8_t driver) -> avtab_ptr_t {
            auto node = insert_avtab_node(&key);
            node->datum.xperms = auto_cast(calloc(1, sizeof(avtab_extended_perms_t)));
            node->datum.xperms->specified = AVTAB_XPERMS_IOCTLFUNCTION;
            node->datum.xperms->driver = driver;
            return node;
        };

        if (!p.reset) {
            if (ioctl_driver(p.low) != ioctl_driver(p.high)) {
                if (driver_node == nullptr) {
                    driver_node = new_driver_node();
                }
                for (int i = ioctl_driver(p.low); i <= ioctl_driver(p.high); ++i) {
                    xperm_set(i, driver_node->datum.xperms->perms);
                }
            } else {
                uint8_t driver = ioctl_driver(p.low);
                auto node = node_list[driver];
                if (node == nullptr) {
                    node = new_func_node(driver);
                    node_list[driver] = node;
                }
                for (int i = ioctl_func(p.low); i <= ioctl_func(p.high); ++i) {
                    xperm_set(i, node->datum.xperms->perms);
                }
            }
        } else {
            if (driver_node == nullptr) {
                driver_node = new_driver_node();
            }
            // Fill the driver perms
            memset(driver_node->datum.xperms->perms, ~0, sizeof(avtab_extended_perms_t::perms));

            if (ioctl_driver(p.low) != ioctl_driver(p.high)) {
                for (int i = ioctl_driver(p.low); i <= ioctl_driver(p.high); ++i) {
                    xperm_clear(i, driver_node->datum.xperms->perms);
                }
            } else {
                uint8_t driver = ioctl_driver(p.low);
                auto node = node_list[driver];
                if (node == nullptr) {
                    node = new_func_node(driver);
                    // Fill the func perms
                    memset(node->datum.xperms->perms, ~0, sizeof(avtab_extended_perms_t::perms));
                    node_list[driver] = node;
                }
                xperm_clear(driver, driver_node->datum.xperms->perms);
                for (int i = ioctl_func(p.low); i <= ioctl_func(p.high); ++i) {
                    xperm_clear(i, node->datum.xperms->perms);
                }
            }
        }
    }
}

bool sepol_impl::add_xperm_rule(Str s, Str t, Str c, const Xperm &p, int effect) {
    type_datum_t *src = nullptr, *tgt = nullptr;
    class_datum_t *cls = nullptr;

    if (!s.empty()) {
        src = hashtab_find(db->p_types.table, s);
        if (src == nullptr) {
            LOGW("source type %.*s does not exist\n", (int) s.size(), s.data());
            return false;
        }
    }

    if (!t.empty()) {
        tgt = hashtab_find(db->p_types.table, t);
        if (tgt == nullptr) {
            LOGW("target type %.*s does not exist\n", (int) t.size(), t.data());
            return false;
        }
    }

    if (!c.empty()) {
        cls = hashtab_find(db->p_classes.table, c);
        if (cls == nullptr) {
            LOGW("class %.*s does not exist\n", (int) c.size(), c.data());
            return false;
        }
    }

    add_xperm_rule(src, tgt, cls, p, effect);
    return true;
}

bool sepol_impl::add_type_rule(Str s, Str t, Str c, Str d, int effect) {
    type_datum_t *src, *tgt, *def;
    class_datum_t *cls;

    src = hashtab_find(db->p_types.table, s);
    if (src == nullptr) {
        LOGW("source type %.*s does not exist\n", (int) s.size(), s.data());
        return false;
    }
    tgt = hashtab_find(db->p_types.table, t);
    if (tgt == nullptr) {
        LOGW("target type %.*s does not exist\n", (int) t.size(), t.data());
        return false;
    }
    cls = hashtab_find(db->p_classes.table, c);
    if (cls == nullptr) {
        LOGW("class %.*s does not exist\n", (int) c.size(), c.data());
        return false;
    }
    def = hashtab_find(db->p_types.table, d);
    if (def == nullptr) {
        LOGW("default type %.*s does not exist\n", (int) d.size(), d.data());
        return false;
    }

    avtab_key_t key;
    key.source_type = src->s.value;
    key.target_type = tgt->s.value;
    key.target_class = cls->s.value;
    key.specified = effect;

    avtab_ptr_t node = get_avtab_node(&key, nullptr);
    node->datum.data = def->s.value;

    return true;
}

bool sepol_impl::add_filename_trans(Str s, Str t, Str c, Str d, Str o) {
    type_datum_t *src, *tgt, *def;
    class_datum_t *cls;

    src = hashtab_find(db->p_types.table, s);
    if (src == nullptr) {
        LOGW("source type %.*s does not exist\n", (int) s.size(), s.data());
        return false;
    }
    tgt = hashtab_find(db->p_types.table, t);
    if (tgt == nullptr) {
        LOGW("target type %.*s does not exist\n", (int) t.size(), t.data());
        return false;
    }
    cls = hashtab_find(db->p_classes.table, c);
    if (cls == nullptr) {
        LOGW("class %.*s does not exist\n", (int) c.size(), c.data());
        return false;
    }
    def = hashtab_find(db->p_types.table, d);
    if (def == nullptr) {
        LOGW("default type %.*s does not exist\n", (int) d.size(), d.data());
        return false;
    }

    array<char, 256> key_name{};
    copy_str(key_name, o);
    filename_trans_key_t key;
    key.ttype = tgt->s.value;
    key.tclass = cls->s.value;
    key.name = key_name.data();

    filename_trans_datum_t *trans = hashtab_find(db->filename_trans, (hashtab_key_t) &key);
    filename_trans_datum_t *last = nullptr;
    while (trans) {
        if (ebitmap_get_bit(&trans->stypes, src->s.value - 1)) {
            // Duplicate, overwrite existing data and return
            trans->otype = def->s.value;
            return true;
        }
        if (trans->otype == def->s.value)
            break;
        last = trans;
        trans = trans->next;
    }
    if (trans == nullptr) {
        trans = auto_cast(calloc(sizeof(*trans), 1));
        ebitmap_init(&trans->stypes);
        trans->otype = def->s.value;
    }
    if (last) {
        last->next = trans;
    } else {
        filename_trans_key_t *new_key = auto_cast(malloc(sizeof(*new_key)));
        memcpy(new_key, &key, sizeof(key));
        new_key->name = strdup(key.name);
        hashtab_insert(db->filename_trans, (hashtab_key_t) new_key, trans);
    }

    db->filename_trans_count++;
    return ebitmap_set_bit(&trans->stypes, src->s.value - 1, 1) == 0;
}

bool sepol_impl::add_genfscon(Str fs_name, Str path, Str context) {
    // First try to create context
    context_struct_t *ctx;
    if (context_from_string(nullptr, db, &ctx, context.data(), context.size())) {
        LOGW("Failed to create context from string [%.*s]\n", (int) context.size(), context.data());
        return false;
    }

    // Find genfs node
    genfs_t *fs = list_find(db->genfs, [&](genfs_t *n) {
        return str_eq(n->fstype, fs_name);
    });
    if (fs == nullptr) {
        fs = auto_cast(calloc(sizeof(*fs), 1));
        fs->fstype = dup_str(fs_name);
        fs->next = db->genfs;
        db->genfs = fs;
    }

    // Find context node
    ocontext_t *o_ctx = list_find(fs->head, [&](ocontext_t *n) {
        return str_eq(n->u.name, path);
    });
    if (o_ctx == nullptr) {
        o_ctx = auto_cast(calloc(sizeof(*o_ctx), 1));
        o_ctx->u.name = dup_str(path);
        o_ctx->next = fs->head;
        fs->head = o_ctx;
    }
    memset(o_ctx->context, 0, sizeof(o_ctx->context));
    memcpy(&o_ctx->context[0], ctx, sizeof(*ctx));
    free(ctx);

    return true;
}

bool sepol_impl::add_type(Str type_name, uint32_t flavor) {
    type_datum_t *type = hashtab_find(db->p_types.table, type_name);
    if (type) {
        LOGW("Type %.*s already exists\n", (int) type_name.size(), type_name.data());
        return true;
    }

    type = auto_cast(malloc(sizeof(*type)));
    type_datum_init(type);
    type->primary = 1;
    type->flavor = flavor;

    uint32_t value = 0;
    auto ty_name = dup_str(type_name);
    if (symtab_insert(db, SYM_TYPES, ty_name, type, SCOPE_DECL, 1, &value)) {
        free(ty_name);
        return false;
    }
    type->s.value = value;
    ebitmap_set_bit(&db->global->branch_list->declared.p_types_scope, value - 1, 1);

    auto new_size = sizeof(ebitmap_t) * db->p_types.nprim;
    db->type_attr_map = auto_cast(realloc(db->type_attr_map, new_size));
    db->attr_type_map = auto_cast(realloc(db->attr_type_map, new_size));
    ebitmap_init(&db->type_attr_map[value - 1]);
    ebitmap_init(&db->attr_type_map[value - 1]);
    ebitmap_set_bit(&db->type_attr_map[value - 1], value - 1, 1);

    // Re-index stuffs
    if (policydb_index_decls(nullptr, db) ||
        policydb_index_classes(db) || policydb_index_others(nullptr, db, 0))
        return false;

    // Add the type to all roles
    for (int i = 0; i < db->p_roles.nprim; ++i) {
        // Not sure all those three calls are needed
        ebitmap_set_bit(&db->role_val_to_struct[i]->types.negset, value - 1, 0);
        ebitmap_set_bit(&db->role_val_to_struct[i]->types.types, value - 1, 1);
        type_set_expand(&db->role_val_to_struct[i]->types, &db->role_val_to_struct[i]->cache, db, 0);
    }

    return true;
}

bool sepol_impl::set_type_state(Str type_name, bool permissive) {
    type_datum_t *type;
    if (type_name.empty()) {
        hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
            type = auto_cast(node->datum);
            if (ebitmap_set_bit(&db->permissive_map, type->s.value, permissive))
                LOGW("Could not set bit in permissive map\n");
        });
    } else {
        type = hashtab_find(db->p_types.table, type_name);
        if (type == nullptr) {
            LOGW("type %.*s does not exist\n", (int) type_name.size(), type_name.data());
            return false;
        }
        if (ebitmap_set_bit(&db->permissive_map, type->s.value, permissive)) {
            LOGW("Could not set bit in permissive map\n");
            return false;
        }
    }
    return true;
}

void sepol_impl::add_typeattribute(type_datum_t *type, type_datum_t *attr) {
    ebitmap_set_bit(&db->type_attr_map[type->s.value - 1], attr->s.value - 1, 1);
    ebitmap_set_bit(&db->attr_type_map[attr->s.value - 1], type->s.value - 1, 1);

    hashtab_for_each(db->p_classes.table, [&](hashtab_ptr_t node){
        auto cls = static_cast<class_datum_t *>(node->datum);
        list_for_each(cls->constraints, [&](constraint_node_t *n) {
            list_for_each(n->expr, [&](constraint_expr_t *e) {
                if (e->expr_type == CEXPR_NAMES &&
                    ebitmap_get_bit(&e->type_names->types, attr->s.value - 1)) {
                    ebitmap_set_bit(&e->names, type->s.value - 1, 1);
                }
            });
        });
    });
}

bool sepol_impl::add_typeattribute(Str type, Str attr) {
    type_datum_t *type_d = hashtab_find(db->p_types.table, type);
    if (type_d == nullptr) {
        LOGW("type %.*s does not exist\n", (int) type.size(), type.data());
        return false;
    } else if (type_d->flavor == TYPE_ATTRIB) {
        LOGW("type %.*s is an attribute\n", (int) type.size(), type.data());
        return false;
    }

    type_datum *attr_d = hashtab_find(db->p_types.table, attr);
    if (attr_d == nullptr) {
        LOGW("attribute %.*s does not exist\n", (int) attr.size(), attr.data());
        return false;
    } else if (attr_d->flavor != TYPE_ATTRIB) {
        LOGW("type %.*s is not an attribute \n", (int) attr.size(), attr.data());
        return false;
    }

    add_typeattribute(type_d, attr_d);
    return true;
}

void SePolicy::strip_dontaudit() noexcept {
    avtab_for_each(&impl->db->te_avtab, [this](avtab_ptr_t node) {
        if (node->key.specified == AVTAB_AUDITDENY || node->key.specified == AVTAB_XPERMS_DONTAUDIT)
            avtab_remove_node(&impl->db->te_avtab, node);
    });
}

void SePolicy::print_rules() const noexcept {
    hashtab_for_each(impl->db->p_types.table, [this](hashtab_ptr_t node) {
        type_datum_t *type = auto_cast(node->datum);
        if (type->flavor == TYPE_ATTRIB) {
            impl->print_type(stdout, type);
        }
    });
    hashtab_for_each(impl->db->p_types.table, [this](hashtab_ptr_t node) {
        type_datum_t *type = auto_cast(node->datum);
        if (type->flavor == TYPE_TYPE) {
            impl->print_type(stdout, type);
        }
    });
    avtab_for_each(&impl->db->te_avtab, [this](avtab_ptr_t node) {
        impl->print_avtab(stdout, node);
    });
    hashtab_for_each(impl->db->filename_trans, [this](hashtab_ptr_t node) {
        impl->print_filename_trans(stdout, node);
    });
    list_for_each(impl->db->genfs, [this](genfs_t *genfs) {
        list_for_each(genfs->head, [&](ocontext *context) {
            char *ctx = nullptr;
            size_t len = 0;
            if (context_to_string(nullptr, impl->db, &context->context[0], &ctx, &len) == 0) {
                fprintf(stdout, "genfscon %s %s %s\n", genfs->fstype, context->u.name, ctx);
                free(ctx);
            }
        });
    });
}

void sepol_impl::print_type(FILE *fp, type_datum_t *type) {
    const char *name = db->p_type_val_to_name[type->s.value - 1];
    if (name == nullptr)
        return;
    if (type->flavor == TYPE_ATTRIB) {
        fprintf(fp, "attribute %s\n", name);
    } else if (type->flavor == TYPE_TYPE) {
        bool first = true;
        ebitmap_t *bitmap = &db->type_attr_map[type->s.value - 1];
        for (uint32_t i = 0; i <= bitmap->highbit; ++i) {
            if (ebitmap_get_bit(bitmap, i)) {
                auto attr_type = db->type_val_to_struct[i];
                if (attr_type->flavor == TYPE_ATTRIB) {
                    if (const char *attr = db->p_type_val_to_name[i]) {
                        if (first) {
                            fprintf(fp, "type %s {", name);
                            first = false;
                        }
                        fprintf(fp, " %s", attr);
                    }
                }
            }
        }
        if (!first) {
            fprintf(fp, " }\n");
        }
    }
    if (ebitmap_get_bit(&db->permissive_map, type->s.value)) {
        fprintf(fp, "permissive %s\n", name);
    }
}

void sepol_impl::print_avtab(FILE *fp, avtab_ptr_t node) {
    const char *src = db->p_type_val_to_name[node->key.source_type - 1];
    const char *tgt = db->p_type_val_to_name[node->key.target_type - 1];
    const char *cls = db->p_class_val_to_name[node->key.target_class - 1];
    if (src == nullptr || tgt == nullptr || cls == nullptr)
        return;

    if (node->key.specified & AVTAB_AV) {
        uint32_t data = node->datum.data;
        const char *name;
        switch (node->key.specified) {
            case AVTAB_ALLOWED:
                name = "allow";
                break;
            case AVTAB_AUDITALLOW:
                name = "auditallow";
                break;
            case AVTAB_AUDITDENY:
                name = "dontaudit";
                // Invert the rules for dontaudit
                data = ~data;
                break;
            default:
                return;
        }

        class_datum_t *clz = db->class_val_to_struct[node->key.target_class - 1];
        if (clz == nullptr)
            return;

        auto it = class_perm_names.find(cls);
        if (it == class_perm_names.end()) {
            it = class_perm_names.try_emplace(cls).first;
            // Find all permission names and cache the value
            hashtab_for_each(clz->permissions.table, [&](hashtab_ptr_t node) {
                perm_datum_t *perm = auto_cast(node->datum);
                it->second[perm->s.value - 1] = node->key;
            });
            if (clz->comdatum) {
                hashtab_for_each(clz->comdatum->permissions.table, [&](hashtab_ptr_t node) {
                    perm_datum_t *perm = auto_cast(node->datum);
                    it->second[perm->s.value - 1] = node->key;
                });
            }
        }

        bool first = true;
        for (int i = 0; i < 32; ++i) {
            if (data & (1u << i)) {
                if (const char *perm = it->second[i]) {
                    if (first) {
                        fprintf(fp, "%s %s %s %s {", name, src, tgt, cls);
                        first = false;
                    }
                    fprintf(fp, " %s", perm);
                }
            }
        }
        if (!first) {
            fprintf(fp, " }\n");
        }
    } else if (node->key.specified & AVTAB_TYPE) {
        const char *name;
        switch (node->key.specified) {
            case AVTAB_TRANSITION:
                name = "type_transition";
                break;
            case AVTAB_MEMBER:
                name = "type_member";
                break;
            case AVTAB_CHANGE:
                name = "type_change";
                break;
            default:
                return;
        }
        if (const char *def = db->p_type_val_to_name[node->datum.data - 1]) {
            fprintf(fp, "%s %s %s %s %s\n", name, src, tgt, cls, def);
        }
    } else if (node->key.specified & AVTAB_XPERMS) {
        const char *name;
        switch (node->key.specified) {
            case AVTAB_XPERMS_ALLOWED:
                name = "allowxperm";
                break;
            case AVTAB_XPERMS_AUDITALLOW:
                name = "auditallowxperm";
                break;
            case AVTAB_XPERMS_DONTAUDIT:
                name = "dontauditxperm";
                break;
            default:
                return;
        }
        avtab_extended_perms_t *xperms = node->datum.xperms;
        if (xperms == nullptr)
            return;

        vector<pair<uint8_t, uint8_t>> ranges;
        {
            int low = -1;
            for (int i = 0; i < 256; ++i) {
                if (xperm_test(i, xperms->perms)) {
                    if (low < 0) {
                        low = i;
                    }
                    if (i == 255) {
                        ranges.emplace_back(low, 255);
                    }
                } else if (low >= 0) {
                    ranges.emplace_back(low, i - 1);
                    low = -1;
                }
            }
        }

        auto to_value = [&](uint8_t val) -> uint16_t {
            if (xperms->specified == AVTAB_XPERMS_IOCTLFUNCTION) {
                return (((uint16_t) xperms->driver) << 8) | val;
            } else {
                return ((uint16_t) val) << 8;
            }
        };

        if (!ranges.empty()) {
            fprintf(fp, "%s %s %s %s ioctl {", name, src, tgt, cls);
            for (auto [l, h] : ranges) {
                uint16_t low = to_value(l);
                uint16_t high = to_value(h);
                if (low == high) {
                    fprintf(fp, " 0x%04X", low);
                } else {
                    fprintf(fp, " 0x%04X-0x%04X", low, high);
                }
            }
            fprintf(fp, " }\n");
        }
    }
}

void sepol_impl::print_filename_trans(FILE *fp, hashtab_ptr_t node) {
    auto key = reinterpret_cast<filename_trans_key_t *>(node->key);
    filename_trans_datum_t *trans = auto_cast(node->datum);

    const char *tgt = db->p_type_val_to_name[key->ttype - 1];
    const char *cls = db->p_class_val_to_name[key->tclass - 1];
    const char *def = db->p_type_val_to_name[trans->otype - 1];
    if (tgt == nullptr || cls == nullptr || def == nullptr || key->name == nullptr)
        return;

    for (uint32_t i = 0; i <= trans->stypes.highbit; ++i) {
        if (ebitmap_get_bit(&trans->stypes, i)) {
            if (const char *src = db->p_type_val_to_name[i]) {
                fprintf(fp, "type_transition %s %s %s %s %s\n", src, tgt, cls, def, key->name);
            }
        }
    }
}
