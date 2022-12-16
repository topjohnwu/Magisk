#pragma once

#include <sys/stat.h>
#include <map>
#include <string>
#include <string_view>
#include <functional>

template <class T, size_t N>
class db_dict {
public:
    T& operator [](std::string_view key) {
        return data[get_idx(key)];
    }

    const T& operator [](std::string_view key) const {
        return data[get_idx(key)];
    }

    T& operator [](int key) {
        return data[key];
    }

    const T& operator [](int key) const {
        return data[key];
    }

protected:
    T data[N + 1];
    virtual int get_idx(std::string_view key) const = 0;
};

/***************
 * DB Settings *
 ***************/

constexpr const char *DB_SETTING_KEYS[] = {
    "root_access",
    "multiuser_mode",
    "mnt_ns",
    "denylist",
    "zygisk"
};

// Settings key indices
enum {
    ROOT_ACCESS = 0,
    SU_MULTIUSER_MODE,
    SU_MNT_NS,
    DENYLIST_CONFIG,
    ZYGISK_CONFIG
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

class db_settings : public db_dict<int, std::size(DB_SETTING_KEYS)> {
public:
    db_settings();
protected:
    int get_idx(std::string_view key) const override;
};

/**************
 * DB Strings *
 **************/

constexpr const char *DB_STRING_KEYS[] = { "requester" };

// Strings keys indices
enum {
    SU_MANAGER = 0
};

class db_strings : public db_dict<std::string, std::size(DB_STRING_KEYS)> {
protected:
    int get_idx(std::string_view key) const override;
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

#define DEFAULT_SU_ACCESS { QUERY, 1, 1 }
#define SILENT_SU_ACCESS  { ALLOW, 0, 0 }
#define NO_SU_ACCESS      { DENY,  0, 0 }

/********************
 * Public Functions *
 ********************/

using db_row = std::map<std::string_view, std::string_view>;
using db_row_cb = std::function<bool(db_row&)>;

int get_db_settings(db_settings &cfg, int key = -1);
int get_db_strings(db_strings &str, int key = -1);
void rm_db_strings(int key);
void exec_sql(int client);
char *db_exec(const char *sql);
char *db_exec(const char *sql, const db_row_cb &fn);
bool db_err(char *e);

#define db_err_cmd(e, cmd) if (db_err(e)) { cmd; }
