/* magiskpolicy.cpp - Main function for policy patching
 *
 * Includes all the parsing logic for the policy statements
 */

#include <stdio.h>
#include <limits.h>
#include <vector>
#include <string>

#include <magisk.h>
#include <utils.h>
#include <flags.h>

#include "sepolicy.h"
#include "magiskpolicy.h"

using namespace std;

static const char *type_msg_1 =
"Type 1:\n"
"\"<rule_name> source_type target_type class perm_set\"\n"
"Rules: allow, deny, auditallow, dontaudit\n";

static const char *type_msg_2 =
"Type 2:\n"
"\"<rule_name> source_type target_type class operation xperm_set\"\n"
"Rules: allowxperm, auditallowxperm, dontauditxperm\n"
"* The only supported operation is ioctl\n"
"* The only supported xperm_set format is range ([low-high])\n";

static const char *type_msg_3 =
"Type 3:\n"
"\"<rule_name> class\"\n"
"Rules: create, permissive, enforcing\n";

static const char *type_msg_4 =
"Type 4:\n"
"\"attradd class attribute\"\n";

static const char *type_msg_5 =
"Type 5:\n"
"\"<rule_name> source_type target_type class default_type\"\n"
"Rules: type_transition, type_change, type_member\n";

static const char *type_msg_6 =
"Type 6:\n"
"\"name_transition source_type target_type class default_type object_name\"\n";


[[noreturn]] static void statements() {
	fprintf(stderr,
		"One policy statement should be treated as one parameter;\n"
		"this means a full policy statement should be enclosed in quotes;\n"
		"multiple policy statements can be provided in a single command\n"
		"\n"
		"The statements has a format of \"<rule_name> [args...]\"\n"
		"Multiple types and permissions can be grouped into collections\n"
		"wrapped in curly brackets.\n"
		"'*' represents a collection containing all valid matches.\n"
		"\n"
		"Supported policy statements:\n"
		"\n"
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
		"Notes:\n"
		"* Type 4 - 6 does not support collections\n"
		"* Object classes cannot be collections\n"
		"* source_type and target_type can also be attributes\n"
		"\n"
		"Example: allow { s1 s2 } { t1 t2 } class *\n"
		"Will be expanded to:\n"
		"\n"
		"allow s1 t1 class { all permissions }\n"
		"allow s1 t2 class { all permissions }\n"
		"allow s2 t1 class { all permissions }\n"
		"allow s2 t2 class { all permissions }\n"
		"\n",
		type_msg_1, type_msg_2, type_msg_3, type_msg_4, type_msg_5, type_msg_6);
	exit(0);
}

