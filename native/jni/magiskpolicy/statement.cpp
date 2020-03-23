#include <cstring>
#include <vector>
#include <string>

#include <magiskpolicy.hpp>
#include <logging.hpp>
#include <utils.hpp>

using namespace std;

static const char *type_msg_1 =
R"EOF(Type 1:
"<rule_name> source_type target_type class perm_set"
Rules: allow, deny, auditallow, dontaudit
)EOF";

static const char *type_msg_2 =
R"EOF(Type 2:
"<rule_name> source_type target_type class operation xperm_set"
Rules: allowxperm, auditallowxperm, dontauditxperm
* The only supported operation is ioctl
* The only supported xperm_set format is range ([low-high])
)EOF";

static const char *type_msg_3 =
R"EOF(Type 3:
"<rule_name> class"
Rules: create, permissive, enforcing
)EOF";

static const char *type_msg_4 =
R"EOF(Type 4:
"attradd class attribute"
)EOF";

static const char *type_msg_5 =
R"EOF(Type 5:
"<rule_name> source_type target_type class default_type"
Rules: type_transition, type_change, type_member
)EOF";

static const char *type_msg_6 =
R"EOF(Type 6:
"name_transition source_type target_type class default_type object_name"
)EOF";

static const char *type_msg_7 =
R"EOF(Type 7:
"genfscon fs_name partial_path fs_context"
)EOF";

void statement_help() {
	fprintf(stderr,
R"EOF(One policy statement should be treated as one parameter;
this means a full policy statement should be enclosed in quotes.
Multiple policy statements can be provided in a single command.

The statements has a format of "<rule_name> [args...]"
Multiple types and permissions can be grouped into collections
wrapped in curly brackets.
'*' represents a collection containing all valid matches.

Supported policy statements:

%s
%s
%s
%s
%s
%s
%s
Notes:
* Type 4 - 7 does not support collections
* Object classes cannot be collections
* source_type and target_type can also be attributes

Example: allow { s1 s2 } { t1 t2 } class *
Will be expanded to:

allow s1 t1 class { all-permissions }
allow s1 t2 class { all-permissions }
allow s2 t1 class { all-permissions }
allow s2 t2 class { all-permissions }

)EOF", type_msg_1, type_msg_2, type_msg_3, type_msg_4, type_msg_5, type_msg_6, type_msg_7);
	exit(0);
}

static int parse_bracket(char *tok, char *&stmt, vector<const char *> &vec) {
	if (tok == nullptr || tok[0] != '{') {
		// Not in a bracket
		vec.push_back(tok);
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
			vec.push_back(cur);
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

		if (vec && parse_bracket(cur, stmt, *vec))
			return 1;
		++state;
	}
	if (state != 4 || source.empty() || target.empty() || permission.empty())
		return 1;

	for (auto src : source)
		for (auto tgt : target)
			for (auto perm : permission)
				if (action_func(src, tgt, cls, perm))
					LOGW("Error in: %s %s %s %s %s\n", action_str, src, tgt, cls, perm);

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

		if (vec && parse_bracket(cur, stmt, *vec))
			return 1;
		++state;
	}
	if (state != 5 || source.empty() || target.empty() || classes.empty())
		return 1;

	for (auto src : source)
		for (auto tgt : target)
			for (auto cls : classes)
				if (action_func(src, tgt, cls, range))
					LOGW("Error in: %s %s %s %s %s\n", action_str, src, tgt, cls, range);

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
			LOGW("Error in: %s %s\n", action_str, dom);

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

		if (parse_bracket(cur, stmt, *vec))
			return 1;
		++state;
	}
	if (state != 2 || classes.empty() || attribute.empty())
		return 1;

	for (auto cls : classes)
		for (auto attr : attribute)
			if (action_func(cls, attr))
				LOGW("Error in: %s %s %s\n", action_str, cls, attr);

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
		LOGW("Error in: %s %s %s %s %s\n", action_str, source, target, cls, def);
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
	if (state < 5) return 1;
	if (sepol_nametrans(source, target, cls, def, filename))
		LOGW("Error in: %s %s %s %s %s %s\n", action_str, source, target, cls, def, filename);
	return 0;
}

// Pattern 7: action name path context
static int parse_pattern_7(int action, const char *action_str, char *stmt) {
	int state = 0;
	char *cur;
	char *name, *path, *context;
	while ((cur = strtok_r(nullptr, " ", &stmt)) != nullptr) {
		switch(state) {
			case 0:
				name = cur;
				break;
			case 1:
				path = cur;
				break;
			case 2:
				context = cur;
				break;
			default:
				return 1;
		}
		++state;
	}
	if (state < 3) return 1;
	if (sepol_genfscon(name, path, context))
		LOGW("Error in: %s %s %s %s\n", action_str, name, path, context);
	return 0;
}

#define add_action(name, type, num) \
else if (strcmp(name, action) == 0) { \
	if (parse_pattern_##type(num, name, remain)) \
		LOGW("Syntax error in '%s'\n\n%s\n", statement, type_msg_##type); \
}

void parse_statement(const char *statement) {
	char *action, *remain;

	// strtok will modify strings, duplicate the statement
	string stmt(statement);

	action = strtok_r(stmt.data(), " ", &remain);

	if (remain == nullptr) {
		LOGE("Syntax error in '%s'\n\n", statement);
		return;
	}

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
	add_action("genfscon", 7, 0)
	else { LOGW("Unknown statement: '%s'\n\n", statement); }
}

void load_rule_file(const char *file) {
	file_readline(true, file, [](string_view line) -> bool {
		if (line.empty() || line[0] == '#')
			return true;
		parse_statement(line.data());
		return true;
	});
}
