#include "sepolicy-inject.h"

static void *cmalloc(size_t s) {
	void *t = malloc(s);
	if (t == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	return t;
}

static int get_attr(char *type, int value) {
	type_datum_t *attr = hashtab_search(policy->p_types.table, type);
	if (!attr)
		return 1;

	if (attr->flavor != TYPE_ATTRIB)
		return 1;

	return !! ebitmap_get_bit(&policy->attr_type_map[attr->s.value-1], value-1);
	//return !! ebitmap_get_bit(&policy->type_attr_map[value-1], attr->s.value-1);
}

static int get_attr_id(char *type) {
	type_datum_t *attr = hashtab_search(policy->p_types.table, type);
	if (!attr)
		return 1;

	if (attr->flavor != TYPE_ATTRIB)
		return 1;

	return attr->s.value;
}

static int set_attr(char *type, int value) {
	type_datum_t *attr = hashtab_search(policy->p_types.table, type);
	if (!attr)
		return 1;

	if (attr->flavor != TYPE_ATTRIB)
		return 1;

	if(ebitmap_set_bit(&policy->type_attr_map[value-1], attr->s.value-1, 1))
		return 1;
	if(ebitmap_set_bit(&policy->attr_type_map[attr->s.value-1], value-1, 1))
		return 1;

	return 0;
}

static int add_irule(int s, int t, int c, int p, int effect, int not) {
	avtab_datum_t *av;
	avtab_key_t key;

	key.source_type = s;
	key.target_type = t;
	key.target_class = c;
	key.specified = effect;
	av = avtab_search(&policy->te_avtab, &key);

	if (av == NULL) {
		av = cmalloc(sizeof(*av));
		memset(av, 0, sizeof(*av));
		av->data |= 1U << (p - 1);
		int ret = avtab_insert(&policy->te_avtab, &key, av);
		if (ret) {
			fprintf(stderr, "Error inserting into avtab\n");
			return 1;
		}
	}

	if(not)
		av->data &= ~(1U << (p - 1));
	else
		av->data |= 1U << (p - 1);
	return 0;
}

static int add_rule_auto(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, perm_datum_t *perm, int effect, int not) {
	hashtab_ptr_t cur;
	int ret = 0;

	if (src == NULL) {
		hashtab_for_each(policy->p_types.table, &cur) {
			src = cur->datum;
			if(add_rule_auto(src, tgt, cls, perm, effect, not))
				return 1;
		}
	} else if (tgt == NULL) {
		hashtab_for_each(policy->p_types.table, &cur) {
			tgt = cur->datum;
			if(add_rule_auto(src, tgt, cls, perm, effect, not))
				return 1;
		}
	} else if (cls == NULL) {
		hashtab_for_each(policy->p_classes.table, &cur) {
			cls = cur->datum;
			if(add_rule_auto(src, tgt, cls, perm, effect, not))
				return 1;
		}
	} else if (perm == NULL) {
		hashtab_for_each(cls->permissions.table, &cur) {
			perm = cur->datum;
			if(add_rule_auto(src, tgt, cls, perm, effect, not))
				return 1;
		}

		if (cls->comdatum != NULL) {
			hashtab_for_each(cls->comdatum->permissions.table, &cur) {
				perm = cur->datum;
				if(add_rule_auto(src, tgt, cls, perm, effect, not))
					return 1;
			}
		}
	} else {
		ebitmap_node_t *s_node, *t_node;
		int i, j;
		if (src->flavor == TYPE_ATTRIB) {
			if (tgt->flavor == TYPE_ATTRIB) {
				ebitmap_for_each_bit(&policy->attr_type_map[src->s.value-1], s_node, i) 
					ebitmap_for_each_bit(&policy->attr_type_map[tgt->s.value-1], t_node, j)
						if(ebitmap_node_get_bit(s_node, i) && ebitmap_node_get_bit(t_node, j))
							ret |= add_irule(i + 1, j + 1, cls->s.value, perm->s.value, effect, not);
					
			} else {
				ebitmap_for_each_bit(&policy->attr_type_map[src->s.value-1], s_node, i)
					if(ebitmap_node_get_bit(s_node, i))
						ret |= add_irule(i + 1, tgt->s.value, cls->s.value, perm->s.value, effect, not);
			}
		} else if (tgt->flavor == TYPE_ATTRIB) {
			ebitmap_for_each_bit(&policy->attr_type_map[tgt->s.value-1], t_node, j)
				if(ebitmap_node_get_bit(t_node, j))
					ret |= add_irule(src->s.value, j + 1, cls->s.value, perm->s.value, effect, not);
		} else
			ret = add_irule(src->s.value, tgt->s.value, cls->s.value, perm->s.value, effect, not);
	}
	return ret;
}

int load_policy(char *filename, policydb_t *policydb, struct policy_file *pf) {
	int fd;
	struct stat sb;
	void *map;
	int ret;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Can't open '%s':  %s\n",
				filename, strerror(errno));
		return 1;
	}
	if (fstat(fd, &sb) < 0) {
		fprintf(stderr, "Can't stat '%s':  %s\n",
				filename, strerror(errno));
		return 1;
	}
	map = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
				fd, 0);
	if (map == MAP_FAILED) {
		fprintf(stderr, "Can't mmap '%s':  %s\n",
				filename, strerror(errno));
		return 1;
	}

	policy_file_init(pf);
	pf->type = PF_USE_MEMORY;
	pf->data = map;
	pf->len = sb.st_size;
	if (policydb_init(policydb)) {
		fprintf(stderr, "policydb_init: Out of memory!\n");
		return 1;
	}
	ret = policydb_read(policydb, pf, 0);
	if (ret) {
		fprintf(stderr, "error(s) encountered while parsing configuration\n");
		return 1;
	}

	return 0;
}

