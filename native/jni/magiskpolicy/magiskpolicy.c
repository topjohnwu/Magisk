/* magiskpolicy.c - Main function for policy patching
 *
 * Includes all the parsing logic for the policy statements
 */

#include <stdio.h>

#include "sepolicy.h"
#include "vector.h"
#include "magiskpolicy.h"
#include "magisk.h"

static int syntax_err = 0;
static char err_msg[ARG_MAX];

static void statements() {
	fprintf(stderr,
		"One policy statement should be treated as one parameter;\n"
		"this means a full policy statement should be enclosed in quotes;\n"
		"multiple policy statements can be provided in a single command\n"
		"\n"
		"The statements has a format of \"<action> [args...]\"\n"
		"Use '*' in args to represent every possible match.\n"
		"Collections wrapped in curly brackets can also be used as args.\n"
		"\n"
		"Supported policy statements:\n"
		"\n"
		"Type 1:\n"
		"\"<action> source-class target-class permission-class permission\"\n"
		"Action: allow, deny, auditallow, auditdeny\n"
		"\n"
		"Type 2:\n"
		"\"<action> source-class target-class permission-class ioctl range\"\n"
		"Action: allowxperm, auditallowxperm, dontauditxperm\n"
		"\n"
		"Type 3:\n"
		"\"<action> class\"\n"
		"Action: create, permissive, enforcing\n"
		"\n"
		"Type 4:\n"
		"\"attradd class attribute\"\n"
		"\n"
		"Type 5:\n"
		"\"typetrans source-class target-class permission-class default-class (optional: object-name)\"\n"
		"\n"
		"Notes:\n"
		"- typetrans does not support the all match '*' syntax\n"
		"- permission-class cannot be collections\n"
		"- source-class and target-class can also be attributes\n"
		"\n"
		"Example: allow { source1 source2 } { target1 target2 } permission-class *\n"
		"Will be expanded to:\n"
		"\n"
		"allow source1 target1 permission-class { all-permissions }\n"
		"allow source1 target2 permission-class { all-permissions }\n"
		"allow source2 target1 permission-class { all-permissions }\n"
		"allow source2 target2 permission-class { all-permissions }\n"
		"\n"
	);
}

static void usage(char *arg0) {
	fprintf(stderr,
		"MagiskPolicy v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu)\n\n"
		"Usage: %s [--options...] [policy statements...]\n"
		"\n"
		"Options:\n"
		"   --live            directly apply sepolicy live\n"
		"   --magisk          inject built-in rules for a minimal\n"
		"                     Magisk selinux environment\n"
		"   --load FILE       load policies from FILE\n"
		"   --compile-split   compile and load split cil policies\n"
		"                     from system and vendor just like init\n"
		"   --save FILE       save policies to FILE\n"
		"\n"
		"If neither --load or --compile-split is specified, it will load\n"
		"from current live policies (" SELINUX_POLICY ")\n"
		"\n"
		, arg0);
	statements();
	exit(1);
}

