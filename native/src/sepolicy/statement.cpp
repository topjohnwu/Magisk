#include <cstring>
#include <vector>
#include <string>

#include <base.hpp>

#include "policy.hpp"

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
