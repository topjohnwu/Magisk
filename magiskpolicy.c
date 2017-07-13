/* magiskpolicy.c - Main function for policy patching
 *
 * Includes all the parsing logic for the policy statements
 */

#include "magisk.h"
#include "magiskpolicy.h"
#include "sepolicy.h"

static int syntax_err = 0;
static char err_msg[ARG_MAX];

static void statements() {
	fprintf(stderr, 
		"\nSupported policy statements:\n"
		"\n"
		"\"allow #source-class #target-class permission-class #permission\"\n"
		"\"deny #source-class #target-class permission-class #permission\"\n"
		"\"auditallow #source-class #target-class permission-class #permission\"\n"
		"\"auditdeny #source-class #target-class permission-class #permission\"\n"
		"\"typetrans source-class target-class permission-class default-class (optional: object-name)\"\n"
		"\"allowxperm #source-class #target-class #permission-class ioctl range\"\n"
		"\"auditallowxperm #source-class #target-class #permission-class ioctl range\"\n"
		"\"dontauditxperm #source-class #target-class #permission-class ioctl range\"\n"
		"\"create #class\"\n"
		"\"permissive #class\"\n"
		"\"enforcing #class\"\n"
		"\"attradd #class #attribute\"\n"
		"\nsource-class and target-class can be attributes (patches the whole group)\n"
		"All sections (except typetrans) can be replaced with \'*\' to patch every possible matches\n"
		"Sections marked with \'#\' can be replaced with collections in curly brackets\n"
		"e.g: allow { source1 source2 } { target1 target2 } permission-class { permission1 permission2 }\n"
		"Will be expanded to:\n"
		"allow source1 target1 permission-class permission1\n"
		"allow source1 target1 permission-class permission2\n"
		"allow source1 target2 permission-class permission1\n"
		"allow source1 target2 permission-class permission2\n"
		"allow source2 target1 permission-class permission1\n"
		"allow source2 target1 permission-class permission2\n"
		"allow source2 target2 permission-class permission1\n"
		"allow source2 target2 permission-class permission2\n"
		"\n"
	);
}

static void usage(char *arg0) {
	fprintf(stderr,
		"MagiskPolicy v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu & phh) - SEPolicy Modification Tool\n\n"
		"%s [--options...] [policystatements...]\n\n"
		"Options:\n"
		"  --live: directly load patched policy to device\n"
		// "  --magisk: complete (very large!) patches for Magisk and MagiskSU\n"
		"  --minimal: minimal patches, used for boot image patches\n"
		"  --load <infile>: load policies from <infile>\n"
		"                   (load from current policies if not specified)\n"
		"  --save <outfile>: save policies to <outfile>\n"
		, arg0);
	statements();
	exit(1);
}

// Pattern 1: action { source } { target } class { permission }
static int parse_pattern_1(int action, char* statement) {
	int state = 0, in_bracket = 0;
	char *tok, *class, *saveptr;
	struct vector source, target, permission, *temp;
	vec_init(&source);
	vec_init(&target);
	vec_init(&permission);
	tok = strtok_r(statement, " ", &saveptr);
	while (tok != NULL) {
		if (tok[0] == '{') {
			if (in_bracket || state == 2) return 1;
			in_bracket = 1;
			if (tok[1]) {
				++tok;
				continue;
			}
		} else if (tok[strlen(tok) - 1] == '}') {
			if (!in_bracket || state == 2) return 1;
			in_bracket = 0;
			if (strlen(tok) - 1) {
				tok[strlen(tok) - 1] = '\0';
				continue;
			}
		} else {
			if (tok[0] == '*') tok = ALL;
			switch (state) {
				case 0:
					temp = &source;
					break;
				case 1:
					temp = &target;
					break;
				case 2:
					temp = NULL;
					class = tok;
					break;
				case 3:
					temp = &permission;
					break;
				default:
					return 1;
			}
			vec_push_back(temp, tok);
		}
		if (!in_bracket) ++state;
		tok = strtok_r(NULL, " ", &saveptr);
	}
	if (state != 4) return 1;
	for(int i = 0; i < source.size; ++i)
		for (int j = 0; j < target.size; ++j)
			for (int k = 0; k < permission.size; ++k)
				switch (action) {
					case 0:
						if (sepol_allow(source.data[i], target.data[j], class, permission.data[k]))
							fprintf(stderr, "Error in: allow %s %s %s %s\n", source.data[i], target.data[j], class, permission.data[k]);
						break;
					case 1:
						if (sepol_deny(source.data[i], target.data[j], class, permission.data[k]))
							fprintf(stderr, "Error in: deny %s %s %s %s\n", source.data[i], target.data[j], class, permission.data[k]);
						break;
					case 2:
						if (sepol_auditallow(source.data[i], target.data[j], class, permission.data[k]))
							fprintf(stderr, "Error in: auditallow %s %s %s %s\n", source.data[i], target.data[j], class, permission.data[k]);
						break;
					case 3:
						if (sepol_auditdeny(source.data[i], target.data[j], class, permission.data[k]))
							fprintf(stderr, "Error in: auditdeny %s %s %s %s\n", source.data[i], target.data[j], class, permission.data[k]);
						break;
					default:
						return 1;
				}
	vec_destroy(&source);
	vec_destroy(&target);
	vec_destroy(&permission);
	return 0;
}

