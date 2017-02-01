#include "sepolicy-inject.h"

static void usage(char *arg0) {
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

int main(int argc, char **argv) {
	char *infile = NULL, *source = NULL, *target = NULL, *class = NULL, *perm = NULL;
	char *fcon = NULL, *outfile = NULL, *permissive = NULL, *attr = NULL, *filetrans = NULL;
	int exists = 0, not = 0, live = 0, builtin = 0, minimal = 0;
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
		{"file", required_argument, NULL, 'P'},
		{"output", required_argument, NULL, 'o'},
		{"permissive", required_argument, NULL, 'Z'},
		{"not-permissive", required_argument, NULL, 'z'},
		{"not", no_argument, NULL, 0},
		{"live", no_argument, NULL, 0},
		{"minimal", no_argument, NULL, 0},
		{NULL, 0, NULL, 0}
	};

	int option_index = -1;
	while ((ch = getopt_long(argc, argv, "a:c:ef:g:s:t:p:P:o:Z:z:n", long_options, &option_index)) != -1) {
		switch (ch) {
			case 0:
				if(strcmp(long_options[option_index].name, "not") == 0)
					not = 1;
				else if(strcmp(long_options[option_index].name, "live") == 0)
					live = 1;
				else if(strcmp(long_options[option_index].name, "minimal") == 0)
					minimal = 1;
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
				infile = optarg;
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

	// Use builtin rules if nothing specified
	if (!minimal && !source && !target && !class && !perm && !permissive && !fcon && !attr &&!filetrans && !exists)
		builtin = 1;

	// Overwrite original if not specified
	if(!outfile)
		outfile = infile;

	// Use current policy if not specified
	if(!infile)
		infile = "/sys/fs/selinux/policy";

	sepol_set_policydb(&policydb);
	sepol_set_sidtab(&sidtab);

	if (load_policy(infile, &policydb, &pf)) {
		fprintf(stderr, "Could not load policy\n");
		return 1;
	}

	if (policydb_load_isids(&policydb, &sidtab))
		return 1;

	policy = &policydb;

	if (builtin) {
		su_rules();
	}
	else if (minimal) {
		min_rules();
	}
	else if (permissive) {
		type_datum_t *type;
		create_domain(permissive);
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
		if(add_file_transition(source, fcon, target, class, filetrans))
			return 1;
	} else if(fcon) {
		if(add_transition(source, fcon, target, class))
			return 1;
	} else if(attr) {
		if(add_type(source, attr))
			return 1;
	} else if(noaudit) {
		if(add_rule(source, target, class, perm, AVTAB_AUDITDENY, not))
			return 1;
	} else {
		//Add a rule to a whole set of typeattribute, not just a type
		if (target != NULL) {
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

				if(add_typerule(source, targetAttribute+1, vals, class, perm, AVTAB_ALLOWED, not))
					return 1;
			}
		}
		if (perm != NULL) {
			char *saveptr = NULL;

			char *p = strtok_r(perm, ",", &saveptr);
			do {
				if (add_rule(source, target, class, p, AVTAB_ALLOWED, not)) {
					fprintf(stderr, "Could not add rule\n");
					return 1;
				}
			} while( (p = strtok_r(NULL, ",", &saveptr)) != NULL);
		} else {
			if (add_rule(source, target, class, perm, AVTAB_ALLOWED, not)) {
				fprintf(stderr, "Could not add rule\n");
				return 1;
			}
		}
	}

	if (live) {
		if (live_patch()) {
			fprintf(stderr, "Could not load new policy into kernel\n");
			return 1;
		}
	}
	
	if (outfile) {
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
		fclose(fp);
	}

	policydb_destroy(&policydb);
	return 0;
}
