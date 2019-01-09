#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <sys/stat.h>

#define db_err(e) db_err_cmd(e, )
#define db_err_cmd(e, cmd) if (e) { \
	LOGE("sqlite3_exec: %s\n", e); \
	sqlite3_free(e); \
	cmd;\
}

/***************
 * DB Settings *
 ***************/

#define DB_SETTING_KEYS \
((const char *[]) { \
"root_access", \
"multiuser_mode", \
"mnt_ns", \
"magiskhide", \
})

#define DB_SETTINGS_NUM 4

// Settings keys
enum {
	ROOT_ACCESS = 0,
	SU_MULTIUSER_MODE,
	SU_MNT_NS,
	HIDE_CONFIG
};

// Values for root_access
enum {
	ROOT_ACCESS_DISABLED = 0,
	ROOT_ACCESS_APPS_ONLY,
	ROOT_ACCESS_ADB_ONLY,
	ROOT_ACCESS_APPS_AND_ADB
};

// Values for multiuser_mode
enum {
	MULTIUSER_MODE_OWNER_ONLY = 0,
	MULTIUSER_MODE_OWNER_MANAGED,
	MULTIUSER_MODE_USER
};

// Values for mnt_ns
enum {
	NAMESPACE_MODE_GLOBAL = 0,
	NAMESPACE_MODE_REQUESTER,
	NAMESPACE_MODE_ISOLATE
};

class db_settings {
public:
	db_settings();
	int& operator [](const char *);
	const int& operator [](const char *) const;
	int& operator [](const int);
	const int& operator [](const int) const;

private:
	int data[DB_SETTINGS_NUM + 1];
	int getKeyIdx(const char *) const;
};

/**************
 * DB Strings *
 **************/

#define DB_STRING_KEYS \
((const char *[]) { \
"requester", \
})

#define DB_STRING_NUM 1

// Strings keys
enum {
	SU_MANAGER = 0
};

class db_strings {
public:
	db_strings();
	char * operator [](const char *);
	const char * operator [](const char *) const;
	char * operator [](const int);
	const char * operator [](const int) const;

private:
	char data[DB_STRING_NUM + 1][128];
	int getKeyIdx(const char *) const;
};

/*************
 * SU Access *
 *************/

typedef enum {
	QUERY = 0,
	DENY = 1,
	ALLOW = 2,
} policy_t;

struct su_access {
	policy_t policy;
	int log;
	int notify;
};

#define DEFAULT_SU_ACCESS (su_access) { \
.policy = QUERY, \
.log = 1, \
.notify = 1 \
}

#define SILENT_SU_ACCESS (su_access) { \
.policy = ALLOW, \
.log = 0, \
.notify = 0 \
}

#define NO_SU_ACCESS (su_access) { \
.policy = DENY, \
.log = 0, \
.notify = 0 \
}

/********************
 * Public Functions *
 ********************/

sqlite3 *get_magiskdb(sqlite3 *&db);
int get_db_settings(db_settings *dbs, int key = -1);
int get_db_strings(db_strings *str, int key = -1);
int get_uid_policy(int uid, struct su_access *su);
int validate_manager(char *alt_pkg, int userid, struct stat *st);
void exec_sql(int client);
char *db_exec(const char *sql, int (*cb)(void *, int, char**, char**) = nullptr, void *v = nullptr);

#endif //DB_H
