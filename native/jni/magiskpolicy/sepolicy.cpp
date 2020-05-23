#include <sepol/policydb/policydb.h>

#include <magiskpolicy.hpp>
#include <logging.hpp>
#include <utils.hpp>

#include "sepolicy.hpp"

#if 0
// Print out all rules going through public API for debugging
template <typename ...Args>
static void dprint(const char *action, Args ...args) {
	std::string s(action);
	for (int i = 0; i < sizeof...(args); ++i) s += " %s";
	s += "\n";
	LOGD(s.data(), (args ? args : "*")...);
}
#else
#define dprint(...)
#endif

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

#define hashtab_for_each(hashtab, fn) \
hash_for_each((hashtab)->htable, (hashtab)->size, fn)

#define avtab_for_each(avtab, fn) \
hash_for_each((avtab)->htable, (avtab)->nslot, fn)

// libsepol internal APIs
extern "C" int policydb_index_decls(sepol_handle_t * handle, policydb_t * p);
extern "C" int context_from_string(
		sepol_handle_t * handle,
		const policydb_t * policydb,
		context_struct_t ** cptr,
		const char *con_str, size_t con_str_len);
extern "C" int type_set_expand(
		type_set_t * set, ebitmap_t * t, policydb_t * p,
		unsigned char alwaysexpand);

int sepol_impl::set_attr(const char *attr_name, int type_val) {
	type_datum_t *attr = hashtab_find(db->p_types.table, attr_name);
	if (!attr || attr->flavor != TYPE_ATTRIB)
		return -1;

	if (ebitmap_set_bit(&db->type_attr_map[type_val - 1], attr->s.value - 1, 1))
		return -1;
	if (ebitmap_set_bit(&db->attr_type_map[attr->s.value - 1], type_val - 1, 1))
		return -1;

	return attr->s.value;
}

void sepol_impl::check_avtab_node(avtab_ptr_t node) {
	bool redundant;
	if (node->key.specified == AVTAB_AUDITDENY)
		redundant = node->datum.data == ~0U;
	else if (node->key.specified & AVTAB_XPERMS)
		redundant = node->datum.xperms == nullptr;
	else
		redundant = node->datum.data == 0U;
	if (redundant)
		avtab_remove_node(&db->te_avtab, node);
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

int sepol_impl::add_avrule(avtab_key_t *key, int val, bool n) {
	avtab_ptr_t node = get_avtab_node(key, nullptr);

	if (n) {
		if (val < 0)
			node->datum.data = 0U;
		else
			node->datum.data &= ~(1U << (val - 1));
	} else {
		if (val < 0)
			node->datum.data = ~0U;
		else
			node->datum.data |= 1U << (val - 1);
	}

	check_avtab_node(node);
	return 0;
}

int sepol_impl::add_rule(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, perm_datum_t *perm, int effect, bool n) {
	int ret = 0;

	if (src == nullptr) {
		hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
			src = auto_cast(node->datum);
			ret |= add_rule(src, tgt, cls, perm, effect, n);
		});
	} else if (tgt == nullptr) {
		hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
			tgt = auto_cast(node->datum);
			ret |= add_rule(src, tgt, cls, perm, effect, n);
		});
	} else if (cls == nullptr) {
		hashtab_for_each(db->p_classes.table, [&](hashtab_ptr_t node) {
			cls = auto_cast(node->datum);
			ret |= add_rule(src, tgt, cls, perm, effect, n);
		});
	} else {
		avtab_key_t key;
		key.source_type = src->s.value;
		key.target_type = tgt->s.value;
		key.target_class = cls->s.value;
		key.specified = effect;
		return add_avrule(&key, perm ? perm->s.value : -1, n);
	}

	return ret;
}

int sepol_impl::add_rule(const char *s, const char *t, const char *c, const char *p, int effect, bool n) {
	type_datum_t *src = nullptr, *tgt = nullptr;
	class_datum_t *cls = nullptr;
	perm_datum_t *perm = nullptr;

	if (s) {
		src = hashtab_find(db->p_types.table, s);
		if (src == nullptr) {
			LOGW("source type %s does not exist\n", s);
			return 1;
		}
	}

	if (t) {
		tgt = hashtab_find(db->p_types.table, t);
		if (tgt == nullptr) {
			LOGW("target type %s does not exist\n", t);
			return 1;
		}
	}

	if (c) {
		cls = hashtab_find(db->p_classes.table, c);
		if (cls == nullptr) {
			LOGW("class %s does not exist\n", c);
			return 1;
		}
	}

	if (p) {
		if (c == nullptr) {
			LOGW("No class is specified, cannot add perm [%s] \n", p);
			return 1;
		}

		perm = hashtab_find(cls->permissions.table, p);
		if (perm == nullptr && cls->comdatum != nullptr) {
			perm = hashtab_find(cls->comdatum->permissions.table, p);
		}
		if (perm == nullptr) {
			LOGW("perm %s does not exist in class %s\n", p, c);
			return 1;
		}
	}
	return add_rule(src, tgt, cls, perm, effect, n);
}

