/*
 * This was derived from public domain works with updates to
 * work with more modern SELinux libraries.
 *
 * It is released into the public domain.
 *
 */

#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sepol/debug.h>
#include <sepol/policydb/policydb.h>
#include <sepol/policydb/expand.h>
#include <sepol/policydb/link.h>
#include <sepol/policydb/services.h>
#include <sepol/policydb/avrule_block.h>
#include <sepol/policydb/conditional.h>
#include <sepol/policydb/constraint.h>

void usage(char *arg0) {
	fprintf(stderr, "%s -s <source type> -t <target type> -c <class> -p <perm_list> -P <policy file>\n", arg0);
	fprintf(stderr, "\tInject a rule\n\n");
	fprintf(stderr, "%s -s <source type> -a <type_attribute> -P <policy file>\n", arg0);
	fprintf(stderr, "\tAdd a type_attribute to a domain\n\n");
	fprintf(stderr, "%s -Z <source type> -P <policy file>\n", arg0);
	fprintf(stderr, "\tInject a permissive domain\n\n");
	fprintf(stderr, "%s -z <source type> -P <policy file>\n", arg0);
	fprintf(stderr, "\tInject a non-permissive domain\n\n");
	fprintf(stderr, "%s -e -s <source type> -P <policy file>\n", arg0);
	fprintf(stderr, "\tCheck if a SELinux type exists\n\n");
	fprintf(stderr, "%s -e -c <class> -P <policy file>\n", arg0);
	fprintf(stderr, "\tCheck if a SELinux class exists\n\n");
	fprintf(stderr, "All options can add -o <output file> to output to another file\n");
	exit(1);
}

