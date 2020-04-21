#include <stdlib.h>
#include <sepol/policydb/expand.h>

#include <logging.hpp>

#include "sepolicy.h"

policydb_t *magisk_policydb = NULL;
#define mpdb magisk_policydb

extern void *xmalloc(size_t size);
extern void *xcalloc(size_t nmemb, size_t size);
extern void *xrealloc(void *ptr, size_t size);

// Internal libsepol APIs
extern int policydb_index_decls(sepol_handle_t * handle, policydb_t * p);
extern int context_from_string(
		sepol_handle_t * handle,
		const policydb_t * policydb,
		context_struct_t ** cptr,
		const char *con_str, size_t con_str_len);

// Generic hash table traversal
#define hash_for_each(node_ptr, n_slots, table, block) \
for (int __i = 0; __i < (table)->n_slots; ++__i) { \
	__typeof__(*(table)->node_ptr) node; \
	__typeof__(node) __next; \
	for (node = (table)->node_ptr[__i]; node; node = __next) { \
		__next = node->next; \
		block \
	} \
}

// hashtab traversal
#define hashtab_for_each(hashtab, block) \
hash_for_each(htable, size, hashtab, block)

// avtab traversal
#define avtab_for_each(avtab, block) \
hash_for_each(htable, nslot, avtab, block)

static int set_attr(const char *type, int value) {
	type_datum_t *attr = hashtab_search(mpdb->p_types.table, type);
	if (!attr || attr->flavor != TYPE_ATTRIB)
		return -1;

	if (ebitmap_set_bit(&mpdb->type_attr_map[value - 1], attr->s.value - 1, 1))
		return -1;
	if (ebitmap_set_bit(&mpdb->attr_type_map[attr->s.value - 1], value - 1, 1))
		return -1;

	return attr->s.value;
}

static void check_avtab_node(avtab_ptr_t node) {
	int redundant = 0;
	if (node->key.specified == AVTAB_AUDITDENY)
		redundant = node->datum.data == ~0U;
	else if (node->key.specified & AVTAB_XPERMS)
		redundant = node->datum.xperms == NULL;
	else
		redundant = node->datum.data == 0U;
	if (redundant)
		avtab_remove_node(&mpdb->te_avtab, node);
}

static avtab_ptr_t get_avtab_node(avtab_key_t *key, avtab_extended_perms_t *xperms)  {
	avtab_ptr_t node;
	avtab_datum_t avdatum;
	int match = 0;

	/* AVTAB_XPERMS entries are not necessarily unique */
	if (key->specified & AVTAB_XPERMS) {
		node = avtab_search_node(&mpdb->te_avtab, key);
		while (node) {
			if ((node->datum.xperms->specified == xperms->specified) &&
				(node->datum.xperms->driver == xperms->driver)) {
				match = 1;
				break;
			}
			node = avtab_search_node_next(node, key->specified);
		}
		if (!match)
			node = NULL;
	} else {
		node = avtab_search_node(&mpdb->te_avtab, key);
	}

	if (!node) {
		memset(&avdatum, 0, sizeof avdatum);
		/*
		 * AUDITDENY, aka DONTAUDIT, are &= assigned, versus |= for
		 * others. Initialize the data accordingly.
		 */
		avdatum.data = key->specified == AVTAB_AUDITDENY ? ~0U : 0U;
		/* this is used to get the node - insertion is actually unique */
		node = avtab_insert_nonunique(&mpdb->te_avtab, key, &avdatum);
	}

	return node;
}

