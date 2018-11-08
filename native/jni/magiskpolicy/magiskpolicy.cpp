/* magiskpolicy.cpp - Main function for policy patching
 *
 * Includes all the parsing logic for the policy statements
 */

#include <stdio.h>
#include <limits.h>

#include "sepolicy.h"
#include "array.h"
#include "magiskpolicy.h"
#include "magisk.h"
#include "flags.h"

static const char *type_msg_1 =
"Type 1:\n"
"\"<action> source-class target-class permission-class permission\"\n"
"Action: allow, deny, auditallow, auditdeny\n";

static const char *type_msg_2 =
"Type 2:\n"
"\"<action> source-class target-class permission-class ioctl range\"\n"
"Action: allowxperm, auditallowxperm, dontauditxperm\n";

static const char *type_msg_3 =
"Type 3:\n"
"\"<action> class\"\n"
"Action: create, permissive, enforcing\n";

static const char *type_msg_4 =
"Type 4:\n"
"\"attradd class attribute\"\n";

static const char *type_msg_5 =
"Type 5:\n"
"\"typetrans source-class target-class permission-class default-class (optional: object-name)\"\n";


[[noreturn]] static void statements() {
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
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
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
		"\n",
  		type_msg_1, type_msg_2, type_msg_3, type_msg_4, type_msg_5);
	exit(0);
}

[[noreturn]] static void usage(char *arg0) {
	fprintf(stderr,
		"MagiskPolicy v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ") (by topjohnwu)\n\n"
		"Usage: %s [--options...] [policy statements...]\n"
		"\n"
		"Options:\n"
  		"   --help            show help message for policy statements\n"
		"   --load FILE       load policies from FILE\n"
		"   --compile-split   compile and load split cil policies\n"
		"                     from system and vendor just like init\n"
		"   --save FILE       save policies to FILE\n"
		"   --live            directly apply sepolicy live\n"
		"   --magisk          inject built-in rules for a minimal\n"
		"                     Magisk selinux environment\n"
		"\n"
		"If neither --load or --compile-split is specified, it will load\n"
		"from current live policies (" SELINUX_POLICY ")\n"
		"\n",
		arg0);
	exit(1);
}

static char *parse_bracket(char *str, Array<const char *> *vec) {
	str = strchr(str, '{') + 1;
	char *end = strchr(str, '}');
	if (end == nullptr)
		return nullptr;
	*end = '\0';
	char *cur;
	while ((cur = strtok_r(nullptr, " ", &str)) != nullptr)
		vec->push_back(cur);
	return end + 1;
}

// Pattern 1: action { source } { target } class { permission }
static int parse_pattern_1(int action, char *stmt) {
	int state = 0;
	char *cur, *cls;
	Array<const char*> source, target, permission;
	while ((cur = strtok_r(nullptr, " ", &stmt)) != nullptr) {
		if (cur[0] == '*') cur = ALL;
		Array<const char *> *vec;
		switch (state) {
			case 0:
				vec = &source;
				break;
			case 1:
				vec = &target;
				break;
			case 2:
				vec = nullptr;
				cls = cur;
				break;
			case 3:
				vec = &permission;
				break;
			default:
				return 1;
		}

		if (vec) {
			if (cur == nullptr || cur[0] != '{') {
				vec->push_back(cur);
			} else {
				stmt[-1] = ' ';
				stmt = parse_bracket(cur, vec);
				if (stmt == nullptr)
					return 1;
			}
		}
		++state;
	}
	if (state != 4)
		return 1;
	for(int i = 0; i < source.size(); ++i)
		for (int j = 0; j < target.size(); ++j)
			for (int k = 0; k < permission.size(); ++k) {
				int (*action_func)(const char*, const char*, const char*, const char*);
				const char *action_str;
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
				if (action_func(source[i], target[j], cls, permission[k]))
					fprintf(stderr, "Error in: %s %s %s %s %s\n",
							action_str, source[i], target[j], cls, permission[k]);
			}
	return 0;
}

// Pattern 2: action { source } { target } { class } ioctl range
static int parse_pattern_2(int action, char *stmt) {
	int state = 0;
	char *cur, *range;
	Array<const char *> source, target, cls;
	while ((cur = strtok_r(nullptr, " ", &stmt)) != nullptr) {
		if (cur[0] == '*') cur = ALL;
		Array<const char *> *vec;
		switch (state) {
			case 0:
				vec = &source;
				break;
			case 1:
				vec = &target;
				break;
			case 2:
				vec = &cls;
				break;
			case 3:
				// Currently only support ioctl
				vec = nullptr;
				break;
			case 4:
				vec = nullptr;
				range = cur;
				break;
			default:
				return 1;
		}

		if (vec) {
			if (cur == nullptr || cur[0] != '{') {
				vec->push_back(cur);
			} else {
				stmt[-1] = ' ';
				stmt = parse_bracket(cur, vec);
				if (stmt == nullptr)
					return 1;
			}
		}
		++state;
	}
	if (state != 5) return 1;
	for(int i = 0; i < source.size(); ++i)
		for (int j = 0; j < target.size(); ++j)
			for (int k = 0; k < cls.size(); ++k) {
				int (*action_func)(const char*, const char*, const char*, const char*);
				const char *action_str;
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
				if (action_func(source[i], target[j], cls[k], range))
					fprintf(stderr, "Error in: %s %s %s %s %s\n",
							action_str, source[i], target[j], cls[k], range);
			}
	return 0;
}