int create_domain(char *d) {
	symtab_datum_t *src = hashtab_search(policy->p_types.table, d);
	if(src)
		return 1;

	type_datum_t *typdatum = (type_datum_t *) malloc(sizeof(type_datum_t));
	type_datum_init(typdatum);
	typdatum->primary = 1;
	typdatum->flavor = TYPE_TYPE;

	uint32_t value = 0;
	int r = symtab_insert(policy, SYM_TYPES, strdup(d), typdatum, SCOPE_DECL, 1, &value);
	typdatum->s.value = value;

	if (ebitmap_set_bit(&policy->global->branch_list->declared.scope[SYM_TYPES], value - 1, 1)) {
		return 1;
	}

	policy->type_attr_map = realloc(policy->type_attr_map, sizeof(ebitmap_t)*policy->p_types.nprim);
	policy->attr_type_map = realloc(policy->attr_type_map, sizeof(ebitmap_t)*policy->p_types.nprim);
	ebitmap_init(&policy->type_attr_map[value-1]);
	ebitmap_init(&policy->attr_type_map[value-1]);
	ebitmap_set_bit(&policy->type_attr_map[value-1], value-1, 1);

	//Add the domain to all roles
	for(unsigned i=0; i<policy->p_roles.nprim; ++i) {
		//Not sure all those three calls are needed
		ebitmap_set_bit(&policy->role_val_to_struct[i]->types.negset, value-1, 0);
		ebitmap_set_bit(&policy->role_val_to_struct[i]->types.types, value-1, 1);
		type_set_expand(&policy->role_val_to_struct[i]->types, &policy->role_val_to_struct[i]->cache, policy, 0);
	}


	src = hashtab_search(policy->p_types.table, d);
	if(!src)
		return 1;

	extern int policydb_index_decls(policydb_t * p);
	if(policydb_index_decls(policy))
		return 1;

	if(policydb_index_classes(policy))
		return 1;

	if(policydb_index_others(NULL, policy, 0))
		return 1;

	return set_attr("domain", value);
}

int set_domain_state(char* s, int state) {
	type_datum_t *type;
	hashtab_ptr_t cur;
	if (s == NULL) {
		hashtab_for_each(policy->p_types.table, &cur) {
			type = cur->datum;
			if (ebitmap_set_bit(&policy->permissive_map, type->s.value, state)) {
				fprintf(stderr, "Could not set bit in permissive map\n");
				return 1;
			}
		}
	} else {
		type = hashtab_search(policy->p_types.table, s);
		if (type == NULL) {
				fprintf(stderr, "type %s does not exist\n", s);
				return 1;
		}
		if (ebitmap_set_bit(&policy->permissive_map, type->s.value, state)) {
			fprintf(stderr, "Could not set bit in permissive map\n");
			return 1;
		}
	}
	
	return 0;
}

