#include <base.hpp>

#include "policy.hpp"

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
__END_DECLS

template <typename T>
struct auto_cast_wrapper
{
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

static auto hashtab_find(hashtab_t h, const_hashtab_key_t key) {
    return auto_cast(hashtab_search(h, key));
}

template <class Node, class Func>
static void hash_for_each(Node **node_ptr, int n_slot, const Func &fn) {
    for (int i = 0; i < n_slot; ++i) {
        for (Node *cur = node_ptr[i]; cur; cur = cur->next) {
            fn(cur);
        }
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
    avtab_ptr_t prev, cur;
    for (prev = nullptr, cur = h->htable[hvalue]; cur; prev = cur, cur = cur->next) {
        if (cur == node)
            break;
    }
    if (cur == nullptr)
        return SEPOL_ENOENT;

    // Detach from hash table
    if (prev)
        prev->next = node->next;
    else
        h->htable[hvalue] = node->next;
    h->nel--;

    // Free memory
    if (node->key.specified & AVTAB_XPERMS)
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

avtab_ptr_t sepol_impl::get_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms) {
    avtab_ptr_t node;

    /* AVTAB_XPERMS entries are not necessarily unique */
    if (key->specified & AVTAB_XPERMS) {
        bool match = false;
        node = avtab_search_node(&db->te_avtab, key);
        while (node) {
            if ((node->datum.xperms->specified == xperms->specified) &&
                (node->datum.xperms->driver == xperms->driver)) {
                match = true;
                break;
            }
            node = avtab_search_node_next(node, key->specified);
        }
        if (!match)
            node = nullptr;
    } else {
        node = avtab_search_node(&db->te_avtab, key);
    }

    if (!node) {
        avtab_datum_t avdatum{};
        /*
         * AUDITDENY, aka DONTAUDIT, are &= assigned, versus |= for
         * others. Initialize the data accordingly.
         */
        avdatum.data = key->specified == AVTAB_AUDITDENY ? ~0U : 0U;
        /* this is used to get the node - insertion is actually unique */
        node = avtab_insert_nonunique(&db->te_avtab, key, &avdatum);
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

bool sepol_impl::add_rule(const char *s, const char *t, const char *c, const char *p, int effect, bool invert) {
    type_datum_t *src = nullptr, *tgt = nullptr;
    class_datum_t *cls = nullptr;
    perm_datum_t *perm = nullptr;

    if (s) {
        src = hashtab_find(db->p_types.table, s);
        if (src == nullptr) {
            LOGW("source type %s does not exist\n", s);
            return false;
        }
    }

    if (t) {
        tgt = hashtab_find(db->p_types.table, t);
        if (tgt == nullptr) {
            LOGW("target type %s does not exist\n", t);
            return false;
        }
    }

    if (c) {
        cls = hashtab_find(db->p_classes.table, c);
        if (cls == nullptr) {
            LOGW("class %s does not exist\n", c);
            return false;
        }
    }

    if (p) {
        if (c == nullptr) {
            LOGW("No class is specified, cannot add perm [%s] \n", p);
            return false;
        }

        perm = hashtab_find(cls->permissions.table, p);
        if (perm == nullptr && cls->comdatum != nullptr) {
            perm = hashtab_find(cls->comdatum->permissions.table, p);
        }
        if (perm == nullptr) {
            LOGW("perm %s does not exist in class %s\n", p, c);
            return false;
        }
    }
    add_rule(src, tgt, cls, perm, effect, invert);
    return true;
}

#define ioctl_driver(x) (x>>8 & 0xFF)
#define ioctl_func(x) (x & 0xFF)

void sepol_impl::add_xperm_rule(type_datum_t *src, type_datum_t *tgt,
        class_datum_t *cls, uint16_t low, uint16_t high, int effect, bool invert) {
    if (src == nullptr) {
        for_each_attr(db->p_types.table, [&](type_datum_t *type) {
            add_xperm_rule(type, tgt, cls, low, high, effect, invert);
        });
    } else if (tgt == nullptr) {
        for_each_attr(db->p_types.table, [&](type_datum_t *type) {
            add_xperm_rule(src, type, cls, low, high, effect, invert);
        });
    } else if (cls == nullptr) {
        hashtab_for_each(db->p_classes.table, [&](hashtab_ptr_t node) {
            add_xperm_rule(src, tgt, auto_cast(node->datum), low, high, effect, invert);
        });
    } else {
        avtab_key_t key;
        key.source_type = src->s.value;
        key.target_type = tgt->s.value;
        key.target_class = cls->s.value;
        key.specified = effect;

        avtab_datum_t *datum;
        avtab_extended_perms_t xperms;

        memset(&xperms, 0, sizeof(xperms));
        if (ioctl_driver(low) != ioctl_driver(high)) {
            xperms.specified = AVTAB_XPERMS_IOCTLDRIVER;
            xperms.driver = 0;
        } else {
            xperms.specified = AVTAB_XPERMS_IOCTLFUNCTION;
            xperms.driver = ioctl_driver(low);
        }

        if (xperms.specified == AVTAB_XPERMS_IOCTLDRIVER) {
            for (int i = ioctl_driver(low); i <= ioctl_driver(high); ++i) {
                if (invert)
                    xperm_clear(i, xperms.perms);
                else
                    xperm_set(i, xperms.perms);
            }
        } else {
            for (int i = ioctl_func(low); i <= ioctl_func(high); ++i) {
                if (invert)
                    xperm_clear(i, xperms.perms);
                else
                    xperm_set(i, xperms.perms);
            }
        }

        datum = &get_avtab_node(&key, &xperms)->datum;

        if (datum->xperms == nullptr)
            datum->xperms = auto_cast(malloc(sizeof(xperms)));

        memcpy(datum->xperms, &xperms, sizeof(xperms));
    }
}

bool sepol_impl::add_xperm_rule(const char *s, const char *t, const char *c, const char *range, int effect, bool invert) {
    type_datum_t *src = nullptr, *tgt = nullptr;
    class_datum_t *cls = nullptr;

    if (s) {
        src = hashtab_find(db->p_types.table, s);
        if (src == nullptr) {
            LOGW("source type %s does not exist\n", s);
            return false;
        }
    }

    if (t) {
        tgt = hashtab_find(db->p_types.table, t);
        if (tgt == nullptr) {
            LOGW("target type %s does not exist\n", t);
            return false;
        }
    }

    if (c) {
        cls = hashtab_find(db->p_classes.table, c);
        if (cls == nullptr) {
            LOGW("class %s does not exist\n", c);
            return false;
        }
    }

    uint16_t low, high;

    if (range) {
        if (strchr(range, '-')){
            sscanf(range, "%hx-%hx", &low, &high);
        } else {
            sscanf(range, "%hx", &low);
            high = low;
        }
    } else {
        low = 0;
        high = 0xFFFF;
    }

    add_xperm_rule(src, tgt, cls, low, high, effect, invert);
    return true;
}

bool sepol_impl::add_type_rule(const char *s, const char *t, const char *c, const char *d, int effect) {
    type_datum_t *src, *tgt, *def;
    class_datum_t *cls;

    src = hashtab_find(db->p_types.table, s);
    if (src == nullptr) {
        LOGW("source type %s does not exist\n", s);
        return false;
    }
    tgt = hashtab_find(db->p_types.table, t);
    if (tgt == nullptr) {
        LOGW("target type %s does not exist\n", t);
        return false;
    }
    cls = hashtab_find(db->p_classes.table, c);
    if (cls == nullptr) {
        LOGW("class %s does not exist\n", c);
        return false;
    }
    def = hashtab_find(db->p_types.table, d);
    if (def == nullptr) {
        LOGW("default type %s does not exist\n", d);
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

bool sepol_impl::add_filename_trans(const char *s, const char *t, const char *c, const char *d, const char *o) {
    type_datum_t *src, *tgt, *def;
    class_datum_t *cls;

    src = hashtab_find(db->p_types.table, s);
    if (src == nullptr) {
        LOGW("source type %s does not exist\n", s);
        return false;
    }
    tgt = hashtab_find(db->p_types.table, t);
    if (tgt == nullptr) {
        LOGW("target type %s does not exist\n", t);
        return false;
    }
    cls = hashtab_find(db->p_classes.table, c);
    if (cls == nullptr) {
        LOGW("class %s does not exist\n", c);
        return false;
    }
    def = hashtab_find(db->p_types.table, d);
    if (def == nullptr) {
        LOGW("default type %s does not exist\n", d);
        return false;
    }

    filename_trans_key_t key;
    key.ttype = tgt->s.value;
    key.tclass = cls->s.value;
    key.name = (char *) o;

    filename_trans_datum_t *last = nullptr;
    filename_trans_datum_t *trans = hashtab_find(db->filename_trans, (hashtab_key_t) &key);
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
        filename_trans_key_t *new_key = auto_cast(malloc(sizeof(*new_key)));
        *new_key = key;
        new_key->name = strdup(key.name);
        trans->next = last;
        trans->otype = def->s.value;
        hashtab_insert(db->filename_trans, (hashtab_key_t) new_key, trans);
    }

    db->filename_trans_count++;
    return ebitmap_set_bit(&trans->stypes, src->s.value - 1, 1) == 0;
}

bool sepol_impl::add_genfscon(const char *fs_name, const char *path, const char *context) {
    // First try to create context
    context_struct_t *ctx;
    if (context_from_string(nullptr, db, &ctx, context, strlen(context))) {
        LOGW("Failed to create context from string [%s]\n", context);
        return false;
    }

    // Allocate genfs context
    ocontext_t *newc = auto_cast(calloc(sizeof(*newc), 1));
    newc->u.name = strdup(path);
    memcpy(&newc->context[0], ctx, sizeof(*ctx));
    free(ctx);

    // Find or allocate genfs
    genfs_t *last_gen = nullptr;
    genfs_t *newfs = nullptr;
    for (genfs_t *node = db->genfs; node; node = node->next) {
        if (strcmp(node->fstype, fs_name) == 0) {
            newfs = node;
            break;
        }
        last_gen = node;
    }
    if (newfs == nullptr) {
        newfs = auto_cast(calloc(sizeof(*newfs), 1));
        newfs->fstype = strdup(fs_name);
        // Insert
        if (last_gen)
            last_gen->next = newfs;
        else
            db->genfs = newfs;
    }

    // Insert or replace genfs context
    ocontext_t *last_ctx = nullptr;
    for (ocontext_t *node = newfs->head; node; node = node->next) {
        if (strcmp(node->u.name, path) == 0) {
            // Unlink
            if (last_ctx)
                last_ctx->next = node->next;
            else
                newfs->head = nullptr;
            // Destroy old node
            free(node->u.name);
            context_destroy(&node->context[0]);
            free(node);
            break;
        }
        last_ctx = node;
    }
    // Insert
    if (last_ctx)
        last_ctx->next = newc;
    else
        newfs->head = newc;

    return true;
}

bool sepol_impl::add_type(const char *type_name, uint32_t flavor) {
    type_datum_t *type = hashtab_find(db->p_types.table, type_name);
    if (type) {
        LOGW("Type %s already exists\n", type_name);
        return true;
    }

    type = auto_cast(malloc(sizeof(type_datum_t)));
    type_datum_init(type);
    type->primary = 1;
    type->flavor = flavor;

    uint32_t value = 0;
    if (symtab_insert(db, SYM_TYPES, strdup(type_name), type, SCOPE_DECL, 1, &value))
        return false;
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

bool sepol_impl::set_type_state(const char *type_name, bool permissive) {
    type_datum_t *type;
    if (type_name == nullptr) {
        hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
            type = auto_cast(node->datum);
            if (ebitmap_set_bit(&db->permissive_map, type->s.value, permissive))
                LOGW("Could not set bit in permissive map\n");
        });
    } else {
        type = hashtab_find(db->p_types.table, type_name);
        if (type == nullptr) {
            LOGW("type %s does not exist\n", type_name);
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
        for (constraint_node_t *n = cls->constraints; n ; n = n->next) {
            for (constraint_expr_t *e = n->expr; e; e = e->next) {
                if (e->expr_type == CEXPR_NAMES &&
                    ebitmap_get_bit(&e->type_names->types, attr->s.value - 1)) {
                    ebitmap_set_bit(&e->names, type->s.value - 1, 1);
                }
            }
        }
    });
}

bool sepol_impl::add_typeattribute(const char *type, const char *attr) {
    type_datum_t *type_d = hashtab_find(db->p_types.table, type);
    if (type_d == nullptr) {
        LOGW("type %s does not exist\n", type);
        return false;
    } else if (type_d->flavor == TYPE_ATTRIB) {
        LOGW("type %s is an attribute\n", attr);
        return false;
    }

    type_datum *attr_d = hashtab_find(db->p_types.table, attr);
    if (attr_d == nullptr) {
        LOGW("attribute %s does not exist\n", type);
        return false;
    } else if (attr_d->flavor != TYPE_ATTRIB) {
        LOGW("type %s is not an attribute \n", attr);
        return false;
    }

    add_typeattribute(type_d, attr_d);
    return true;
}

void sepol_impl::strip_dontaudit() {
    avtab_for_each(&db->te_avtab, [=](avtab_ptr_t node) {
        if (node->key.specified == AVTAB_AUDITDENY || node->key.specified == AVTAB_XPERMS_DONTAUDIT)
            avtab_remove_node(&db->te_avtab, node);
    });
}
