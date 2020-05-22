#include <cstring>
#include <vector>
#include <string>

#include <magiskpolicy.hpp>
#include <logging.hpp>
#include <utils.hpp>

#include "sepolicy.h"

using namespace std;

static const char *type_msg_1 =
R"EOF(Type 1:
"<rule_name> ^source_type ^target_type ^class ^perm_set"
Rules: allow, deny, auditallow, dontaudit
)EOF";

static const char *type_msg_2 =
R"EOF(Type 2:
"<rule_name> ^source_type ^target_type ^class operation xperm_set"
Rules: allowxperm, auditallowxperm, dontauditxperm
- The only supported operation is ioctl
- The only supported xperm_set format is range ([low-high])
)EOF";

static const char *type_msg_3 =
R"EOF(Type 3:
"<rule_name> ^type"
Rules: create, permissive, enforcing
)EOF";

static const char *type_msg_4 =
R"EOF(Type 4:
"typeattribute ^type ^attribute"
)EOF";

static const char *type_msg_5 =
R"EOF(Type 5:
"<rule_name> source_type target_type class default_type"
Rules: type_change, type_member
)EOF";

static const char *type_msg_6 =
R"EOF(Type 6:
"type_transition source_type target_type class default_type (object_name)"
- Entry 'object_name' is optional
)EOF";

static const char *type_msg_7 =
R"EOF(Type 7:
"genfscon fs_name partial_path fs_context"
)EOF";

void statement_help() {
	fprintf(stderr,
R"EOF(One policy statement should be treated as one parameter;
this means each policy statement should be enclosed in quotes.
Multiple policy statements can be provided in a single command.

Statements has a format of "<rule_name> [args...]".
Arguments labeled with (^) can accept one or more entries. Multiple
entries consist of a space separated list enclosed in braces ({}).
For args that support multiple entries, (*) can be used to
represent all valid matches.

Example: "allow { s1 s2 } { t1 t2 } class *"
Will be expanded to:

allow s1 t1 class { all-permissions-of-class }
allow s1 t2 class { all-permissions-of-class }
allow s2 t1 class { all-permissions-of-class }
allow s2 t2 class { all-permissions-of-class }

Supported policy statements:

%s
%s
%s
%s
%s
%s
%s
)EOF", type_msg_1, type_msg_2, type_msg_3, type_msg_4, type_msg_5, type_msg_6, type_msg_7);
	exit(0);
}

static bool tokenize_string(char *stmt, vector<vector<char *>> &arr) {
	// cur is the pointer to where the top level is parsing
	char *cur = stmt;
	for (char *tok; (tok = strtok_r(nullptr, " ", &cur)) != nullptr;) {
		vector<char *> token;
		if (tok[0] == '{') {
			// cur could point to somewhere in the braces, restore the string
			if (cur)
				cur[-1] = ' ';
			++tok;
			char *end = strchr(tok, '}');
			if (end == nullptr) {
				// Bracket not closed, syntax error
				return false;
			}
			*end = '\0';
			for (char *sub_tok; (sub_tok = strtok_r(nullptr, " ", &tok)) != nullptr;)
				token.push_back(sub_tok);
			cur = end + 1;
		} else if (tok[0] == '*') {
			token.push_back(nullptr);
		} else {
			token.push_back(tok);
		}
		arr.push_back(std::move(token));
	}
	return true;
}

// Check array size and all args listed in 'ones' have size = 1 (no multiple entries)
template <int size, int ...ones>
static bool check_tokens(vector<vector<char *>> &arr) {
	if (arr.size() != size)
		return false;
	initializer_list<int> list{ones...};
	for (int i : list)
		if (arr[i].size() != 1)
			return false;
	return true;
}

template <int size, int ...ones>
static bool tokenize_and_check(char *stmt, vector<vector<char *>> &arr) {
	return tokenize_string(stmt, arr) && check_tokens<size, ones...>(arr);
}

template <typename Func, typename ...Args>
static void run_and_check(const Func &fn, const char *action, Args ...args) {
	if (fn(args...)) {
		string s = "Error in: %s";
		for (int i = 0; i < sizeof...(args); ++i) s += " %s";
		s += "\n";
		LOGW(s.data(), action, (args ? args : "*")...);
	}
}

#define run_fn(...) run_and_check(fn, action, __VA_ARGS__)