[[noreturn]] static void usage(char *arg0) {
	fprintf(stderr,
		FULL_VER(MagiskPolicy) "\n\n"
		"Usage: %s [--options...] [policy statements...]\n"
		"\n"
		"Options:\n"
  		"   --help            show help message for policy statements\n"
		"   --load FILE       load policies from FILE\n"
		"   --load-split      load from preloaded sepolicy or compile\n"
		"                     split policies\n"
		"   --compile-split   compile split cil policies\n"
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

static int parse_bracket(char *tok, char *&stmt, vector<const char *> *vec) {
	if (tok == nullptr || tok[0] != '{') {
		// Not in a bracket
		vec->push_back(tok);
	} else {
		if (stmt)
			stmt[-1] = ' ';
		tok = strchr(tok, '{') + 1;
		char *end = strchr(tok, '}');
		if (end == nullptr) // Bracket not closed
			return 1;
		*end = '\0';
		char *cur;
		while ((cur = strtok_r(nullptr, " ", &tok)) != nullptr)
			vec->push_back(cur);
		stmt = end + 1;
	}
	return 0;
}

// Pattern 1: action { source } { target } class { permission }
static int parse_pattern_1(int action, const char *action_str, char *stmt) {
	int (*action_func)(const char*, const char*, const char*, const char*);
	switch (action) {
		case 0:
			action_func = sepol_allow;
			break;
		case 1:
			action_func = sepol_deny;
			break;
		case 2:
			action_func = sepol_auditallow;
			break;
		case 3:
			action_func = sepol_dontaudit;
			break;
		default:
			return 1;
	}

	int state = 0;
	char *cur, *cls;
	vector<const char*> source, target, permission;
	while ((cur = strtok_r(nullptr, " ", &stmt)) != nullptr) {
		if (cur[0] == '*') cur = ALL;
		vector<const char *> *vec;
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

		if (vec && parse_bracket(cur, stmt, vec))
			return 1;
		++state;
	}
	if (state != 4 || source.empty() || target.empty() || permission.empty())
		return 1;

	for (auto src : source)
		for (auto tgt : target)
			for (auto perm : permission)
				if (action_func(src, tgt, cls, perm))
					fprintf(stderr, "Error in: %s %s %s %s %s\n", action_str, src, tgt, cls, perm);

	return 0;
}

// Pattern 2: action { source } { target } { class } ioctl range
static int parse_pattern_2(int action, const char *action_str, char *stmt) {
	int (*action_func)(const char*, const char*, const char*, const char*);
	switch (action) {
		case 0:
			action_func = sepol_allowxperm;
			break;
		case 1:
			action_func = sepol_auditallowxperm;
			break;
		case 2:
			action_func = sepol_dontauditxperm;
			break;
		default:
			return 1;
	}

	int state = 0;
	char *cur, *range;
	vector<const char *> source, target, classes;
	while ((cur = strtok_r(nullptr, " ", &stmt)) != nullptr) {
		if (cur[0] == '*') cur = ALL;
		vector<const char *> *vec;
		switch (state) {
			case 0:
				vec = &source;
				break;
			case 1:
				vec = &target;
				break;
			case 2:
				vec = &classes;
				break;
			case 3:
				// Currently only support ioctl
				if (strcmp(cur, "ioctl") != 0)
					return 1;
				vec = nullptr;
				break;
			case 4:
				vec = nullptr;
				range = cur;
				break;
			default:
				return 1;
		}

		if (vec && parse_bracket(cur, stmt, vec))
			return 1;
		++state;
	}
	if (state != 5 || source.empty() || target.empty() || classes.empty())
		return 1;

	for (auto src : source)
		for (auto tgt : target)
			for (auto cls : classes)
				if (action_func(src, tgt, cls, range))
					fprintf(stderr, "Error in: %s %s %s %s %s\n", action_str, src, tgt, cls, range);

	return 0;
}

// Pattern 3: action { type }
static int parse_pattern_3(int action, const char *action_str, char* stmt) {
	int (*action_func)(const char*);
	switch (action) {
		case 0:
			action_func = sepol_create;
			break;
		case 1:
			action_func = sepol_permissive;
			break;
		case 2:
			action_func = sepol_enforce;
			break;
		default:
			return 1;
	}

	char *cur;
	vector<const char *> domains;
	while ((cur = strtok_r(nullptr, " {}", &stmt)) != nullptr) {
		if (cur[0] == '*') cur = ALL;
		domains.push_back(cur);
	}

	if (domains.empty())
		return 1;

	for (auto dom : domains)
		if (action_func(dom))
			fprintf(stderr, "Error in: %s %s\n", action_str, dom);

	return 0;
}

// Pattern 4: action { class } { attribute }
static int parse_pattern_4(int action, const char *action_str, char *stmt) {
	int (*action_func)(const char*, const char*);
	switch (action) {
		case 0:
			action_func = sepol_attradd;
			break;
		default:
			return 1;
	}

	int state = 0;
	char *cur;
	vector<const char *> classes, attribute;
	while ((cur = strtok_r(nullptr, " ", &stmt)) != nullptr) {
		if (cur[0] == '*') cur = ALL;
		vector<const char *> *vec;
		switch (state) {
			case 0:
				vec = &classes;
				break;
			case 1:
				vec = &attribute;
				break;
			default:
				return 1;
		}

		if (parse_bracket(cur, stmt, vec))
			return 1;
		++state;
	}
	if (state != 2 || classes.empty() || attribute.empty())
		return 1;

	for (auto cls : classes)
		for (auto attr : attribute)
			if (action_func(cls, attr))
				fprintf(stderr, "Error in: %s %s %s\n", action_str, cls, attr);

	return 0;
}

// Pattern 5: action source target class default
static int parse_pattern_5(int action, const char *action_str, char *stmt) {
	int (*action_func)(const char*, const char*, const char*, const char*);
	switch (action) {
		case 0:
			action_func = sepol_typetrans;
			break;
		case 1:
			action_func = sepol_typechange;
			break;
		case 2:
			action_func = sepol_typemember;
			break;
		default:
			return 1;
	}
	int state = 0;
	char *cur;
	char *source, *target, *cls, *def;
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
		default:
			return 1;
		}
		++state;
	}
	if (state < 4) return 1;
	if (action_func(source, target, cls, def))
		fprintf(stderr, "Error in: %s %s %s %s %s\n", action_str, source, target, cls, def);
	return 0;
}

// Pattern 6: action source target class default filename
static int parse_pattern_6(int action, const char *action_str, char *stmt) {
	int state = 0;
	char *cur;
	char *source, *target, *cls, *def, *filename;
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
	if (sepol_nametrans(source, target, cls, def, filename))
		fprintf(stderr, "Error in: %s %s %s %s %s %s\n",
				action_str, source, target, cls, def, filename);
	return 0;
}

#define add_action(name, type, num) \
else if (strcmp(name, action) == 0) { \
	if (parse_pattern_##type(num, name, remain)) \
		fprintf(stderr, "Syntax error in '%s'\n\n%s\n", orig.c_str(), type_msg_##type); \
}

static void parse_statement(char *statement) {
	char *action, *remain;

	// strtok will modify the origin string, duplicate the statement for error messages
	string orig(statement);

	action = strtok_r(statement, " ", &remain);
	if (remain == nullptr) remain = &action[strlen(action)];

	if (0) {}
	add_action("allow", 1, 0)
	add_action("deny", 1, 1)
	add_action("auditallow", 1, 2)
	add_action("dontaudit", 1, 3)
	add_action("allowxperm", 2, 0)
	add_action("auditallowxperm", 2, 1)
	add_action("dontauditxperm", 2, 2)
	add_action("create", 3, 0)
	add_action("permissive", 3, 1)
	add_action("enforce", 3, 2)
	add_action("attradd", 4, 0)
	add_action("type_transition", 5, 0)
	add_action("type_change", 5, 1)
	add_action("type_member", 5, 2)
	add_action("name_transition", 6, 0)
	else { fprintf(stderr, "Unknown statement: '%s'\n\n", orig.c_str()); }
}

int magiskpolicy_main(int argc, char *argv[]) {
	cmdline_logging();
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
			} else if (strcmp(argv[i] + 2, "load-split") == 0) {
				if (load_split_cil()) {
					fprintf(stderr, "Cannot load split cil\n");
					return 1;
				}
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

	if (magisk)
		sepol_magisk_rules();

	for (; i < argc; ++i)
		parse_statement(argv[i]);

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
