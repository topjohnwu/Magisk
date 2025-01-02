#include <unistd.h>
#include <sys/stat.h>

#include <consts.hpp>
#include <base.hpp>
#include <db.hpp>
#include <sqlite.hpp>
#include <core.hpp>

#define DB_VERSION     12
#define DB_VERSION_STR "12"

using namespace std;

#define DBLOGV(...)
//#define DBLOGV(...) LOGD("magiskdb: " __VA_ARGS__)

#define sql_chk_log(fn, ...) if (int rc = fn(__VA_ARGS__); rc != SQLITE_OK) { \
    LOGE("sqlite3(db.cpp:%d): %s\n", __LINE__, sqlite3_errstr(rc));           \
    return false;                                                             \
}

static bool open_and_init_db_impl(sqlite3 **dbOut) {
    if (!load_sqlite()) {
        LOGE("sqlite3: Cannot load libsqlite.so\n");
        return false;
    }

    unique_ptr<sqlite3, decltype(sqlite3_close)> db(nullptr, sqlite3_close);
    {
        sqlite3 *sql;
        sql_chk_log(sqlite3_open_v2, MAGISKDB, &sql,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
        db.reset(sql);
    }

    int ver = 0;
    bool upgrade = false;
    auto ver_cb = [](void *ver, auto, DbValues &data) {
        *static_cast<int *>(ver) = data.get_int(0);
    };
    sql_chk_log(sql_exec, db.get(), "PRAGMA user_version", nullptr, nullptr, ver_cb, &ver);
    if (ver > DB_VERSION) {
        // Don't support downgrading database
        LOGE("sqlite3: Downgrading database is not supported\n");
        return false;
    }

    auto create_policy = [&] {
        return sql_exec(db.get(),
                "CREATE TABLE IF NOT EXISTS policies "
                "(uid INT, policy INT, until INT, logging INT, "
                "notification INT, PRIMARY KEY(uid))");
    };
    auto create_settings = [&] {
        return sql_exec(db.get(),
                "CREATE TABLE IF NOT EXISTS settings "
                "(key TEXT, value INT, PRIMARY KEY(key))");
    };
    auto create_strings = [&] {
        return sql_exec(db.get(),
                "CREATE TABLE IF NOT EXISTS strings "
                "(key TEXT, value TEXT, PRIMARY KEY(key))");
    };
    auto create_denylist = [&] {
        return sql_exec(db.get(),
                "CREATE TABLE IF NOT EXISTS denylist "
                "(package_name TEXT, process TEXT, PRIMARY KEY(package_name, process))");
    };

    // Database changelog:
    //
    // 0 - 6: DB stored in app private data. There are no longer any code in the project to
    //        migrate these data, so no need to take any of these versions into consideration.
    // 7 : create table `hidelist` (process TEXT, PRIMARY KEY(process))
    // 8 : add new column (package_name TEXT) to table `hidelist`
    // 9 : rebuild table `hidelist` to change primary key (PRIMARY KEY(package_name, process))
    // 10: remove table `logs`
    // 11: remove table `hidelist` and create table `denylist` (same data structure)
    // 12: rebuild table `policies` to drop column `package_name`

    if (/* 0, 1, 2, 3, 4, 5, 6 */ ver <= 6) {
        sql_chk_log(create_policy);
        sql_chk_log(create_settings);
        sql_chk_log(create_strings);
        sql_chk_log(create_denylist);

        // Directly jump to latest
        ver = DB_VERSION;
        upgrade = true;
    }
    if (ver == 7) {
        sql_chk_log(sql_exec, db.get(),
                "BEGIN TRANSACTION;"
                "ALTER TABLE hidelist RENAME TO hidelist_tmp;"
                "CREATE TABLE IF NOT EXISTS hidelist "
                "(package_name TEXT, process TEXT, PRIMARY KEY(package_name, process));"
                "INSERT INTO hidelist SELECT process as package_name, process FROM hidelist_tmp;"
                "DROP TABLE hidelist_tmp;"
                "COMMIT;");
        // Directly jump to version 9
        ver = 9;
        upgrade = true;
    }
    if (ver == 8) {
        sql_chk_log(sql_exec, db.get(),
                "BEGIN TRANSACTION;"
                "ALTER TABLE hidelist RENAME TO hidelist_tmp;"
                "CREATE TABLE IF NOT EXISTS hidelist "
                "(package_name TEXT, process TEXT, PRIMARY KEY(package_name, process));"
                "INSERT INTO hidelist SELECT * FROM hidelist_tmp;"
                "DROP TABLE hidelist_tmp;"
                "COMMIT;");
        ver = 9;
        upgrade = true;
    }
    if (ver == 9) {
        sql_chk_log(sql_exec, db.get(), "DROP TABLE IF EXISTS logs", nullptr, nullptr);
        ver = 10;
        upgrade = true;
    }
    if (ver == 10) {
        sql_chk_log(sql_exec, db.get(),
                "DROP TABLE IF EXISTS hidelist;"
                "DELETE FROM settings WHERE key='magiskhide';");
        sql_chk_log(create_denylist);
        ver = 11;
        upgrade = true;
    }
    if (ver == 11) {
        sql_chk_log(sql_exec, db.get(),
                "BEGIN TRANSACTION;"
                "ALTER TABLE policies RENAME TO policies_tmp;"
                "CREATE TABLE IF NOT EXISTS policies "
                "(uid INT, policy INT, until INT, logging INT, "
                "notification INT, PRIMARY KEY(uid));"
                "INSERT INTO policies "
                "SELECT uid, policy, until, logging, notification FROM policies_tmp;"
                "DROP TABLE policies_tmp;"
                "COMMIT;");
        ver = 12;
        upgrade = true;
    }

    if (upgrade) {
        // Set version
        sql_chk_log(sql_exec, db.get(), "PRAGMA user_version=" DB_VERSION_STR);
    }

    *dbOut = db.release();
    return true;
}

sqlite3 *open_and_init_db() {
    sqlite3 *db = nullptr;
    return open_and_init_db_impl(&db) ? db : nullptr;
}

static sqlite3 *get_db() {
    static sqlite3 *db = nullptr;
    if (db == nullptr) {
        db = open_and_init_db();
        if (db == nullptr) {
            // Open failed, remove and reconstruct
            unlink(MAGISKDB);
            db = open_and_init_db();
        }
    }
    return db;
}

bool db_exec(const char *sql, DbArgs args, db_exec_callback exec_fn) {
    using db_bind_callback = std::function<int(int, DbStatement&)>;

    if (sqlite3 *db = get_db()) {
        db_bind_callback bind_fn = {};
        sql_bind_callback bind_cb = nullptr;
        if (!args.empty()) {
            bind_fn = std::ref(args);
            bind_cb = [](void *v, int index, DbStatement &stmt) -> int {
                auto fn = static_cast<db_bind_callback*>(v);
                return fn->operator()(index, stmt);
            };
        }
        sql_exec_callback exec_cb = nullptr;
        if (exec_fn) {
            exec_cb = [](void *v, StringSlice columns, DbValues &data) {
                auto fn = static_cast<db_exec_callback*>(v);
                fn->operator()(columns, data);
            };
        }
        sql_chk_log(sql_exec, db, sql, bind_cb, &bind_fn, exec_cb, &exec_fn);
        return true;
    }
    return false;
}

bool get_db_settings(db_settings &cfg, int key) {
    if (key >= 0) {
        return db_exec("SELECT * FROM settings WHERE key=?", { DB_SETTING_KEYS[key] }, cfg);
    } else {
        return db_exec("SELECT * FROM settings", {}, cfg);
    }
}

bool set_db_settings(int key, int value) {
    return db_exec(
            "INSERT OR REPLACE INTO settings (key,value) VALUES(?,?)",
            { DB_SETTING_KEYS[key], value });
}

bool get_db_strings(db_strings &str, int key) {
    if (key >= 0) {
        return db_exec("SELECT * FROM strings WHERE key=?", { DB_STRING_KEYS[key] }, str);
    } else {
        return db_exec("SELECT * FROM strings", {}, str);
    }
}

bool rm_db_strings(int key) {
    return db_exec("DELETE FROM strings WHERE key=?", { DB_STRING_KEYS[key] });
}

void exec_sql(owned_fd client) {
    string sql = read_string(client);
    db_exec(sql.data(), {}, [fd = (int) client](StringSlice columns, DbValues &data) {
        string out;
        for (int i = 0; i < columns.size(); ++i) {
            if (i != 0) out += '|';
            out += columns[i].c_str();
            out += '=';
            out += data.get_text(i);
        }
        write_string(fd, out);
    });
    write_int(client, 0);
}

db_settings::db_settings() :
    root_access(ROOT_ACCESS_APPS_AND_ADB),
    multiuser_mode(MULTIUSER_MODE_OWNER_ONLY),
    mnt_ns(NAMESPACE_MODE_REQUESTER),
    bootloop(0),
    denylist(false),
    zygisk(MagiskD().is_emulator()) {}

void db_settings::operator()(StringSlice columns, DbValues &data) {
    string_view key;
    int val;
    for (int i = 0; i < columns.size(); ++i) {
        const auto &name = columns[i];
        if (name == "key") {
            key = data.get_text(i);
        } else if (name == "value") {
            val = data.get_int(i);
        }
    }
    if (key == DB_SETTING_KEYS[ROOT_ACCESS]) {
        root_access = val;
    } else if (key == DB_SETTING_KEYS[SU_MULTIUSER_MODE]) {
        multiuser_mode = val;
    } else if (key == DB_SETTING_KEYS[SU_MNT_NS]) {
        mnt_ns = val;
    } else if (key == DB_SETTING_KEYS[BOOTLOOP_COUNT]) {
        bootloop = val;
    } else if (key == DB_SETTING_KEYS[DENYLIST_CONFIG]) {
        denylist = val;
    } else if (key == DB_SETTING_KEYS[ZYGISK_CONFIG]) {
        zygisk = val;
    }
}

void db_strings::operator()(StringSlice columns, DbValues &data) {
    string_view key;
    const char *val;
    for (int i = 0; i < columns.size(); ++i) {
        const auto &name = columns[i];
        if (name == "key") {
            key = data.get_text(i);
        } else if (name == "value") {
            val = data.get_text(i);
        }
    }
    if (key == DB_STRING_KEYS[SU_MANAGER]) {
        su_manager = val;
    }
}

int DbArgs::operator()(int index, DbStatement &stmt) {
    if (curr < args.size()) {
        const auto &arg = args[curr++];
        switch (arg.type) {
            case DbArg::INT:
                return stmt.bind_int64(index, arg.int_val);
            case DbArg::TEXT:
                return stmt.bind_text(index, arg.str_val);
        }
    }
    return SQLITE_OK;
}