static int add_avrule(avtab_key_t *key, int val, int not) {
	avtab_ptr_t node = get_avtab_node(key, NULL);

	if (not) {
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

static int add_rule_auto(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls,
						 perm_datum_t *perm, int effect, int not) {
	avtab_key_t key;
	int ret = 0;

	if (src == NULL) {
		hashtab_for_each(mpdb->p_types.table, {
			src = node->datum;
			ret |= add_rule_auto(src, tgt, cls, perm, effect, not);
		})
	} else if (tgt == NULL) {
		hashtab_for_each(mpdb->p_types.table, {
			tgt = node->datum;
			ret |= add_rule_auto(src, tgt, cls, perm, effect, not);
		})
	} else if (cls == NULL) {
		hashtab_for_each(mpdb->p_classes.table, {
			cls = node->datum;
			ret |= add_rule_auto(src, tgt, cls, perm, effect, not);
		})
	} else {
		key.source_type = src->s.value;
		key.target_type = tgt->s.value;
		key.target_class = cls->s.value;
		key.specified = effect;
		return add_avrule(&key, perm ? perm->s.value : -1, not);
	}
	return ret;
}

#define ioctl_driver(x) (x>>8 & 0xFF)
#define ioctl_func(x) (x & 0xFF)

static int add_avxrule(avtab_key_t *key, uint16_t low, uint16_t high, int not) {
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
			if (not)
				xperm_clear(i, xperms.perms);
			else
				xperm_set(i, xperms.perms);
		}
	} else {
		for (int i = ioctl_func(low); i <= ioctl_func(high); ++i) {
			if (not)
				xperm_clear(i, xperms.perms);
			else
				xperm_set(i, xperms.perms);
		}
	}

	datum = &get_avtab_node(key, &xperms)->datum;

	if (datum->xperms == NULL)
		datum->xperms = xmalloc(sizeof(xperms));

	memcpy(datum->xperms, &xperms, sizeof(xperms));
	return 0;
}

static int add_xperm_rule_auto(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls,
			uint16_t low, uint16_t high, int effect, int not) {
	avtab_key_t key;
	int ret = 0;

	if (src == NULL) {
		hashtab_for_each(mpdb->p_types.table, {
			src = node->datum;
			ret |= add_xperm_rule_auto(src, tgt, cls, low, high, effect, not);
		})
	} else if (tgt == NULL) {
		hashtab_for_each(mpdb->p_types.table, {
			tgt = node->datum;
			ret |= add_xperm_rule_auto(src, tgt, cls, low, high, effect, not);
		})
	} else if (cls == NULL) {
		hashtab_for_each(mpdb->p_classes.table, {
			cls = node->datum;
			ret |= add_xperm_rule_auto(src, tgt, cls, low, high, effect, not);
		})
	} else {
		key.source_type = src->s.value;
		key.target_type = tgt->s.value;
		key.target_class = cls->s.value;
		key.specified = effect;
		return add_avxrule(&key, low, high, not);
	}
	return ret;
}

int create_domain(const char *d) {
	symtab_datum_t *src = hashtab_search(mpdb->p_types.table, d);
	if (src) {
		LOGW("Domain %s already exists\n", d);
		return 0;
	}

	type_datum_t *typedatum = xmalloc(sizeof(type_datum_t));
	type_datum_init(typedatum);
	typedatum->primary = 1;
	typedatum->flavor = TYPE_TYPE;

	uint32_t value = 0;
	symtab_insert(mpdb, SYM_TYPES, strdup(d), typedatum, SCOPE_DECL, 1, &value);
	typedatum->s.value = value;

	if (ebitmap_set_bit(&mpdb->global->branch_list->declared.scope[SYM_TYPES], value - 1, 1)) {
		return 1;
	}

	mpdb->type_attr_map = xrealloc(mpdb->type_attr_map, sizeof(ebitmap_t) * mpdb->p_types.nprim);
	mpdb->attr_type_map = xrealloc(mpdb->attr_type_map, sizeof(ebitmap_t) * mpdb->p_types.nprim);
	ebitmap_init(&mpdb->type_attr_map[value-1]);
	ebitmap_init(&mpdb->attr_type_map[value-1]);
	ebitmap_set_bit(&mpdb->type_attr_map[value-1], value-1, 1);

	src = hashtab_search(mpdb->p_types.table, d);
	if(!src)
		return 1;

	if(policydb_index_decls(NULL, mpdb))
		return 1;

	if(policydb_index_classes(mpdb))
		return 1;

	if(policydb_index_others(NULL, mpdb, 0))
		return 1;

	//Add the domain to all roles
	for(unsigned i = 0; i < mpdb->p_roles.nprim; ++i) {
		//Not sure all those three calls are needed
		ebitmap_set_bit(&mpdb->role_val_to_struct[i]->types.negset, value - 1, 0);
		ebitmap_set_bit(&mpdb->role_val_to_struct[i]->types.types, value - 1, 1);
		type_set_expand(&mpdb->role_val_to_struct[i]->types, &mpdb->role_val_to_struct[i]->cache, mpdb, 0);
	}

	return set_attr("domain", value);
}

