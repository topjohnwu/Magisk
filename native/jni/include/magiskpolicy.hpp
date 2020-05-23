#pragma once

#include <stdlib.h>
#include <selinux.hpp>

#define ALL nullptr

struct policydb;

class sepolicy {
public:
	typedef const char * c_str;
	~sepolicy();

	// Public static factory functions
	static sepolicy *from_file(c_str file);
	static sepolicy *from_split();
	static sepolicy *compile_split();

	// External APIs
	int to_file(c_str file);
	void parse_statement(c_str stmt);
	void load_rule_file(c_str file);

	// Operation on types
	int create(c_str type);
	int permissive(c_str type);
	int enforce(c_str type);
	int typeattribute(c_str type, c_str attr);
	int exists(c_str type);

	// Access vector rules
	int allow(c_str src, c_str tgt, c_str cls, c_str perm);
	int deny(c_str src, c_str tgt, c_str cls, c_str perm);
	int auditallow(c_str src, c_str tgt, c_str cls, c_str perm);
	int dontaudit(c_str src, c_str tgt, c_str cls, c_str perm);

	// Extended permissions access vector rules
	int allowxperm(c_str src, c_str tgt, c_str cls, c_str range);
	int auditallowxperm(c_str src, c_str tgt, c_str cls, c_str range);
	int dontauditxperm(c_str src, c_str tgt, c_str cls, c_str range);

	// Type rules
	int type_transition(c_str s, c_str t, c_str c, c_str d, c_str o = nullptr);
	int type_change(c_str src, c_str tgt, c_str cls, c_str def);
	int type_member(c_str src, c_str tgt, c_str cls, c_str def);

	// File system labeling
	int genfscon(c_str fs_name, c_str path, c_str ctx);

	// Magisk
	void magisk_rules();

protected:
	policydb *db;
};