// Pattern 1: action { source } { target } { class } { permission }
template <typename Func>
static bool parse_pattern_1(const Func &fn, const char *action, char *stmt) {
	vector<vector<char *>> arr;
	if (!tokenize_and_check<4>(stmt, arr))
		return false;
	for (char *src : arr[0])
		for (char *tgt : arr[1])
			for (char *cls : arr[2])
				for (char *perm : arr[3])
					run_fn(src, tgt, cls, perm);
	return true;
}

// Pattern 2: action { source } { target } { class } ioctl range
template <typename Func>
static bool parse_pattern_2(const Func &fn, const char *action, char *stmt) {
	vector<vector<char *>> arr;
	if (!tokenize_and_check<5, 3, 4>(stmt, arr) || arr[3][0] != "ioctl"sv)
		return false;
	char *range = arr[4][0];
	for (char *src : arr[0])
		for (char *tgt : arr[1])
			for (char *cls : arr[2])
				run_fn(src, tgt, cls, range);
	return true;
}

// Pattern 3: action { type }
template <typename Func>
static bool parse_pattern_3(const Func &fn, const char *action, char *stmt) {
	vector<vector<char *>> arr;
	if (!tokenize_and_check<1>(stmt, arr))
		return false;
	for (char *type : arr[0])
		run_fn(type);
	return true;
}

// Pattern 4: action { type } { attribute }
template <typename Func>
static bool parse_pattern_4(const Func &fn, const char *action, char *stmt) {
	vector<vector<char *>> arr;
	if (!tokenize_and_check<2>(stmt, arr))
		return false;
	for (char *type : arr[0])
		for (char *attr : arr[1])
			run_fn(type, attr);
	return true;
}

// Pattern 5: action source target class default
template <typename Func>
static bool parse_pattern_5(const Func &fn, const char *action, char *stmt) {
	vector<vector<char *>> arr;
	if (!tokenize_and_check<4, 0, 1, 2, 3>(stmt, arr))
		return false;
	run_fn(arr[0][0], arr[1][0], arr[2][0], arr[3][0]);
	return true;
}

// Pattern 6: action source target class default (filename)
template <typename Func>
static bool parse_pattern_6(const Func &fn, const char *action, char *stmt) {
	vector<vector<char *>> arr;
	if (!tokenize_string(stmt, arr))
		return false;
	if (arr.size() == 4)
		arr.emplace_back(initializer_list<char*>{nullptr});
	if (!check_tokens<5, 0, 1, 2, 3, 4>(arr))
		return false;
	run_fn(arr[0][0], arr[1][0], arr[2][0], arr[3][0], arr[4][0]);
	return true;
}

// Pattern 7: action name path context
template <typename Func>
static bool parse_pattern_7(const Func &fn, const char *action, char *stmt) {
	vector<vector<char *>> arr;
	if (!tokenize_and_check<3, 0, 1, 2>(stmt, arr))
		return false;
	run_fn(arr[0][0], arr[1][0], arr[2][0]);
	return true;
}

#define add_action_func(name, type, fn) \
else if (strcmp(name, action) == 0) { \
	auto __fn = [=](auto && ...args){ return (fn)(args...); };\
	if (!parse_pattern_##type(__fn, name, remain)) \
		LOGW("Syntax error in '%s'\n\n%s\n", stmt, type_msg_##type); \
}

#define add_action(act, type) add_action_func(#act, type, act)

void sepolicy::parse_statement(const char *stmt) {
	// strtok modify strings, create a copy
	string cpy(stmt);

	char *remain;
	char *action = strtok_r(cpy.data(), " ", &remain);
	if (remain == nullptr) {
		LOGW("Syntax error in '%s'\n\n", stmt);
		return;
	}

	if (0) {}
	add_action(allow, 1)
	add_action(deny, 1)
	add_action(auditallow, 1)
	add_action(dontaudit, 1)
	add_action(allowxperm, 2)
	add_action(auditallowxperm, 2)
	add_action(dontauditxperm, 2)
	add_action(create, 3)
	add_action(permissive, 3)
	add_action(enforce, 3)
	add_action(typeattribute, 4)
	add_action(type_change, 5)
	add_action(type_member, 5)
	add_action(type_transition, 6)
	add_action(genfscon, 7)

	// Backwards compatible syntax
	add_action_func("attradd", 4, typeattribute)
	add_action_func("name_transition", 6, type_transition)

	else { LOGW("Unknown action: '%s'\n\n", action); }
}

void sepolicy::load_rule_file(const char *file) {
	file_readline(true, file, [=](string_view line) -> bool {
		if (line.empty() || line[0] == '#')
			return true;
		parse_statement(line.data());
		return true;
	});
}