// Pattern 1: action { source } { target } class { permission }
static int parse_pattern_1(int action, char* statement) {
	int state = 0, in_bracket = 0;
	char *tok, *class, *saveptr;
	struct vector source, target, permission;
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
			struct vector *vec;
			switch (state) {
			case 0:
				vec = &source;
				break;
			case 1:
				vec = &target;
				break;
			case 2:
				vec = NULL;
				class = tok;
				break;
			case 3:
				vec = &permission;
				break;
			default:
				return 1;
			}
			vec_push_back(vec, tok);
		}
		if (!in_bracket) ++state;
		tok = strtok_r(NULL, " ", &saveptr);
	}
	if (state != 4) return 1;
	for(int i = 0; i < source.size; ++i)
		for (int j = 0; j < target.size; ++j)
			for (int k = 0; k < permission.size; ++k) {
				int (*action_func)(char*, char*, char*, char*);
				char *action_str;
				switch (action) {
				case 0:
					action_func = sepol_allow;
					action_str = "allow";
					break;
				case 1:
					action_func = sepol_deny;
					action_str = "deny";
					break;
				case 2:
					action_func = sepol_auditallow;
					action_str = "auditallow";
					break;
				case 3:
					action_func = sepol_auditdeny;
					action_str = "auditdeny";
					break;
				default:
					return 1;
				}
				if (action_func(source.data[i], target.data[j], class, permission.data[k]))
					fprintf(stderr, "Error in: %s %s %s %s %s\n",
						action_str, (char *) source.data[i], (char *) target.data[j], class, (char *) permission.data[k]);
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
	struct vector class, attribute;
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
			struct vector *vec;
			switch (state) {
			case 0:
				vec = &class;
				break;
			case 1:
				vec = &attribute;
				break;
			default:
				return 1;
			}
			vec_push_back(vec, tok);
		}
		if (!in_bracket) ++state;
		tok = strtok_r(NULL, " ", &saveptr);
	}
	if (state != 2) return 1;
	for(int i = 0; i < class.size; ++i)
		for (int j = 0; j < attribute.size; ++j) {
			int (*action_func)(char*, char*);
			char *action_str;
			switch (action) {
				case 0:
					action_func = sepol_attradd;
					action_str = "attradd";
					break;
				default:
					return 1;
			}
			if (action_func(class.data[i], attribute.data[j]))
				fprintf(stderr, "Error in: %s %s %s\n",
					action_str, (char *) class.data[i], (char *) attribute.data[j]);
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
		int (*action_func)(char*);
		char *action_str;
		switch (action) {
		case 0:
			action_func = sepol_create;
			action_str = "create";
			break;
		case 1:
			action_func = sepol_permissive;
			action_str = "permissive";
			break;
		case 2:
			action_func = sepol_enforce;
			action_str = "enforce";
			break;
		}
		if (action_func(classes.data[i]))
			fprintf(stderr, "Error in: %s %s\n", action_str, (char *) classes.data[i]);
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
	struct vector source, target, class;
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
			struct vector *vec;
			switch (state) {
			case 0:
				vec = &source;
				break;
			case 1:
				vec = &target;
				break;
			case 2:
				vec = &class;
				break;
			case 3:
				// Should always be ioctl
				vec = NULL;
				break;
			case 4:
				vec = NULL;
				range = tok;
				break;
			default:
				return 1;
			}
			vec_push_back(vec, tok);
		}
		if (!in_bracket) ++state;
		tok = strtok_r(NULL, " ", &saveptr);
	}
	if (state != 5) return 1;
	for(int i = 0; i < source.size; ++i)
		for (int j = 0; j < target.size; ++j)
			for (int k = 0; k < class.size; ++k) {
				int (*action_func)(char*, char*, char*, char*);
				char *action_str;
				switch (action) {
				case 0:
					action_func = sepol_allowxperm;
					action_str = "allowxperm";
					break;
				case 1:
					action_func = sepol_auditallowxperm;
					action_str = "auditallowxperm";
					break;
				case 2:
					action_func = sepol_dontauditxperm;
					action_str = "dontauditxperm";
					break;
				default:
					return 1;
				}
				if (action_func(source.data[i], target.data[j], class.data[k], range))
					fprintf(stderr, "Error in: %s %s %s %s %s\n",
						action_str, (char *) source.data[i], (char *) target.data[j], (char *) class.data[k], range);
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
	char *outfile = NULL, *tok, *saveptr;
	int magisk = 0;
	struct vector rules;

	vec_init(&rules);

	if (argc < 2) usage(argv[0]);
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			if (strcmp(argv[i] + 2, "live") == 0)
				outfile = SELINUX_LOAD;
			else if (strcmp(argv[i] + 2, "magisk") == 0)
				magisk = 1;
			else if (strcmp(argv[i] + 2, "load") == 0) {
				if (i + 1 >= argc)
					usage(argv[0]);
				if (load_policydb(argv[i + 1])) {
					fprintf(stderr, "Cannot load policy from %s\n", argv[i + 1]);
					return 1;
				}
				++i;
			} else if (strcmp(argv[i] + 2, "compile-split") == 0) {
				if (compile_split_cil()) {
					fprintf(stderr, "Cannot compile split cil\n");
					return 1;
				}
			} else if (strcmp(argv[i] + 2, "save") == 0) {
				if (i + 1 >= argc)
					usage(argv[0]);
				outfile = argv[i + 1];
				++i;
			} else {
				usage(argv[0]);
			}
		} else {
			vec_push_back(&rules, argv[i]);
		}
	}

	// Use current policy if nothing is loaded
	if(policydb == NULL && load_policydb(SELINUX_POLICY)) {
		fprintf(stderr, "Cannot load policy from " SELINUX_POLICY "\n");
		return 1;
	}

	if (magisk)
		sepol_magisk_rules();

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

	if (outfile && dump_policydb(outfile)) {
		fprintf(stderr, "Cannot dump policy to %s\n", outfile);
		return 1;
	}

	destroy_policydb();
	return 0;
}
