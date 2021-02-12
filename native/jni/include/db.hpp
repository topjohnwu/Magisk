#pragma once

#include <sys/stat.h>
#include <map>
#include <string>
#include <string_view>
#include <functional>

template <class T, size_t num>
class db_data_base {
public:
    T& operator [](std::string_view key) {
        return data[getKeyIdx(key)];
    }

    const T& operator [](std::string_view key) const {
        return data[getKeyIdx(key)];
    }

    T& operator [](int key) {
        return data[key];
    }

    const T& operator [](int key) const {
        return data[key];
    }

protected:
    T data[num + 1];
    virtual int getKeyIdx(std::string_view key) const = 0;
};

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

class db_settings : public db_data_base<int, DB_SETTINGS_NUM> {
public:
    db_settings();

protected:
    int getKeyIdx(std::string_view key) const override;
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

class db_strings : public db_data_base<std::string, DB_STRING_NUM> {
protected:
    int getKeyIdx(std::string_view key) const override;
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

typedef std::map<std::string_view, std::string_view> db_row;
typedef std::function<bool(db_row&)> db_row_cb;

int get_db_settings(db_settings &cfg, int key = -1);
int get_db_strings(db_strings &str, int key = -1);
int get_uid_policy(su_access &su, int uid);
bool check_manager(std::string *pkg = nullptr);
bool validate_manager(std::string &pkg, int userid, struct stat *st);
void exec_sql(int client);
char *db_exec(const char *sql);
char *db_exec(const char *sql, const db_row_cb &fn);
bool db_err(char *e);

#define db_err_cmd(e, cmd) if (db_err(e)) { cmd; }
