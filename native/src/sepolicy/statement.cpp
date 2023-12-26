#include <cstring>
#include <vector>
#include <string>

#include <base.hpp>

#include "policy.hpp"

using namespace std;

const argument &all_xperm() {
    static argument arg;
    if (arg.first.empty())
        arg.first.push_back(nullptr);
    return arg;
}

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
- The only supported operation right now is 'ioctl'
- xperm_set is one or multiple hexadecimal numeric values ranging from 0x0000 to 0xFFFF.
  Multiple values consist of a space separated list enclosed in braces ({}).
  Use the complement operator (~) to specify all permissions except those explicitly listed.
  Use the range operator (-) to specify all permissions within the low – high range.
  Use the match all operator (*) to match all ioctl commands.
  The special value 0 is used to clear all rules.
  Some examples:
  allowxperm source target class ioctl 0x8910
  allowxperm source target class ioctl { 0x8910-0x8926 0x892A-0x8935 }
  allowxperm source target class ioctl ~{ 0x8910 0x892A }
  allowxperm source target class ioctl *
)EOF";

static const char *type_msg_3 =
R"EOF("permissive *type"
"enforce *type"
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
Arguments labeled with (^) can accept one or more entries.
Multiple entries consist of a space separated list enclosed in braces ({}).
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

// Return value:
//  0: success
// -1: unclosed bracket
// -2: nested brackets
// -3: double complement
static int tokenize_string(char *stmt, argument_list &args) {
    char *cur = stmt;
    bool complement = false;

    auto add_new_arg = [&] (char *str) {
        argument arg;
        for (char *tok; (tok = strtok_r(nullptr, " ", &str)) != nullptr;) {
            if (tok == "*"sv) {
                // If any of the tokens is "*", the result is a single nullptr
                arg.first.clear();
                arg.first.push_back(nullptr);
                break;
            } else {
                arg.first.push_back(tok);
            }
        }
        arg.second = complement;
        complement = false;
        args.push_back(std::move(arg));
    };

    char *tok = strtok_r(nullptr, " ", &cur);
    while (tok) {
        if (tok[0] == '~') {
            if (complement) {
                // Double complement is not supported
                return -3;
            }
            complement = true;
            if (tok[1] == '\0') {
                // If '~' is followed by a space, find the next token
                tok = strtok_r(nullptr, " ", &cur);
            } else {
                // Reparse the rest of the string
                ++tok;
            }
            continue;
        }

        // Find brackets
        char *begin = strchr(tok, '{');
        char *end = strchr(tok, '}');

        if (begin == nullptr && end) {
            // Bracket not closed, syntax error
            return -1;
        }

        if (begin) {
            if (end && (begin > end)) {
                // Bracket not closed, syntax error
                return -1;
            }

            // Restore the string so we can properly find the closing bracket
            if (cur)
                cur[-1] = ' ';

            if (end == nullptr) {
                // Find again
                end = strchr(begin + 1, '}');
                if (end == nullptr) {
                    // Bracket not closed, syntax error
                    return -1;
                }
            }

            // Close bracket and start the next parsing iteration after that
            *end = '\0';
            cur = end + 1;

            if (strchr(begin + 1, '{')) {
                // We don't support nested brackets
                return -2;
            }

            if (begin != tok) {
                // There is an argument before the opening bracket
                *begin = '\0';
                add_new_arg(tok);
            }

            // Parse the arguments enclosed in the brackets
            add_new_arg(begin + 1);
        } else {
            add_new_arg(tok);
        }

        tok = strtok_r(nullptr, " ", &cur);
    }
    return 0;
}

// Check all args listed have size = 1 (no multiple entries)
template <int ...indices>
static bool enforce_single(const argument_list &args) {
    initializer_list<int> list{indices...};
    for (int i : list)
        if (args[i].first.size() != 1)
            return false;
    return true;
}

// Check all args listed are not null (no match all operator)
template <int ...indices>
static bool enforce_non_null(const argument_list &args) {
    initializer_list<int> list{indices...};
    for (int i : list)
        if (args[i].first.size() == 1 && args[i].first[0] == nullptr)
            return false;
    return true;
}

template <int size>
static bool enforce_size(const argument_list &args) {
    return args.size() == size;
}

// Check all args are not complements except those listed
template <int ...except>
static bool check_complements(const argument_list &args) {
    initializer_list<int> list{except...};
    for (int i = 0; i < args.size(); ++i) {
        bool disallow = true;
        for (int e : list) {
            if (i == e) {
                disallow = false;
                break;
            }
        }
        if (disallow && args[i].second)
            return false;
    }
    return true;
}

// Tokenize and check argument count
template <int size>
static bool tokenize_and_check(char *stmt, argument_list &args) {
    return tokenize_string(stmt, args) == 0 &&
        enforce_size<size>(args) &&
        check_complements<>(args);
}

#define sprint(...) off += ssprintf(buf + off, sizeof(buf) - off, __VA_ARGS__)

