#include <unistd.h>
#include <sys/stat.h>

#include <consts.hpp>
#include <base.hpp>
#include <db.hpp>
#include <sqlite.hpp>
#include <core.hpp>

#define DB_VERSION 12

using namespace std;

static sqlite3 *mDB = nullptr;

#define DBLOGV(...)
//#define DBLOGV(...) LOGD("magiskdb: " __VA_ARGS__)

int db_strings::get_idx(string_view key) const {
    int idx = 0;
    for (const char *k : DB_STRING_KEYS) {
        if (key == k)
            break;
        ++idx;
    }
    return idx;
}

db_settings::db_settings() {
    // Default settings
    data[ROOT_ACCESS] = ROOT_ACCESS_APPS_AND_ADB;
    data[SU_MULTIUSER_MODE] = MULTIUSER_MODE_OWNER_ONLY;
    data[SU_MNT_NS] = NAMESPACE_MODE_REQUESTER;
    data[DENYLIST_CONFIG] = false;
    data[ZYGISK_CONFIG] = MagiskD().is_emulator();
    data[BOOTLOOP_COUNT] = 0;
}

int db_settings::get_idx(string_view key) const {
    int idx = 0;
    for (const char *k : DB_SETTING_KEYS) {
        if (key == k)
            break;
        ++idx;
    }
    return idx;
}

struct db_result {
    db_result() = default;
    db_result(const char *s) : err(s) {}
    db_result(int code) : err(code == SQLITE_OK ? "" : (sqlite3_errstr(code) ?: "")) {}
    operator bool() {
        if (!err.empty()) {
            LOGE("sqlite3: %s\n", err.data());
            return false;
        }
        return true;
    }
private:
    string err;
};

static int sql_exec(sqlite3 *db, const char *sql, sqlite_row_callback callback = nullptr, void *v = nullptr) {
    return sql_exec(db, sql, {}, callback, v);
}

static db_result open_and_init_db() {
    if (!load_sqlite())
        return "Cannot load libsqlite.so";

    unique_ptr<sqlite3, decltype(sqlite3_close)> db(nullptr, sqlite3_close);
    {
        sqlite3 *sql;
        fn_run_ret(sqlite3_open_v2, MAGISKDB, &sql,
                   SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
        db.reset(sql);
    }

    int ver = 0;
    bool upgrade = false;
    auto ver_cb = [](void *ver, auto, StringSlice data) {
        *((int *) ver) = parse_int(data[0].c_str());
    };
    fn_run_ret(sql_exec, db.get(), "PRAGMA user_version", ver_cb, &ver);
    if (ver > DB_VERSION) {
        // Don't support downgrading database
        return "Downgrading database is not supported";
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
        fn_run_ret(create_policy);
        fn_run_ret(create_settings);
        fn_run_ret(create_strings);
        fn_run_ret(create_denylist);

        // Directly jump to latest
        ver = DB_VERSION;
        upgrade = true;
    }
    if (ver == 7) {
        fn_run_ret(sql_exec, db.get(),
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
        fn_run_ret(sql_exec, db.get(),
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
        fn_run_ret(sql_exec, db.get(), "DROP TABLE IF EXISTS logs", nullptr, nullptr);
        ver = 10;
        upgrade = true;
    }
    if (ver == 10) {
        fn_run_ret(sql_exec, db.get(),
                "DROP TABLE IF EXISTS hidelist;"
                "DELETE FROM settings WHERE key='magiskhide';");
        fn_run_ret(create_denylist);
        ver = 11;
        upgrade = true;
    }
    if (ver == 11) {
        fn_run_ret(sql_exec, db.get(),
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
        char query[32];
        sprintf(query, "PRAGMA user_version=%d", ver);
        fn_run_ret(sql_exec, db.get(), query);
    }
    mDB = db.release();
    return {};
}

static db_result ensure_db() {
    if (mDB == nullptr) {
        db_result res = open_and_init_db();
        if (!res) {
            // Open fails, remove and reconstruct
            unlink(MAGISKDB);
            return open_and_init_db();
        }
    }
    return {};
}

bool db_exec(const char *sql) {
    if (ensure_db() && mDB) {
        db_result res = sql_exec(mDB, sql);
        return res;
    }
    return false;
}

bool db_exec(const char *sql, const db_row_cb &fn) {
    if (ensure_db() && mDB) {
        auto convert = [](void *cb, StringSlice columns, StringSlice data) {
            auto &func = *static_cast<const db_row_cb*>(cb);
            db_row row;
            for (int i = 0; i < columns.size(); ++i)
                row[columns[i].c_str()] = data[i].c_str();
            func(row);
        };
        db_result res = sql_exec(mDB, sql, convert, (void *) &fn);
        return res;
    }
    return false;
}

int get_db_settings(db_settings &cfg, int key) {
    bool res;
    auto settings_cb = [&](db_row &row) -> bool {
        cfg[row["key"]] = parse_int(row["value"]);
        DBLOGV("query %s=[%s]\n", row["key"].data(), row["value"].data());
        return true;
    };
    if (key >= 0) {
        char query[128];
        ssprintf(query, sizeof(query), "SELECT * FROM settings WHERE key='%s'", DB_SETTING_KEYS[key]);
        res = db_exec(query, settings_cb);
    } else {
        res = db_exec("SELECT * FROM settings", settings_cb);
    }
    return res ? 0 : 1;
}

int set_db_settings(int key, int value) {
    char sql[128];
    ssprintf(sql, sizeof(sql), "INSERT OR REPLACE INTO settings VALUES ('%s', %d)",
             DB_SETTING_KEYS[key], value);
    return db_exec(sql) ? 0 : 1;
}

int get_db_strings(db_strings &str, int key) {
    bool res;
    auto string_cb = [&](db_row &row) -> bool {
        str[row["key"]] = row["value"];
        DBLOGV("query %s=[%s]\n", row["key"].data(), row["value"].data());
        return true;
    };
    if (key >= 0) {
        char query[128];
        ssprintf(query, sizeof(query), "SELECT * FROM strings WHERE key='%s'", DB_STRING_KEYS[key]);
        res = db_exec(query, string_cb);
    } else {
        res = db_exec("SELECT * FROM strings", string_cb);
    }
    return res ? 0 : 1;
}

void rm_db_strings(int key) {
    char query[128];
    ssprintf(query, sizeof(query), "DELETE FROM strings WHERE key == '%s'", DB_STRING_KEYS[key]);
    db_exec(query);
}

void exec_sql(owned_fd client) {
    string sql = read_string(client);
    db_exec(sql.data(), [fd = (int) client](db_row &row) -> bool {
        string out;
        bool first = true;
        for (auto it : row) {
            if (first) first = false;
            else out += '|';
            out += it.first;
            out += '=';
            out += it.second;
        }
        write_string(fd, out);
        return true;
    });
    write_int(client, 0);
}