// Pattern 3: action { type }
static int parse_pattern_3(int action, char* stmt) {
	char *cur;
	Array<const char *> domains;
	while ((cur = strtok_r(nullptr, " {}", &stmt)) != nullptr) {
		if (cur[0] == '*') cur = ALL;
		domains.push_back(cur);
	}
	for (int i = 0; i < domains.size(); ++i) {
		int (*action_func)(const char*);
		const char *action_str;
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
			default:
				return 1;
		}
		if (action_func(domains[i]))
			fprintf(stderr, "Error in: %s %s\n", action_str, domains[i]);
	}
	return 0;
}

// Pattern 4: action { class } { attribute }
static int parse_pattern_4(int action, char *stmt) {
	int state = 0;
	char *cur;
	Array<const char *> cls, attribute;
	while ((cur = strtok_r(nullptr, " ", &stmt)) != nullptr) {
		if (cur[0] == '*') cur = ALL;
		Array<const char *> *vec;
		switch (state) {
			case 0:
				vec = &cls;
				break;
			case 1:
				vec = &attribute;
				break;
			default:
				return 1;
		}

		if (cur == nullptr || cur[0] != '{') {
			vec->push_back(cur);
		} else {
			stmt[-1] = ' ';
			stmt = parse_bracket(cur, vec);
			if (stmt == nullptr)
				return 1;
		}
		++state;
	}
	if (state != 2) return 1;
	for(int i = 0; i < cls.size(); ++i)
		for (int j = 0; j < attribute.size(); ++j) {
			int (*action_func)(const char*, const char*);
			const char *action_str;
			switch (action) {
				case 0:
					action_func = sepol_attradd;
					action_str = "attradd";
					break;
				default:
					return 1;
			}
			if (action_func(cls[i], attribute[j]))
				fprintf(stderr, "Error in: %s %s %s\n", action_str, cls[i], attribute[j]);
		}
	return 0;
}

// Pattern 5: action source target class default (filename)
static int parse_pattern_5(int action, char *stmt) {
	int state = 0;
	char *cur;
	char *source, *target, *cls, *def, *filename = nullptr;
	while ((cur = strtok_r(nullptr, " ", &stmt)) != nullptr) {
		switch(state) {
		case 0:
			source = cur;
			break;
		case 1:
			target = cur;
			break;
		case 2:
			cls = cur;
			break;
		case 3:
			def = cur;
			break;
		case 4:
			filename = cur;
			break;
		default:
			return 1;
		}
		++state;
	}
	if (state < 4) return 1;
	if (sepol_typetrans(source, target, cls, def, filename))
		fprintf(stderr, "Error in: typetrans %s %s %s %s %s\n", source, target, cls, def, filename ? filename : "");
	return 0;
}

#define add_action(name, type, num) \
else if (strcmp(name, action) == 0) { \
	if (parse_pattern_##type(num, remain)) \
		fprintf(stderr, "Syntax error in '%s'\n\n%s\n", orig, type_msg_##type); \
}

static void parse_statement(char *statement) {
	char *action, *remain;

	// strtok will modify the origin string, duplicate the statement for error messages
	char *orig = strdup(statement);

	action = strtok_r(statement, " ", &remain);
	if (remain == nullptr) remain = &action[strlen(action)];

	if (0) {}
	add_action("allow", 1, 0)
	add_action("deny", 1, 1)
	add_action("auditallow", 1, 2)
	add_action("auditdeny", 1, 3)
	add_action("allowxperm", 2, 0)
	add_action("auditallowxperm", 2, 1)
	add_action("dontauditxperm", 2, 2)
	add_action("create", 3, 0)
	add_action("permissive", 3, 1)
	add_action("enforce", 3, 2)
	add_action("attradd", 4, 0)
	add_action("typetrans", 5, 0)
	else { fprintf(stderr, "Unknown statement: '%s'\n\n", orig); }

	free(orig);
}

int magiskpolicy_main(int argc, char *argv[]) {
	const char *outfile = nullptr;
	bool magisk = false, live = false;

	if (argc < 2) usage(argv[0]);
	int i = 1;
	for (; i < argc; ++i) {
		// Parse options
		if (argv[i][0] == '-' && argv[i][1] == '-') {
			if (strcmp(argv[i] + 2, "live") == 0)
				live = true;
			else if (strcmp(argv[i] + 2, "magisk") == 0)
				magisk = true;
			else if (strcmp(argv[i] + 2, "load") == 0) {
				if (argv[i + 1] == nullptr)
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
				if (argv[i + 1] == nullptr)
					usage(argv[0]);
				outfile = argv[i + 1];
				++i;
			} else if (strcmp(argv[i] + 2, "help") == 0) {
				statements();
			} else {
				usage(argv[0]);
			}
		} else {
			break;
		}
	}

	// Use current policy if nothing is loaded
	if (policydb == nullptr && load_policydb(SELINUX_POLICY)) {
		fprintf(stderr, "Cannot load policy from " SELINUX_POLICY "\n");
		return 1;
	}

	for (; i < argc; ++i)
		parse_statement(argv[i]);

	if (magisk)
		sepol_magisk_rules();

	if (live && dump_policydb(SELINUX_LOAD)) {
		fprintf(stderr, "Cannot apply policy\n");
		return 1;
	}

	if (outfile && dump_policydb(outfile)) {
		fprintf(stderr, "Cannot dump policy to %s\n", outfile);
		return 1;
	}

	destroy_policydb();
	return 0;
}