#define ioctl_driver(x) (x>>8 & 0xFF)
#define ioctl_func(x) (x & 0xFF)

int sepol_impl::add_avxrule(avtab_key_t *key, uint16_t low, uint16_t high, bool n) {
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
			if (n)
				xperm_clear(i, xperms.perms);
			else
				xperm_set(i, xperms.perms);
		}
	} else {
		for (int i = ioctl_func(low); i <= ioctl_func(high); ++i) {
			if (n)
				xperm_clear(i, xperms.perms);
			else
				xperm_set(i, xperms.perms);
		}
	}

	datum = &get_avtab_node(key, &xperms)->datum;

	if (datum->xperms == nullptr)
		datum->xperms = auto_cast(xmalloc(sizeof(xperms)));

	memcpy(datum->xperms, &xperms, sizeof(xperms));
	return 0;
}

int sepol_impl::add_xperm_rule(type_datum_t *src, type_datum_t *tgt,
		class_datum_t *cls, uint16_t low, uint16_t high, int effect, bool n) {
	int ret = 0;
	if (src == nullptr) {
		hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
			src = auto_cast(node->datum);
			ret |= add_xperm_rule(src, tgt, cls, low, high, effect, n);
		});
	} else if (tgt == nullptr) {
		hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
			tgt = auto_cast(node->datum);
			ret |= add_xperm_rule(src, tgt, cls, low, high, effect, n);
		});
	} else if (cls == nullptr) {
		hashtab_for_each(db->p_classes.table, [&](hashtab_ptr_t node) {
			tgt = auto_cast(node->datum);
			ret |= add_xperm_rule(src, tgt, cls, low, high, effect, n);
		});
	} else {
		avtab_key_t key;
		key.source_type = src->s.value;
		key.target_type = tgt->s.value;
		key.target_class = cls->s.value;
		key.specified = effect;
		return add_avxrule(&key, low, high, n);
	}
	return ret;
}