int add_transition(char *s, char *t, char *c, char *d) {
	type_datum_t *src, *tgt, *def;
	class_datum_t *cls;

	avtab_datum_t *av;
	avtab_key_t key;

	src = hashtab_search(policy->p_types.table, s);
	if (src == NULL) {
		fprintf(stderr, "source type %s does not exist\n", s);
		return 1;
	}
	tgt = hashtab_search(policy->p_types.table, t);
	if (tgt == NULL) {
		fprintf(stderr, "target type %s does not exist\n", t);
		return 1;
	}
	cls = hashtab_search(policy->p_classes.table, c);
	if (cls == NULL) {
		fprintf(stderr, "class %s does not exist\n", c);
		return 1;
	}
	def = hashtab_search(policy->p_types.table, d);
	if (def == NULL) {
		fprintf(stderr, "default type %s does not exist\n", d);
		return 1;
	}

	key.source_type = src->s.value;
	key.target_type = tgt->s.value;
	key.target_class = cls->s.value;
	key.specified = AVTAB_TRANSITION;
	av = avtab_search(&policy->te_avtab, &key);

	if (av == NULL) {
		av = cmalloc(sizeof(*av));
		av->data = def->s.value;
		int ret = avtab_insert(&policy->te_avtab, &key, av);
		if (ret) {
			fprintf(stderr, "Error inserting into avtab\n");
			return 1;
		}
	} else {
		fprintf(stderr, "Warning, rule already defined! Won't override.\n");
		fprintf(stderr, "Previous value = %d, wanted value = %d\n", av->data, def->s.value);
	}

	return 0;
}

int add_file_transition(char *s, char *t, char *c, char *d, char* filename) {
	type_datum_t *src, *tgt, *def;
	class_datum_t *cls;

	src = hashtab_search(policy->p_types.table, s);
	if (src == NULL) {
		fprintf(stderr, "source type %s does not exist\n", s);
		return 1;
	}
	tgt = hashtab_search(policy->p_types.table, t);
	if (tgt == NULL) {
		fprintf(stderr, "target type %s does not exist\n", t);
		return 1;
	}
	cls = hashtab_search(policy->p_classes.table, c);
	if (cls == NULL) {
		fprintf(stderr, "class %s does not exist\n", c);
		return 1;
	}
	def = hashtab_search(policy->p_types.table, d);
	if (def == NULL) {
		fprintf(stderr, "default type %s does not exist\n", d);
		return 1;
	}

	filename_trans_t *new_transition = cmalloc(sizeof(*new_transition));
	new_transition->stype = src->s.value;
	new_transition->ttype = tgt->s.value;
	new_transition->tclass = cls->s.value;
	new_transition->otype = def->s.value;
	new_transition->name = strdup(filename);
	new_transition->next = policy->filename_trans;

	policy->filename_trans = new_transition;

	return 0;
}

int add_typeattribute(char *domainS, char *attr) {
	type_datum_t *domain;

	domain = hashtab_search(policy->p_types.table, domainS);
	if (domain == NULL) {
		fprintf(stderr, "source type %s does not exist\n", domainS);
		return 1;
	}

	set_attr(attr, domain->s.value);

	int typeId = get_attr_id(attr);
	//Now let's update all constraints!
	//(kernel doesn't support (yet?) type_names rules)
	for(int i=0; i<policy->p_classes.nprim; ++i) {
		class_datum_t *cl = policy->class_val_to_struct[i];
		for(constraint_node_t *n = cl->constraints; n ; n=n->next) {
			for(constraint_expr_t *e = n->expr; e; e=e->next) {
				if(e->expr_type == CEXPR_NAMES) {
					if(ebitmap_get_bit(&e->type_names->types, typeId-1)) {
						ebitmap_set_bit(&e->names, domain->s.value-1, 1);
					}
				}
			}
		}
	}
	return 0;
}

int add_rule(char *s, char *t, char *c, char *p, int effect, int not) {
	type_datum_t *src = NULL, *tgt = NULL;
	class_datum_t *cls = NULL;
	perm_datum_t *perm = NULL;

	if (s) {
		src = hashtab_search(policy->p_types.table, s);
		if (src == NULL) {
			fprintf(stderr, "source type %s does not exist\n", s);
			return 1;
		}
	}

	if (t) {
		tgt = hashtab_search(policy->p_types.table, t);
		if (tgt == NULL) {
			fprintf(stderr, "target type %s does not exist\n", t);
			return 1;
		}
	}

	if (c) {
		cls = hashtab_search(policy->p_classes.table, c);
		if (cls == NULL) {
			fprintf(stderr, "class %s does not exist\n", c);
			return 1;
		}
	}

	if (p) {
		if (c == NULL) {
			fprintf(stderr, "No class is specified, cannot add perm [%s] \n", p);
			return 1;
		}
		
		if (cls != NULL) {
			perm = hashtab_search(cls->permissions.table, p);
			if (perm == NULL && cls->comdatum != NULL) {
				perm = hashtab_search(cls->comdatum->permissions.table, p);
			}
			if (perm == NULL) {
				fprintf(stderr, "perm %s does not exist in class %s\n", p, c);
				return 1;
			}
		}
	}
	return add_rule_auto(src, tgt, cls, perm, effect, not);
}