int set_domain_state(const char *s, int state) {
	type_datum_t *type;
	if (s == NULL) {
		hashtab_for_each(mpdb->p_types.table, {
			type = node->datum;
			if (ebitmap_set_bit(&mpdb->permissive_map, type->s.value, state)) {
				LOGW("Could not set bit in permissive map\n");
				return 1;
			}
		})
	} else {
		type = hashtab_search(mpdb->p_types.table, s);
		if (type == NULL) {
			LOGW("type %s does not exist\n", s);
			return 1;
		}
		if (ebitmap_set_bit(&mpdb->permissive_map, type->s.value, state)) {
			LOGW("Could not set bit in permissive map\n");
			return 1;
		}
	}

	return 0;
}

int add_filename_trans(const char *s, const char *t, const char *c, const char *d, const char *o) {
	type_datum_t *src, *tgt, *def;
	class_datum_t *cls;

	src = hashtab_search(mpdb->p_types.table, s);
	if (src == NULL) {
		LOGW("source type %s does not exist\n", s);
		return 1;
	}
	tgt = hashtab_search(mpdb->p_types.table, t);
	if (tgt == NULL) {
		LOGW("target type %s does not exist\n", t);
		return 1;
	}
	cls = hashtab_search(mpdb->p_classes.table, c);
	if (cls == NULL) {
		LOGW("class %s does not exist\n", c);
		return 1;
	}
	def = hashtab_search(mpdb->p_types.table, d);
	if (def == NULL) {
		LOGW("default type %s does not exist\n", d);
		return 1;
	}

	filename_trans_t trans_key;
	trans_key.stype = src->s.value;
	trans_key.ttype = tgt->s.value;
	trans_key.tclass = cls->s.value;
	trans_key.name = (char *) o;

	filename_trans_datum_t *trans_datum;
	trans_datum = hashtab_search(mpdb->filename_trans, (hashtab_key_t) &trans_key);

	if (trans_datum == NULL) {
		trans_datum = xcalloc(sizeof(*trans_datum), 1);
		hashtab_insert(mpdb->filename_trans, (hashtab_key_t) &trans_key, trans_datum);
	}

	// Overwrite existing
	trans_datum->otype = def->s.value;
	return 0;
}

int add_typeattribute(const char *type, const char *attr) {
	type_datum_t *domain = hashtab_search(mpdb->p_types.table, type);
	if (domain == NULL) {
		LOGW("type %s does not exist\n", type);
		return 1;
	}

	int attr_id = set_attr(attr, domain->s.value);
	if (attr_id < 0)
		return 1;

	hashtab_for_each(mpdb->p_classes.table, {
		class_datum_t *cls = node->datum;
		for (constraint_node_t *n = cls->constraints; n ; n = n->next) {
			for (constraint_expr_t *e = n->expr; e; e = e->next) {
				if (e->expr_type == CEXPR_NAMES &&
					ebitmap_get_bit(&e->type_names->types, attr_id - 1)) {
					ebitmap_set_bit(&e->names, domain->s.value - 1, 1);
				}
			}
		}
	})

	return 0;
}

int add_rule(const char *s, const char *t, const char *c, const char *p, int effect, int n) {
	type_datum_t *src = NULL, *tgt = NULL;
	class_datum_t *cls = NULL;
	perm_datum_t *perm = NULL;

	if (s) {
		src = hashtab_search(mpdb->p_types.table, s);
		if (src == NULL) {
			LOGW("source type %s does not exist\n", s);
			return 1;
		}
	}

	if (t) {
		tgt = hashtab_search(mpdb->p_types.table, t);
		if (tgt == NULL) {
			LOGW("target type %s does not exist\n", t);
			return 1;
		}
	}

	if (c) {
		cls = hashtab_search(mpdb->p_classes.table, c);
		if (cls == NULL) {
			LOGW("class %s does not exist\n", c);
			return 1;
		}
	}

	if (p) {
		if (c == NULL) {
			LOGW("No class is specified, cannot add perm [%s] \n", p);
			return 1;
		}

		perm = hashtab_search(cls->permissions.table, p);
		if (perm == NULL && cls->comdatum != NULL) {
			perm = hashtab_search(cls->comdatum->permissions.table, p);
		}
		if (perm == NULL) {
			LOGW("perm %s does not exist in class %s\n", p, c);
			return 1;
		}
	}
	return add_rule_auto(src, tgt, cls, perm, effect, n);
}