// Pattern 2: action { class } { attribute }
static int parse_pattern_2(int action, char* statement) {
	int state = 0, in_bracket = 0;
	char *tok, *saveptr;
	struct vector class, attribute, *temp;
	vec_init(&class);
	vec_init(&attribute);
	tok = strtok_r(statement, " ", &saveptr);
	while (tok != NULL) {
		if (tok[0] == '{') {
			if (in_bracket) return 1;
			in_bracket = 1;
			if (tok[1]) {
				++tok;
				continue;
			}
		} else if (tok[strlen(tok) - 1] == '}') {
			if (!in_bracket) return 1;
			in_bracket = 0;
			if (strlen(tok) - 1) {
				tok[strlen(tok) - 1] = '\0';
				continue;
			}
		} else {
			if (tok[0] == '*') tok = ALL;
			switch (state) {
				case 0:
					temp = &class;
					break;
				case 1:
					temp = &attribute;
					break;
				default:
					return 1;
			}
			vec_push_back(temp, tok);
		}
		if (!in_bracket) ++state;
		tok = strtok_r(NULL, " ", &saveptr);
	}
	if (state != 2) return 1;
	for(int i = 0; i < class.size; ++i)
		for (int j = 0; j < attribute.size; ++j)
			switch (action) {
				case 0:
					if (sepol_attradd(class.data[i], attribute.data[j]))
						fprintf(stderr, "Error in: attradd %s %s\n", class.data[i], attribute.data[j]);
					break;
				default:
					return 1;
			}
	vec_destroy(&class);
	vec_destroy(&attribute);
	return 0;
}

// Pattern 3: action { type }
static int parse_pattern_3(int action, char* statement) {
	char *tok, *saveptr;
	struct vector classes;
	vec_init(&classes);
	tok = strtok_r(statement, " {}", &saveptr);
	while (tok != NULL) {
		if (tok[0] == '*') tok = ALL;
		vec_push_back(&classes, tok);
		tok = strtok_r(NULL, " {}", &saveptr);
	}
	for (int i = 0; i < classes.size; ++i) {
		switch (action) {
			case 0:
				if (sepol_create(classes.data[i]))
					fprintf(stderr, "Domain %s already exists\n", classes.data[i]);
				break;
			case 1:
				if (sepol_permissive(classes.data[i]))
					fprintf(stderr, "Error in: permissive %s\n", classes.data[i]);
				break;
			case 2:
				if (sepol_enforce(classes.data[i]))
					fprintf(stderr, "Error in: enforce %s\n", classes.data[i]);
				break;
		}
	}
	vec_destroy(&classes);
	return 0;
}

// Pattern 4: action source target class default (filename)
static int parse_pattern_4(int action, char* statement) {
	int state = 0;
	char *tok, *saveptr;
	char *source, *target, *class, *def, *filename = NULL;
	tok = strtok_r(statement, " ", &saveptr);
	while (tok != NULL) {
		switch(state) {
			case 0:
				source = tok;
				break;
			case 1:
				target = tok;
				break;
			case 2:
				class = tok;
				break;
			case 3:
				def = tok;
				break;
			case 4:
				filename = tok;
				break;
			default:
				return 1;
		}
		tok = strtok_r(NULL, " ", &saveptr);
		++state;
	}
	if (state < 4) return 1;
	if (sepol_typetrans(source, target, class, def, filename))
		fprintf(stderr, "Error in: typetrans %s %s %s %s %s\n", source, target, class, def, filename ? filename : "");
	return 0;
}