const char *as_str(const argument &arg) {
    size_t off = 0;
    static char buf[4096];

    if (arg.second) {
        sprint("~");
    }
    sprint("{ ");
    for (const char *tok : arg.first) {
        if (tok == nullptr) {
            sprint("0x0000-0xFF00 ");
        } else {
            sprint("%s ", tok);
        }
    }
    sprint("}");

    return buf;
}

const char *as_str(const char *arg) { return arg ?: "*"; }

template <typename Func, typename ...Args>
static void run_and_check(const Func &fn, const char *action, Args ...args) {
    if (!fn(args...)) {
        string s = "Error in: %s";
        for (int i = 0; i < sizeof...(args); ++i) s += " %s";
        s += "\n";
        LOGW(s.data(), action, as_str(args)...);
    }
}

#define run_fn(...) run_and_check(fn, action, __VA_ARGS__)

// Pattern 1: allow *{ source } *{ target } *{ class } *{ permission }
template <typename Func>
static bool parse_pattern_1(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    if (!tokenize_and_check<4>(stmt, args))
        return false;
    for (auto src : args[0].first)
        for (auto tgt : args[1].first)
            for (auto cls : args[2].first)
                for (auto perm : args[3].first)
                    run_fn(src, tgt, cls, perm);
    return true;
}

// Pattern 2: allowxperm *{ source } *{ target } *{ class } ioctl xperm_set
template <typename Func>
static bool parse_pattern_2(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    if (tokenize_string(stmt, args) != 0 ||
        !enforce_size<5>(args) ||
        !check_complements<4>(args) ||
        !enforce_single<3>(args) ||
        !enforce_non_null<3>(args) ||
        args[3].first[0] != "ioctl"sv)
        return false;
    for (auto src : args[0].first)
        for (auto tgt : args[1].first)
            for (auto cls : args[2].first)
                run_fn(src, tgt, cls, args[4]);
    return true;
}

// Pattern 3: permissive *{ type }
template <typename Func>
static bool parse_pattern_3(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    if (!tokenize_and_check<1>(stmt, args))
        return false;
    for (auto type : args[0].first)
        run_fn(type);
    return true;
}

// Pattern 4: typeattribute { type } { attribute }
template <typename Func>
static bool parse_pattern_4(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    if (!tokenize_and_check<2>(stmt, args) || !enforce_non_null<0, 1>(args))
        return false;
    for (auto type : args[0].first)
        for (auto attr : args[1].first)
            run_fn(type, attr);
    return true;
}

// Pattern 5: type name { attribute }
template <typename Func>
static bool parse_pattern_5(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    string tmp_str;
    if (tokenize_string(stmt, args) != 0 || !enforce_single<0>(args))
        return false;
    if (args.size() == 1) {
        args.emplace_back(make_pair(initializer_list<const char*>{ "domain" }, false));
    }
    if (!enforce_size<2>(args) || !enforce_non_null<1>(args) || !check_complements<>(args))
        return false;
    for (auto attr : args[1].first)
        run_fn(args[0].first[0], attr);
    return true;
}

// Pattern 6: attribute name
template <typename Func>
static bool parse_pattern_6(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    if (!tokenize_and_check<1>(stmt, args) || !enforce_single<0>(args))
        return false;
    run_fn(args[0].first[0]);
    return true;
}

// Pattern 7: type_transition source target class default (filename)
template <typename Func>
static bool parse_pattern_7(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    if (tokenize_string(stmt, args) != 0)
        return false;
    if (args.size() == 4)
        args.emplace_back(make_pair(initializer_list<const char*>{ nullptr }, false));
    if (!enforce_size<5>(args) ||
        !enforce_non_null<0, 1, 2, 3>(args) ||
        !enforce_single<0, 1, 2, 3, 4>(args) ||
        !check_complements<>(args))
        return false;
    run_fn(args[0].first[0], args[1].first[0], args[2].first[0],
           args[3].first[0], args[4].first[0]);
    return true;
}

// Pattern 8: type_change source target class default
template <typename Func>
static bool parse_pattern_8(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    if (!tokenize_and_check<4>(stmt, args) || !enforce_single<0, 1, 2, 3>(args))
        return false;
    run_fn(args[0].first[0], args[1].first[0], args[2].first[0], args[3].first[0]);
    return true;
}

// Pattern 9: genfscon name path context
template <typename Func>
static bool parse_pattern_9(const Func &fn, const char *action, char *stmt) {
    argument_list args;
    if (!tokenize_and_check<3>(stmt, args) || !enforce_single<0, 1, 2>(args))
        return false;
    run_fn(args[0].first[0], args[1].first[0], args[2].first[0]);
    return true;
}