int add_xperm_rule(const char *s, const char *t, const char *c, const char *range, int effect,
				   int n) {
	type_datum_t *src = NULL, *tgt = NULL;
	class_datum_t *cls = NULL;

	if (s) {
		src = hashtab_search(mpdb->p_types.table, s);
		if (src == NULL) {
			LOGW("source type %s does not exist\n", s);
			return 1;
		}
	}

	if (t) {
		tgt = hashtab_search(mpdb->p_types.table, t);
		if (tgt == NULL) {
			LOGW("target type %s does not exist\n", t);
			return 1;
		}
	}

	if (c) {
		cls = hashtab_search(mpdb->p_classes.table, c);
		if (cls == NULL) {
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

	return add_xperm_rule_auto(src, tgt, cls, low, high, effect, n);
}

int add_type_rule(const char *s, const char *t, const char *c, const char *d, int effect) {
	type_datum_t *src, *tgt, *def;
	class_datum_t *cls;

	src = hashtab_search(mpdb->p_types.table, s);
	if (src == NULL) {
		LOGW("source type %s does not exist\n", s);
		return 1;
	}
	tgt = hashtab_search(mpdb->p_types.table, t);
	if (tgt == NULL) {
		LOGW("target type %s does not exist\n", t);
		return 1;
	}
	cls = hashtab_search(mpdb->p_classes.table, c);
	if (cls == NULL) {
		LOGW("class %s does not exist\n", c);
		return 1;
	}
	def = hashtab_search(mpdb->p_types.table, d);
	if (def == NULL) {
		LOGW("default type %s does not exist\n", d);
		return 1;
	}

	avtab_key_t key;
	key.source_type = src->s.value;
	key.target_type = tgt->s.value;
	key.target_class = cls->s.value;
	key.specified = effect;

	avtab_ptr_t node = get_avtab_node(&key, NULL);
	node->datum.data = def->s.value;

	return 0;
}

int add_genfscon(const char *name, const char *path, const char *context) {
	// First try to create context
	context_struct_t *ctx;
	if (context_from_string(NULL, mpdb, &ctx, context, strlen(context))) {
		LOGW("Failed to create context from string [%s]\n", context);
		return 1;
	}

	// Allocate genfs context
	ocontext_t *newc = xcalloc(sizeof(*newc), 1);
	newc->u.name = strdup(path);
	memcpy(&newc->context[0], ctx, sizeof(*ctx));
	free(ctx);

	// Find or allocate genfs
	genfs_t *last_gen = NULL;
	genfs_t *newfs = NULL;
	for (genfs_t *node = mpdb->genfs; node; node = node->next) {
		if (strcmp(node->fstype, name) == 0) {
			newfs = node;
			break;
		}
		last_gen = node;
	}
	if (newfs == NULL) {
		newfs = xcalloc(sizeof(*newfs), 1);
		newfs->fstype = strdup(name);
		// Insert
		if (last_gen)
			last_gen->next = newfs;
		else
			mpdb->genfs = newfs;
	}

	// Insert or replace genfs context
	ocontext_t *last_ctx = NULL;
	for (ocontext_t *node = newfs->head; node; node = node->next) {
		if (strcmp(node->u.name, path) == 0) {
			// Unlink
			if (last_ctx)
				last_ctx->next = node->next;
			else
				newfs->head = NULL;
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

void strip_dontaudit() {
	avtab_for_each(&mpdb->te_avtab, {
		if (node->key.specified == AVTAB_AUDITDENY || node->key.specified == AVTAB_XPERMS_DONTAUDIT)
			avtab_remove_node(&magisk_policydb->te_avtab, node);
	})
}
