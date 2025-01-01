#pragma once

#include <sys/stat.h>
#include <map>
#include <string>
#include <string_view>
#include <functional>

#include <base.hpp>
#include <sqlite.hpp>

/***************
 * DB Settings *
 ***************/

constexpr const char *DB_SETTING_KEYS[] = {
    "root_access",
    "multiuser_mode",
    "mnt_ns",
    "denylist",
    "zygisk",
    "bootloop",
};

// Settings key indices
enum {
    ROOT_ACCESS = 0,
    SU_MULTIUSER_MODE,
    SU_MNT_NS,
    DENYLIST_CONFIG,
    ZYGISK_CONFIG,
    BOOTLOOP_COUNT,
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

struct db_settings {
    int root_access;
    int multiuser_mode;
    int mnt_ns;
    int bootloop;
    bool denylist;
    bool zygisk;

    db_settings();
    void operator()(StringSlice columns, DbValues &data);
};

/**************
 * DB Strings *
 **************/

constexpr const char *DB_STRING_KEYS[] = { "requester" };

// Strings keys indices
enum {
    SU_MANAGER = 0
};

struct db_strings {
    std::string su_manager;

    void operator()(StringSlice columns, DbValues &data);
};

/********************
 * Public Functions *
 ********************/

using db_exec_callback = std::function<void(StringSlice, DbValues&)>;
using db_bind_callback = std::function<void(int, DbStatement&)>;

int get_db_settings(db_settings &cfg, int key = -1);
int set_db_settings(int key, int value);
int get_db_strings(db_strings &str, int key = -1);
void rm_db_strings(int key);
void exec_sql(owned_fd client);
bool db_exec(const char *sql, db_bind_callback bind_fn = {}, db_exec_callback exec_fn = {});

static inline bool db_exec(const char *sql, db_exec_callback exec_fn) {
    return db_exec(sql, {}, std::move(exec_fn));
}

template<typename T>
concept DbData = requires(T t, StringSlice s, DbValues &v) { t(s, v); };

template<DbData T>
bool db_exec(const char *sql, T &data) {
    return db_exec(sql, (db_exec_callback) std::ref(data));
}