#define add_action_func(name, type, fn) \
else if (strcmp(name, action) == 0) {   \
    auto __fn = [&](auto && ...args){ return (fn)(args...); }; \
    if (!parse_pattern_##type(__fn, name, remain))             \
        LOGW("Syntax error in '%.*s'\n\n%s\n", (int) stmt.length(), stmt.data(), type_msg_##type); \
}

#define add_action(act, type) add_action_func(#act, type, act)

void sepolicy::parse_statement(rust::Str stmt) {
    // strtok modify strings, create a copy
    string cpy(stmt.data(), stmt.length());

    char *remain;
    char *action = strtok_r(cpy.data(), " ", &remain);
    if (remain == nullptr) {
        LOGW("Syntax error in '%.*s'\n\n", (int) stmt.length(), stmt.data());
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

// Parsing is hard, the following is a test suite to ensure correctness

static void test_parse_stmt(const char *stmt, const char *expect, int code = 0) {
    auto print_error = [](int code) {
        switch (code) {
            case -1:
                fprintf(stderr, "unclosed bracket\n");
                break;
            case -2:
                fprintf(stderr, "nested brackets\n");
                break;
            case -3:
                fprintf(stderr, "double complement\n");
                break;
        }
    };

    char cpy[4096];
    strncpy(cpy, stmt, sizeof(cpy));
    argument_list args;
    int result = tokenize_string(cpy, args);
    if (result != 0) {
        if (expect != nullptr) {
            fprintf(stderr, "Parsing: '%s' with unexpected error: ", stmt);
            print_error(result);
            exit(1);
        } else {
            if (code != result) {
                fprintf(stderr, "Parsing: '%s'\n", stmt);
                fprintf(stderr, "Expect error: ");
                print_error(code);
                fprintf(stderr, "Result error: ");
                print_error(result);
                exit(1);
            }
            return;
        }
    }

    char buf[4096];
    size_t off = 0;
    for (const auto &arg : args) {
        sprint("(%d,[", arg.second);
        bool first = true;
        for (const char *tok : arg.first) {
            if (first) {
                first = false;
            } else {
                sprint(",");
            }
            sprint("'%s'", tok ?: "(null)");
        }
        sprint("])\n");
    }
    if (strncmp(buf, expect, sizeof(buf)) != 0) {
        fprintf(stderr, "Parsing: '%s'\n", stmt);
        fprintf(stderr, "Expect:\n%s", expect);
        fprintf(stderr, "-------------------\n");
        fprintf(stderr, "Result:\n%s", buf);
        fprintf(stderr, "-------------------\n");
        exit(1);
    }
}

static void assert_msg(bool b, const char *msg) {
    if (!b) {
        fprintf(stderr, "Assertion failed: %s\n", msg);
        exit(1);
    }
}

[[maybe_unused]]
void test_parse_statements() {
    // Test parsing correctness
    test_parse_stmt("a b c",
                    "(0,['a'])\n(0,['b'])\n(0,['c'])\n");
    test_parse_stmt("  a   b {c}",
                    "(0,['a'])\n(0,['b'])\n(0,['c'])\n");
    test_parse_stmt("a b{c }",
                    "(0,['a'])\n(0,['b'])\n(0,['c'])\n");
    test_parse_stmt("a b{c}d",
                    "(0,['a'])\n(0,['b'])\n(0,['c'])\n(0,['d'])\n");
    test_parse_stmt("a b {   c d  }",
                    "(0,['a'])\n(0,['b'])\n(0,['c','d'])\n");
    test_parse_stmt("a {b} ~c",
                    "(0,['a'])\n(0,['b'])\n(1,['c'])\n");
    test_parse_stmt("a b ~ { c d }",
                    "(0,['a'])\n(0,['b'])\n(1,['c','d'])\n");
    test_parse_stmt("a b *",
                    "(0,['a'])\n(0,['b'])\n(0,['(null)'])\n");
    test_parse_stmt("a b { c * }",
                    "(0,['a'])\n(0,['b'])\n(0,['(null)'])\n");

    // Invalid syntax tests
    test_parse_stmt("a b { c", nullptr, -1);
    test_parse_stmt("a {b}}c}", nullptr, -1);
    test_parse_stmt("a }b{c}", nullptr, -1);
    test_parse_stmt("{a} b } {c}", nullptr, -1);
    test_parse_stmt("a { b } { c", nullptr, -1);
    test_parse_stmt("a b {{ c }}", nullptr, -2);
    test_parse_stmt("a b ~ ~c", nullptr, -3);
    test_parse_stmt("a b ~~c", nullptr, -3);

    // Test enforcement functions
    string s = "a * { b c } { d e } ~f *";
    argument_list args;
    assert_msg(tokenize_string(s.data(), args) == 0, "parse failure");
    assert_msg(enforce_size<6>(args), "size != 6");
    assert_msg(enforce_non_null<0, 2, 3, 4>(args), "non-null enforcement failed");
    assert_msg(!enforce_non_null<0, 1, 2, 3, 4>(args), "non-null enforcement should fail");
    assert_msg(enforce_single<0, 1, 4, 5>(args), "single enforcement check failed");
    assert_msg(!enforce_single<0, 1, 2>(args), "single enforcement check should fail");
    assert_msg(check_complements<4>(args), "check complements failed");
    assert_msg(!check_complements<>(args), "check complements should fail");
}
