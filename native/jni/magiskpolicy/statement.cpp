#include <cstring>
#include <vector>
#include <string>

#include <magiskpolicy.hpp>
#include <utils.hpp>

#include "sepolicy.hpp"

using namespace std;

static const char *type_msg_1 =
R"EOF("allow *source_type *target_type *class *perm_set"
"deny *source_type *target_type *class *perm_set"
"auditallow *source_type *target_type *class *perm_set"
"dontaudit *source_type *target_type *class *perm_set"
)EOF";

static const char *type_msg_2 =
R"EOF("allowxperm *source_type *target_type *class operation xperm_set"
"auditallowxperm *source_type *target_type *class operation xperm_set"
"dontauditxperm *source_type *target_type *class operation xperm_set"
- The only supported operation is 'ioctl'
- xperm_set format is either 'low-high', 'value', or '*'.
  '*' will be treated as '0x0000-0xFFFF'.
  All values should be written in hexadecimal.
)EOF";

static const char *type_msg_3 =
R"EOF("permissive ^type"
"enforce ^type"
)EOF";

static const char *type_msg_4 =
R"EOF("typeattribute ^type ^attribute"
)EOF";

static const char *type_msg_5 =
R"EOF("type type_name ^(attribute)"
- Argument 'attribute' is optional, default to 'domain'
)EOF";

static const char *type_msg_6 =
R"EOF("attribute attribute_name"
)EOF";

static const char *type_msg_7 =
R"EOF("type_transition source_type target_type class default_type (object_name)"
- Argument 'object_name' is optional
)EOF";

static const char *type_msg_8 =
R"EOF("type_change source_type target_type class default_type"
"type_member source_type target_type class default_type"
)EOF";

static const char *type_msg_9 =
R"EOF("genfscon fs_name partial_path fs_context"
)EOF";

void statement_help() {
    fprintf(stderr,
R"EOF(One policy statement should be treated as one parameter;
this means each policy statement should be enclosed in quotes.
Multiple policy statements can be provided in a single command.

Statements has a format of "<rule_name> [args...]".
Arguments labeled with (^) can accept one or more entries. Multiple
entries consist of a space separated list enclosed in braces ({}).
Arguments labeled with (*) are the same as (^), but additionally
support the match-all operator (*).

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
%s
%s
)EOF", type_msg_1, type_msg_2, type_msg_3, type_msg_4,
type_msg_5, type_msg_6, type_msg_7, type_msg_8, type_msg_9);
    exit(0);
}

using parsed_tokens = vector<vector<const char *>>;

static bool tokenize_string(char *stmt, parsed_tokens &arr) {
    // cur is the pointer to where the top level is parsing
    char *cur = stmt;
    for (char *tok; (tok = strtok_r(nullptr, " ", &cur)) != nullptr;) {
        vector<const char *> token;
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
static bool check_tokens(parsed_tokens &arr) {
    if (arr.size() != size)
        return false;
    initializer_list<int> list{ones...};
    for (int i : list)
        if (arr[i].size() != 1)
            return false;
    return true;
}

template <int size, int ...ones>
static bool tokenize_and_check(char *stmt, parsed_tokens &arr) {
    return tokenize_string(stmt, arr) && check_tokens<size, ones...>(arr);
}

template <typename Func, typename ...Args>
static void run_and_check(const Func &fn, const char *action, Args ...args) {
    if (!fn(args...)) {
        string s = "Error in: %s";
        for (int i = 0; i < sizeof...(args); ++i) s += " %s";
        s += "\n";
        LOGW(s.data(), action, (args ? args : "*")...);
    }
}

#define run_fn(...) run_and_check(fn, action, __VA_ARGS__)

// Pattern 1: allow { source } { target } { class } { permission }
template <typename Func>
static bool parse_pattern_1(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
    if (!tokenize_and_check<4>(stmt, arr))
        return false;
    for (auto src : arr[0])
        for (auto tgt : arr[1])
            for (auto cls : arr[2])
                for (auto perm : arr[3])
                    run_fn(src, tgt, cls, perm);
    return true;
}

// Pattern 2: allowxperm { source } { target } { class } ioctl range
template <typename Func>
static bool parse_pattern_2(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
    if (!tokenize_and_check<5, 3, 4>(stmt, arr) || arr[3][0] != "ioctl"sv)
        return false;
    auto range = arr[4][0];
    for (auto src : arr[0])
        for (auto tgt : arr[1])
            for (auto cls : arr[2])
                run_fn(src, tgt, cls, range);
    return true;
}

// Pattern 3: permissive { type }
template <typename Func>
static bool parse_pattern_3(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
    if (!tokenize_and_check<1>(stmt, arr))
        return false;
    for (auto type : arr[0])
        run_fn(type);
    return true;
}

// Pattern 4: typeattribute { type } { attribute }
template <typename Func>
static bool parse_pattern_4(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
    if (!tokenize_and_check<2>(stmt, arr))
        return false;
    for (auto type : arr[0])
        for (auto attr : arr[1])
            run_fn(type, attr);
    return true;
}

// Pattern 5: type name { attribute }
template <typename Func>
static bool parse_pattern_5(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
    string tmp_str;
    if (!tokenize_string(stmt, arr))
        return false;
    if (arr.size() == 1) {
        arr.emplace_back(initializer_list<const char*>{ "domain" });
    }
    if (!check_tokens<2, 0>(arr))
        return false;
    for (auto attr : arr[1])
        run_fn(arr[0][0], attr);
    return true;
}

// Pattern 6: attribute name
template <typename Func>
static bool parse_pattern_6(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
    if (!tokenize_and_check<1, 0>(stmt, arr))
        return false;
    run_fn(arr[0][1]);
    return true;
}

// Pattern 7: type_transition source target class default (filename)
template <typename Func>
static bool parse_pattern_7(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
    if (!tokenize_string(stmt, arr))
        return false;
    if (arr.size() == 4)
        arr.emplace_back(initializer_list<const char*>{nullptr});
    if (!check_tokens<5, 0, 1, 2, 3, 4>(arr))
        return false;
    run_fn(arr[0][0], arr[1][0], arr[2][0], arr[3][0], arr[4][0]);
    return true;
}

// Pattern 8: type_change source target class default
template <typename Func>
static bool parse_pattern_8(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
    if (!tokenize_and_check<4, 0, 1, 2, 3>(stmt, arr))
        return false;
    run_fn(arr[0][0], arr[1][0], arr[2][0], arr[3][0]);
    return true;
}

// Pattern 9: genfscon name path context
template <typename Func>
static bool parse_pattern_9(const Func &fn, const char *action, char *stmt) {
    parsed_tokens arr;
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
    add_action(permissive, 3)
    add_action(enforce, 3)
    add_action(typeattribute, 4)
    add_action(type, 5)
    add_action(attribute, 6)
    add_action(type_transition, 7)
    add_action(type_change, 8)
    add_action(type_member, 8)
    add_action(genfscon, 9)

    // Backwards compatible syntax
    add_action(create, 3)
    add_action_func("attradd", 4, typeattribute)
    add_action_func("name_transition", 7, type_transition)

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