void *cmalloc(size_t s) {
	void *t = malloc(s);
	if (t == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	return t;
}

int get_attr(char *type, int value, policydb_t *policy) {
	type_datum_t *attr = hashtab_search(policy->p_types.table, type);
	if (!attr)
		exit(1);

	if (attr->flavor != TYPE_ATTRIB)
		exit(1);

	return !! ebitmap_get_bit(&policy->attr_type_map[attr->s.value-1], value-1);
	//return !! ebitmap_get_bit(&policy->type_attr_map[value-1], attr->s.value-1);
}

int get_attr_id(char *type, policydb_t *policy) {
	type_datum_t *attr = hashtab_search(policy->p_types.table, type);
	if (!attr)
		exit(1);

	if (attr->flavor != TYPE_ATTRIB)
		exit(1);

	return attr->s.value;
}

int set_attr(char *type, int value, policydb_t *policy) {
	type_datum_t *attr = hashtab_search(policy->p_types.table, type);
	if (!attr)
		exit(1);

	if (attr->flavor != TYPE_ATTRIB)
		exit(1);

	if(ebitmap_set_bit(&policy->type_attr_map[value-1], attr->s.value-1, 1))
		exit(1);
	if(ebitmap_set_bit(&policy->attr_type_map[attr->s.value-1], value-1, 1))
		exit(1);

	return 0;
}

void create_domain(char *d, policydb_t *policy) {
	symtab_datum_t *src = hashtab_search(policy->p_types.table, d);
	if(src)
		return;

	type_datum_t *typdatum = (type_datum_t *) malloc(sizeof(type_datum_t));
	type_datum_init(typdatum);
	typdatum->primary = 1;
	typdatum->flavor = TYPE_TYPE;

	uint32_t value = 0;
	int r = symtab_insert(policy, SYM_TYPES, strdup(d), typdatum, SCOPE_DECL, 1, &value);
	typdatum->s.value = value;

	fprintf(stderr, "source type %s does not exist: %d,%d\n", d, r, value);
	if (ebitmap_set_bit(&policy->global->branch_list->declared.scope[SYM_TYPES], value - 1, 1)) {
		exit(1);
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
		exit(1);

	extern int policydb_index_decls(policydb_t * p);
	if(policydb_index_decls(policy))
		exit(1);

	if(policydb_index_classes(policy))
		exit(1);

	if(policydb_index_others(NULL, policy, 1))
		exit(1);

	set_attr("domain", value, policy);
}

int add_irule(int s, int t, int c, int p, int effect, int not, policydb_t* policy) {
	avtab_datum_t *av;
	avtab_key_t key;

	key.source_type = s;
	key.target_type = t;
	key.target_class = c;
	key.specified = effect;
	av = avtab_search(&policy->te_avtab, &key);

	if (av == NULL) {
		av = cmalloc(sizeof(*av));
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

int add_rule(char *s, char *t, char *c, char *p, int effect, int not, policydb_t *policy) {
	type_datum_t *src, *tgt;
	class_datum_t *cls;
	perm_datum_t *perm;

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
	perm = hashtab_search(cls->permissions.table, p);
	if (perm == NULL) {
		if (cls->comdatum == NULL) {
			fprintf(stderr, "perm %s does not exist in class %s\n", p, c);
			return 1;
		}
		perm = hashtab_search(cls->comdatum->permissions.table, p);
		if (perm == NULL) {
			fprintf(stderr, "perm %s does not exist in class %s\n", p, c);
			return 1;
		}
	}

	return add_irule(src->s.value, tgt->s.value, cls->s.value, perm->s.value, effect, not, policy);
}

int add_typerule(char *s, char *targetAttribute, char **minusses, char *c, char *p, int effect, int not, policydb_t *policy) {
	type_datum_t *src, *tgt;
	class_datum_t *cls;
	perm_datum_t *perm;

	//64(0kB) should be enough for everyone, right?
	int m[64] = { -1 };

	src = hashtab_search(policy->p_types.table, s);
	if (src == NULL) {
		fprintf(stderr, "source type %s does not exist\n", s);
		return 1;
	}

	tgt = hashtab_search(policy->p_types.table, targetAttribute);
	if (tgt == NULL) {
		fprintf(stderr, "target type %s does not exist\n", targetAttribute);
		return 1;
	}
	if(tgt->flavor != TYPE_ATTRIB)
		exit(1);

	for(int i=0; minusses && minusses[i]; ++i) {
		type_datum_t *obj;
		obj = hashtab_search(policy->p_types.table, minusses[i]);
		if (obj == NULL) {
			fprintf(stderr, "minus type %s does not exist\n", minusses[i]);
			return 1;
		}
		m[i] = obj->s.value-1;
		m[i+1] = -1;
	}

	cls = hashtab_search(policy->p_classes.table, c);
	if (cls == NULL) {
		fprintf(stderr, "class %s does not exist\n", c);
		return 1;
	}

	perm = hashtab_search(cls->permissions.table, p);
	if (perm == NULL) {
		if (cls->comdatum == NULL) {
			fprintf(stderr, "perm %s does not exist in class %s\n", p, c);
			return 1;
		}
		perm = hashtab_search(cls->comdatum->permissions.table, p);
		if (perm == NULL) {
			fprintf(stderr, "perm %s does not exist in class %s\n", p, c);
			return 1;
		}
	}

	ebitmap_node_t *node;
	int i;

	int ret = 0;

	ebitmap_for_each_bit(&policy->attr_type_map[tgt->s.value-1], node, i) {
		if(ebitmap_node_get_bit(node, i)) {
			int found = 0;
			for(int j=0; m[j] != -1; ++j) {
				if(i == m[j])
					found = 1;
			}

			if(!found)
				ret |= add_irule(src->s.value, i+1, cls->s.value, perm->s.value, effect, not, policy);
		}
	}
	return ret;
}

int add_transition(char *srcS, char *origS, char *tgtS, char *c, policydb_t *policy) {
	type_datum_t *src, *tgt, *orig;
	class_datum_t *cls;

	avtab_datum_t *av;
	avtab_key_t key;

	src = hashtab_search(policy->p_types.table, srcS);
	if (src == NULL) {
		fprintf(stderr, "source type %s does not exist\n", srcS);
		return 1;
	}
	tgt = hashtab_search(policy->p_types.table, tgtS);
	if (tgt == NULL) {
		fprintf(stderr, "target type %s does not exist\n", tgtS);
		return 1;
	}
	cls = hashtab_search(policy->p_classes.table, c);
	if (cls == NULL) {
		fprintf(stderr, "class %s does not exist\n", c);
		return 1;
	}
	orig = hashtab_search(policy->p_types.table, origS);
	if (cls == NULL) {
		fprintf(stderr, "class %s does not exist\n", origS);
		return 1;
	}

	key.source_type = src->s.value;
	key.target_type = orig->s.value;
	key.target_class = cls->s.value;
	key.specified = AVTAB_TRANSITION;
	av = avtab_search(&policy->te_avtab, &key);

	if (av == NULL) {
		av = cmalloc(sizeof(*av));
		av->data = tgt->s.value;
		int ret = avtab_insert(&policy->te_avtab, &key, av);
		if (ret) {
			fprintf(stderr, "Error inserting into avtab\n");
			return 1;
		}
	} else {
		fprintf(stderr, "Warning, rule already defined! Won't override.\n");
		fprintf(stderr, "Previous value = %d, wanted value = %d\n", av->data, tgt->s.value);
	}

	return 0;
}

int add_file_transition(char *srcS, char *origS, char *tgtS, char *c, char* filename, policydb_t *policy) {
	type_datum_t *src, *tgt, *orig;
	class_datum_t *cls;

	src = hashtab_search(policy->p_types.table, srcS);
	if (src == NULL) {
		fprintf(stderr, "source type %s does not exist\n", srcS);
		return 1;
	}
	tgt = hashtab_search(policy->p_types.table, tgtS);
	if (tgt == NULL) {
		fprintf(stderr, "target type %s does not exist\n", tgtS);
		return 1;
	}
	cls = hashtab_search(policy->p_classes.table, c);
	if (cls == NULL) {
		fprintf(stderr, "class %s does not exist\n", c);
		return 1;
	}
	orig = hashtab_search(policy->p_types.table, origS);
	if (cls == NULL) {
		fprintf(stderr, "class %s does not exist\n", origS);
		return 1;
	}

	filename_trans_t *new_transition = cmalloc(sizeof(*new_transition));
	new_transition->stype = src->s.value;
	new_transition->ttype = orig->s.value;
	new_transition->tclass = cls->s.value;
	new_transition->otype = tgt->s.value;
	new_transition->name = strdup(filename);
	new_transition->next = policy->filename_trans;

	policy->filename_trans = new_transition;

	return 0;
}

int add_type(char *domainS, char *typeS, policydb_t *policy) {
	type_datum_t *domain;

	domain = hashtab_search(policy->p_types.table, domainS);
	if (domain == NULL) {
		fprintf(stderr, "source type %s does not exist\n", domainS);
		return 1;
	}

	set_attr(typeS, domain->s.value, policy);

	int typeId = get_attr_id(typeS, policy);
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
	ret = policydb_read(policydb, pf, 1);
	if (ret) {
		fprintf(stderr, "error(s) encountered while parsing configuration\n");
		return 1;
	}

	return 0;
}

int auto_allow(type_datum_t *src, type_datum_t *tgt, class_datum_t *cls, policydb_t *policy) {
	perm_datum_t *perm;
	hashtab_t type_table, class_table, perm_table;
	hashtab_ptr_t cur;
	
	type_table = policy->p_types.table;
	class_table = policy->p_classes.table;

	if (src == NULL) {
		for (int i = 0; i < type_table->size; ++i) {
			cur = type_table->htable[i];
			while (cur != NULL) {
				src = cur->datum;
				if(auto_allow(src, tgt, cls, policy))
					return 1;
				cur = cur->next;
			}
		}
	} else if (tgt == NULL) {
		for (int i = 0; i < type_table->size; ++i) {
			cur = type_table->htable[i];
			while (cur != NULL) {
				tgt = cur->datum;
				if(auto_allow(src, tgt, cls, policy))
					return 1;
				cur = cur->next;
			}
		}
	} else if (cls == NULL) {
		for (int i = 0; i < class_table->size; ++i) {
			cur = class_table->htable[i];
			while (cur != NULL) {
				cls = cur->datum;
				if(auto_allow(src, tgt, cls, policy))
					return 1;
				cur = cur->next;
			}
		}
	} else {
		perm_table = cls->permissions.table;
		for (int i = 0; i < perm_table->size; ++i) {
			cur = perm_table->htable[i];
			while (cur != NULL) {
				perm = cur->datum;
				if(add_irule(src->s.value, tgt->s.value, cls->s.value, perm->s.value, AVTAB_ALLOWED, 0, policy))
					return 1;
				cur = cur->next;
			}
		}

		if (cls->comdatum != NULL) {
			perm_table = cls->comdatum->permissions.table;
			for (int i = 0; i < perm_table->size; ++i) {
				cur = perm_table->htable[i];
				while (cur != NULL) {
					perm = cur->datum;
					if(add_irule(src->s.value, tgt->s.value, cls->s.value, perm->s.value, AVTAB_ALLOWED, 0, policy))
						return 1;
					cur = cur->next;
				}
			}
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	char *policy = NULL, *source = NULL, *target = NULL, *class = NULL, *perm = NULL;
	char *fcon = NULL, *outfile = NULL, *permissive = NULL, *attr = NULL, *filetrans = NULL;
	int exists = 0, not = 0, autoAllow = 0;
	policydb_t policydb;
	struct policy_file pf, outpf;
	sidtab_t sidtab;
	int ch;
	FILE *fp;
	int permissive_value = 0, noaudit = 0;

	struct option long_options[] = {
		{"attr", required_argument, NULL, 'a'},
		{"exists", no_argument, NULL, 'e'},
		{"source", required_argument, NULL, 's'},
		{"target", required_argument, NULL, 't'},
		{"class", required_argument, NULL, 'c'},
		{"perm", required_argument, NULL, 'p'},
		{"fcon", required_argument, NULL, 'f'},
		{"filetransition", required_argument, NULL, 'g'},
		{"noaudit", no_argument, NULL, 'n'},
		{"policy", required_argument, NULL, 'P'},
		{"output", required_argument, NULL, 'o'},
		{"permissive", required_argument, NULL, 'Z'},
		{"not-permissive", required_argument, NULL, 'z'},
		{"not", no_argument, NULL, 0},
		{"auto", no_argument, NULL, 0},
		{NULL, 0, NULL, 0}
	};

	int option_index = -1;
	while ((ch = getopt_long(argc, argv, "a:c:ef:g:s:t:p:P:o:Z:z:n", long_options, &option_index)) != -1) {
		switch (ch) {
			case 0:
				if(strcmp(long_options[option_index].name, "not") == 0)
					not = 1;
				else if(strcmp(long_options[option_index].name, "auto") == 0)
					autoAllow = 1;
				else
					usage(argv[0]);
				break;
			case 'a':
				attr = optarg;
				break;
			case 'e':
				exists = 1;
				break;
			case 'f':
				fcon = optarg;
				break;
			case 'g':
				filetrans = optarg;
				break;
			case 's':
				source = optarg;
				break;
			case 't':
				target = optarg;
				break;
			case 'c':
				class = optarg;
				break;
			case 'p':
				perm = optarg;
				break;
			case 'P':
				policy = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'Z':
				permissive = optarg;
				permissive_value = 1;
				break;
			case 'z':
				permissive = optarg;
				permissive_value = 0;
				break;
			case 'n':
				noaudit = 1;
				break;
			default:
				usage(argv[0]);
			}
	}

	if (((!source || !target || !class || !perm) && !permissive && !fcon && !attr &&!filetrans && !exists && !auto_allow) || !policy)
		usage(argv[0]);

	if(!outfile)
		outfile = policy;

	sepol_set_policydb(&policydb);
	sepol_set_sidtab(&sidtab);

	if (load_policy(policy, &policydb, &pf)) {
		fprintf(stderr, "Could not load policy\n");
		return 1;
	}

	if (policydb_load_isids(&policydb, &sidtab))
		return 1;

	if (autoAllow) {
		type_datum_t *src = NULL, *tgt = NULL;
		class_datum_t *cls = NULL;
		if (source)
			src = hashtab_search(policydb.p_types.table, source);
		if (target)
			tgt = hashtab_search(policydb.p_types.table, target);
		if (class)
			cls = hashtab_search(policydb.p_classes.table, class);
		if (auto_allow(src, tgt, cls, &policydb))
			return 1;
	} else if (permissive) {
		type_datum_t *type;
		create_domain(permissive, &policydb);
		type = hashtab_search(policydb.p_types.table, permissive);
		if (type == NULL) {
				fprintf(stderr, "type %s does not exist\n", permissive);
				return 1;
		}
		if (ebitmap_set_bit(&policydb.permissive_map, type->s.value, permissive_value)) {
			fprintf(stderr, "Could not set bit in permissive map\n");
			return 1;
		}
	} else if(exists) {
		if(source) {
			type_datum_t *tmp = hashtab_search(policydb.p_types.table, source);
			if (!tmp)
				exit(1);
			else
				exit(0);
		} else if(class) {
			class_datum_t *tmp = hashtab_search(policydb.p_classes.table, class);
			if(!tmp)
				exit(1);
			else
				exit(0);
		} else {
			usage(argv[0]);
		}
	} else if(filetrans) {
		if(add_file_transition(source, fcon, target, class, filetrans, &policydb))
			return 1;
	} else if(fcon) {
		if(add_transition(source, fcon, target, class, &policydb))
			return 1;
	} else if(attr) {
		if(add_type(source, attr, &policydb))
			return 1;
	} else if(noaudit) {
		if(add_rule(source, target, class, perm, AVTAB_AUDITDENY, not, &policydb))
			return 1;
	} else {
		//Add a rule to a whole set of typeattribute, not just a type
		if(*target == '=') {
			char *saveptr = NULL;

			char *targetAttribute = strtok_r(target, "-", &saveptr);

			char *vals[64];
			int i = 0;

			char *m = NULL;
			while( (m = strtok_r(NULL, "-", &saveptr)) != NULL) {
				vals[i++] = m;
			}
			vals[i] = NULL;

			if(add_typerule(source, targetAttribute+1, vals, class, perm, AVTAB_ALLOWED, not, &policydb))
				return 1;
		} else {
			char *saveptr = NULL;

			char *p = strtok_r(perm, ",", &saveptr);
			do {
				if (add_rule(source, target, class, p, AVTAB_ALLOWED, not, &policydb)) {
					fprintf(stderr, "Could not add rule\n");
					return 1;
				}
			} while( (p = strtok_r(NULL, ",", &saveptr)) != NULL);
		}
	}

	fp = fopen(outfile, "w");
	if (!fp) {
		fprintf(stderr, "Could not open outfile\n");
		return 1;
	}

	policy_file_init(&outpf);
	outpf.type = PF_USE_STDIO;
	outpf.fp = fp;

	if (policydb_write(&policydb, &outpf)) {
		fprintf(stderr, "Could not write policy\n");
		return 1;
	}

	policydb_destroy(&policydb);
	fclose(fp);

	return 0;
}