// Pattern 5: action { source } { target } { class } ioctl range
static int parse_pattern_5(int action, char* statement) {
	int state = 0, in_bracket = 0;
	char *tok, *range, *saveptr;
	struct vector source, target, class, *temp;
	vec_init(&source);
	vec_init(&target);
	vec_init(&class);
	tok = strtok_r(statement, " ", &saveptr);
	while (tok != NULL) {
		if (tok[0] == '{') {
			if (in_bracket || state == 3 || state == 4) return 1;
			in_bracket = 1;
			if (tok[1]) {
				++tok;
				continue;
			}
		} else if (tok[strlen(tok) - 1] == '}') {
			if (!in_bracket || state == 3 || state == 4) return 1;
			in_bracket = 0;
			if (strlen(tok) - 1) {
				tok[strlen(tok) - 1] = '\0';
				continue;
			}
		} else {
			if (tok[0] == '*') tok = ALL;
			switch (state) {
				case 0:
					temp = &source;
					break;
				case 1:
					temp = &target;
					break;
				case 2:
					temp = &class;
					break;
				case 3:
					// Should always be ioctl
					temp = NULL;
					break;
				case 4:
					temp = NULL;
					range = tok;
					break;
				default:
					return 1;
			}
			vec_push_back(temp, tok);
		}
		if (!in_bracket) ++state;
		tok = strtok_r(NULL, " ", &saveptr);
	}
	if (state != 5) return 1;
	for(int i = 0; i < source.size; ++i)
		for (int j = 0; j < target.size; ++j)
			for (int k = 0; k < class.size; ++k)
				switch (action) {
					case 0:
						if (sepol_allowxperm(source.data[i], target.data[j], class.data[k], range))
							fprintf(stderr, "Error in: allowxperm %s %s %s %s\n", source.data[i], target.data[j], class.data[k], range);
						break;
					case 1:
						if (sepol_auditallowxperm(source.data[i], target.data[j], class.data[k], range))
							fprintf(stderr, "Error in: auditallowxperm %s %s %s %s\n", source.data[i], target.data[j], class.data[k], range);
						break;
					case 2:
						if (sepol_dontauditxperm(source.data[i], target.data[j], class.data[k], range))
							fprintf(stderr, "Error in: dontauditxperm %s %s %s %s\n", source.data[i], target.data[j], class.data[k], range);
						break;
					default:
						return 1;
				}
	vec_destroy(&source);
	vec_destroy(&target);
	vec_destroy(&class);
	return 0;
}

static void syntax_error_msg() {
	fprintf(stderr, "Syntax error in \"%s\"\n", err_msg);
	syntax_err = 1;
}

int magiskpolicy_main(int argc, char *argv[]) {
	char *infile = NULL, *outfile = NULL, *tok, *saveptr;
	int live = 0, minimal = 0;
	struct vector rules;

	vec_init(&rules);

	if (argc < 2) usage(argv[0]);
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			if (strcmp(argv[i], "--live") == 0)
				live = 1;
			else if (strcmp(argv[i], "--minimal") == 0)
				minimal = 1;
			else if (strcmp(argv[i], "--load") == 0) {
				if (i + 1 >= argc) usage(argv[0]);
				infile = argv[i + 1];
				i += 1;
			} else if (strcmp(argv[i], "--save") == 0) {
				if (i + 1 >= argc) usage(argv[0]);
				outfile = argv[i + 1];
				i += 1;
			} else 
				usage(argv[0]);
		} else
			vec_push_back(&rules, argv[i]);
	}

	// Use current policy if not specified
	if(!infile)
		infile = SELINUX_POLICY;

	if (load_policydb(infile)) {
		fprintf(stderr, "Could not load policy\n");
		return 1;
	}

	if (minimal)
		sepol_min_rules();

	for (int i = 0; i < rules.size; ++i) {
		// Since strtok will modify the origin string, copy the policy for error messages
		strcpy(err_msg, rules.data[i]);
		tok = strtok_r(rules.data[i], " ", &saveptr);
		if (strcmp(tok, "allow") == 0) {
			if (parse_pattern_1(0, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "deny") == 0) {
			if (parse_pattern_1(1, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "auditallow") == 0) {
			if (parse_pattern_1(2, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "auditdeny") == 0) {
			if (parse_pattern_1(3, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "attradd") == 0) {
			if (parse_pattern_2(0, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "create") == 0) {
			if (parse_pattern_3(0, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "permissive") == 0) {
			if (parse_pattern_3(1, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "enforce") == 0) {
			if (parse_pattern_3(2, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "typetrans") == 0) {
			if (parse_pattern_4(0, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "allowxperm") == 0) {
			if (parse_pattern_5(0, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "auditallowxperm") == 0) {
			if (parse_pattern_5(1, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else if (strcmp(tok, "dontauditxperm") == 0) {
			if (parse_pattern_5(2, rules.data[i] + strlen(tok) + 1))
				syntax_error_msg();
		} else {
			syntax_error_msg();
		}
	}

	if (syntax_err)
		statements();

	vec_destroy(&rules);

	if (live)
		if (dump_policydb(SELINUX_LOAD))
			return 1;

	if (outfile) {
		unlink(outfile);
		if (dump_policydb(outfile))
			return 1;
	}

	destroy_policydb();
	return 0;
}