int sepol_impl::add_xperm_rule(const char *s, const char *t, const char *c, const char *range, int effect, bool n) {
	type_datum_t *src = nullptr, *tgt = nullptr;
	class_datum_t *cls = nullptr;

	if (s) {
		src = hashtab_find(db->p_types.table, s);
		if (src == nullptr) {
			LOGW("source type %s does not exist\n", s);
			return 1;
		}
	}

	if (t) {
		tgt = hashtab_find(db->p_types.table, t);
		if (tgt == nullptr) {
			LOGW("target type %s does not exist\n", t);
			return 1;
		}
	}

	if (c) {
		cls = hashtab_find(db->p_classes.table, c);
		if (cls == nullptr) {
			LOGW("class %s does not exist\n", c);
			return 1;
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

	return add_xperm_rule(src, tgt, cls, low, high, effect, n);
}

int sepol_impl::create_domain(const char *type_name) {
	symtab_datum_t *src = hashtab_find(db->p_types.table, type_name);
	if (src) {
		LOGW("Type %s already exists\n", type_name);
		return 0;
	}

	type_datum_t *type = auto_cast(xmalloc(sizeof(type_datum_t)));
	type_datum_init(type);
	type->primary = 1;
	type->flavor = TYPE_TYPE;

	uint32_t value = 0;
	symtab_insert(db, SYM_TYPES, strdup(type_name), type, SCOPE_DECL, 1, &value);
	type->s.value = value;

	if (ebitmap_set_bit(&db->global->branch_list->declared.scope[SYM_TYPES], value - 1, 1)) {
		return 1;
	}

	db->type_attr_map = auto_cast(xrealloc(db->type_attr_map, sizeof(ebitmap_t) * db->p_types.nprim));
	db->attr_type_map = auto_cast(xrealloc(db->attr_type_map, sizeof(ebitmap_t) * db->p_types.nprim));
	ebitmap_init(&db->type_attr_map[value - 1]);
	ebitmap_init(&db->attr_type_map[value - 1]);
	ebitmap_set_bit(&db->type_attr_map[value - 1], value - 1, 1);

	src = hashtab_find(db->p_types.table, type_name);
	if (!src)
		return 1;

	if (policydb_index_decls(nullptr, db))
		return 1;

	if (policydb_index_classes(db))
		return 1;

	if (policydb_index_others(nullptr, db, 0))
		return 1;

	// Add the domain to all roles
	for (unsigned i = 0; i < db->p_roles.nprim; ++i) {
		// Not sure all those three calls are needed
		ebitmap_set_bit(&db->role_val_to_struct[i]->types.negset, value - 1, 0);
		ebitmap_set_bit(&db->role_val_to_struct[i]->types.types, value - 1, 1);
		type_set_expand(&db->role_val_to_struct[i]->types, &db->role_val_to_struct[i]->cache, db, 0);
	}

	return set_attr("domain", value);
}

int sepol_impl::set_domain_state(const char *s, bool permissive) {
	type_datum_t *type;
	if (s == nullptr) {
		hashtab_for_each(db->p_types.table, [&](hashtab_ptr_t node) {
			type = auto_cast(node->datum);
			if (ebitmap_set_bit(&db->permissive_map, type->s.value, permissive))
				LOGW("Could not set bit in permissive map\n");
		});
	} else {
		type = hashtab_find(db->p_types.table, s);
		if (type == nullptr) {
			LOGW("type %s does not exist\n", s);
			return 1;
		}
		if (ebitmap_set_bit(&db->permissive_map, type->s.value, permissive)) {
			LOGW("Could not set bit in permissive map\n");
			return 1;
		}
	}
	return 0;
}

int sepol_impl::add_filename_trans(const char *s, const char *t, const char *c, const char *d, const char *o) {
	type_datum_t *src, *tgt, *def;
	class_datum_t *cls;
	filename_trans_datum_t *trans;

	src = hashtab_find(db->p_types.table, s);
	if (src == nullptr) {
		LOGW("source type %s does not exist\n", s);
		return 1;
	}
	tgt = hashtab_find(db->p_types.table, t);
	if (tgt == nullptr) {
		LOGW("target type %s does not exist\n", t);
		return 1;
	}
	cls = hashtab_find(db->p_classes.table, c);
	if (cls == nullptr) {
		LOGW("class %s does not exist\n", c);
		return 1;
	}
	def = hashtab_find(db->p_types.table, d);
	if (def == nullptr) {
		LOGW("default type %s does not exist\n", d);
		return 1;
	}

	filename_trans_t trans_key;
	trans_key.stype = src->s.value;
	trans_key.ttype = tgt->s.value;
	trans_key.tclass = cls->s.value;
	trans_key.name = (char *) o;

	trans = hashtab_find(db->filename_trans, (hashtab_key_t) &trans_key);

	if (trans == nullptr) {
		trans = auto_cast(xcalloc(sizeof(*trans), 1));
		hashtab_insert(db->filename_trans, (hashtab_key_t) &trans_key, trans);
	}

	// Overwrite existing
	trans->otype = def->s.value;
	return 0;
}

int sepol_impl::add_typeattribute(const char *type, const char *attr) {
	type_datum_t *domain = hashtab_find(db->p_types.table, type);
	if (domain == nullptr) {
		LOGW("type %s does not exist\n", type);
		return 1;
	}

	int attr_val = set_attr(attr, domain->s.value);
	if (attr_val < 0)
		return 1;

	hashtab_for_each(db->p_classes.table, [&](hashtab_ptr_t node){
		auto cls = static_cast<class_datum_t *>(node->datum);
		for (constraint_node_t *n = cls->constraints; n ; n = n->next) {
			for (constraint_expr_t *e = n->expr; e; e = e->next) {
				if (e->expr_type == CEXPR_NAMES &&
					ebitmap_get_bit(&e->type_names->types, attr_val - 1)) {
					ebitmap_set_bit(&e->names, domain->s.value - 1, 1);
				}
			}
		}
	});
	return 0;
}

int sepol_impl::add_type_rule(const char *s, const char *t, const char *c, const char *d, int effect) {
	type_datum_t *src, *tgt, *def;
	class_datum_t *cls;

	src = hashtab_find(db->p_types.table, s);
	if (src == nullptr) {
		LOGW("source type %s does not exist\n", s);
		return 1;
	}
	tgt = hashtab_find(db->p_types.table, t);
	if (tgt == nullptr) {
		LOGW("target type %s does not exist\n", t);
		return 1;
	}
	cls = hashtab_find(db->p_classes.table, c);
	if (cls == nullptr) {
		LOGW("class %s does not exist\n", c);
		return 1;
	}
	def = hashtab_find(db->p_types.table, d);
	if (def == nullptr) {
		LOGW("default type %s does not exist\n", d);
		return 1;
	}

	avtab_key_t key;
	key.source_type = src->s.value;
	key.target_type = tgt->s.value;
	key.target_class = cls->s.value;
	key.specified = effect;

	avtab_ptr_t node = get_avtab_node(&key, nullptr);
	node->datum.data = def->s.value;

	return 0;
}

int sepol_impl::add_genfscon(const char *fs_name, const char *path, const char *context) {
	// First try to create context
	context_struct_t *ctx;
	if (context_from_string(nullptr, db, &ctx, context, strlen(context))) {
		LOGW("Failed to create context from string [%s]\n", context);
		return 1;
	}

	// Allocate genfs context
	ocontext_t *newc = auto_cast(xcalloc(sizeof(*newc), 1));
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
		newfs = auto_cast(xcalloc(sizeof(*newfs), 1));
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

	return 0;
}

void sepol_impl::strip_dontaudit() {
	avtab_for_each(&db->te_avtab, [=](avtab_ptr_t node) {
		if (node->key.specified == AVTAB_AUDITDENY || node->key.specified == AVTAB_XPERMS_DONTAUDIT)
			avtab_remove_node(&db->te_avtab, node);
	});
}

int sepolicy::allow(const char *s, const char *t, const char *c, const char *p) {
	dprint(__FUNCTION__, s, t, c, p);
	return impl->add_rule(s, t, c, p, AVTAB_ALLOWED, false);
}

int sepolicy::deny(const char *s, const char *t, const char *c, const char *p) {
	dprint(__FUNCTION__, s, t, c, p);
	return impl->add_rule(s, t, c, p, AVTAB_ALLOWED, true);
}

int sepolicy::auditallow(const char *s, const char *t, const char *c, const char *p) {
	dprint(__FUNCTION__, s, t, c, p);
	return impl->add_rule(s, t, c, p, AVTAB_AUDITALLOW, false);
}

int sepolicy::dontaudit(const char *s, const char *t, const char *c, const char *p) {
	dprint(__FUNCTION__, s, t, c, p);
	return impl->add_rule(s, t, c, p, AVTAB_AUDITDENY, true);
}

int sepolicy::allowxperm(const char *s, const char *t, const char *c, const char *range) {
	dprint(__FUNCTION__, s, t, c, "ioctl", range);
	return impl->add_xperm_rule(s, t, c, range, AVTAB_XPERMS_ALLOWED, false);
}

int sepolicy::auditallowxperm(const char *s, const char *t, const char *c, const char *range) {
	dprint(__FUNCTION__, s, t, c, "ioctl", range);
	return impl->add_xperm_rule(s, t, c, range, AVTAB_XPERMS_AUDITALLOW, false);
}

int sepolicy::dontauditxperm(const char *s, const char *t, const char *c, const char *range) {
	dprint(__FUNCTION__, s, t, c, "ioctl", range);
	return impl->add_xperm_rule(s, t, c, range, AVTAB_XPERMS_DONTAUDIT, false);
}

int sepolicy::type_change(const char *s, const char *t, const char *c, const char *d) {
	dprint(__FUNCTION__, s, t, c, d);
	return impl->add_type_rule(s, t, c, d, AVTAB_CHANGE);
}

int sepolicy::type_member(const char *s, const char *t, const char *c, const char *d) {
	dprint(__FUNCTION__, s, t, c, d);
	return impl->add_type_rule(s, t, c, d, AVTAB_MEMBER);
}

int sepolicy::type_transition(const char *s, const char *t, const char *c, const char *d, const char *o) {
	if (o) {
		dprint(__FUNCTION__, s, t, c, d);
		return impl->add_type_rule(s, t, c, d, AVTAB_TRANSITION);
	} else {
		dprint(__FUNCTION__, s, t, c, d, o);
		return impl->add_filename_trans(s, t, c, d, o);
	}
}

int sepolicy::permissive(const char *s) {
	dprint(__FUNCTION__, s);
	return impl->set_domain_state(s, true);
}

int sepolicy::enforce(const char *s) {
	dprint(__FUNCTION__, s);
	return impl->set_domain_state(s, false);
}

int sepolicy::create(const char *s) {
	dprint(__FUNCTION__, s);
	return impl->create_domain(s);
}

int sepolicy::typeattribute(const char *type, const char *attr) {
	dprint(__FUNCTION__, type, attr);
	return impl->add_typeattribute(type, attr);
}

int sepolicy::genfscon(const char *fs_name, const char *path, const char *ctx) {
	dprint(__FUNCTION__, fs_name, path, ctx);
	return impl->add_genfscon(fs_name, path, ctx);
}

int sepolicy::exists(const char *source) {
	return hashtab_search(db->p_types.table, source) != nullptr;
}
