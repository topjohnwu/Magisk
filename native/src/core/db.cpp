#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include <consts.hpp>
#include <base.hpp>
#include <db.hpp>
#include <core.hpp>

#define DB_VERSION 12

using namespace std;

struct sqlite3;
struct sqlite3_stmt;

static sqlite3 *mDB = nullptr;

#define DBLOGV(...)
//#define DBLOGV(...) LOGD("magiskdb: " __VA_ARGS__)

// SQLite APIs

#define SQLITE_OPEN_READWRITE        0x00000002  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_CREATE           0x00000004  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_FULLMUTEX        0x00010000  /* Ok for sqlite3_open_v2() */

#define SQLITE_OK           0   /* Successful result */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */

static int (*sqlite3_open_v2)(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
static int (*sqlite3_close)(sqlite3 *db);
static int (*sqlite3_prepare_v2)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
static int (*sqlite3_bind_parameter_count)(sqlite3_stmt*);
static int (*sqlite3_bind_int)(sqlite3_stmt*, int, int);
static int (*sqlite3_bind_text)(sqlite3_stmt*,int,const char*,int,void(*)(void*));
static int (*sqlite3_column_count)(sqlite3_stmt *pStmt);
static const char *(*sqlite3_column_name)(sqlite3_stmt*, int N);
static const char *(*sqlite3_column_text)(sqlite3_stmt*, int iCol);
static int (*sqlite3_step)(sqlite3_stmt*);
static int (*sqlite3_finalize)(sqlite3_stmt *pStmt);
static const char *(*sqlite3_errstr)(int);

// Internal Android linker APIs

static void (*android_get_LD_LIBRARY_PATH)(char *buffer, size_t buffer_size);
static void (*android_update_LD_LIBRARY_PATH)(const char *ld_library_path);

#define DLERR(ptr) if (!(ptr)) { \
    LOGE("db: %s\n", dlerror()); \
    return false; \
}

#define DLOAD(handle, arg) {\
    auto f = dlsym(handle, #arg); \
    DLERR(f) \
    *(void **) &(arg) = f; \
}

#ifdef __LP64__
constexpr char apex_path[] = "/apex/com.android.runtime/lib64:/apex/com.android.art/lib64:/apex/com.android.i18n/lib64:";
#else
constexpr char apex_path[] = "/apex/com.android.runtime/lib:/apex/com.android.art/lib:/apex/com.android.i18n/lib:";
#endif

static bool dload_sqlite() {
    static int dl_init = 0;
    if (dl_init)
        return dl_init > 0;
    dl_init = -1;

    auto sqlite = dlopen("libsqlite.so", RTLD_LAZY);
    if (!sqlite) {
        // Should only happen on Android 10+
        auto dl = dlopen("libdl_android.so", RTLD_LAZY);
        DLERR(dl);

        DLOAD(dl, android_get_LD_LIBRARY_PATH);
        DLOAD(dl, android_update_LD_LIBRARY_PATH);

        // Inject APEX into LD_LIBRARY_PATH
        char ld_path[4096];
        memcpy(ld_path, apex_path, sizeof(apex_path));
        constexpr int len = sizeof(apex_path) - 1;
        android_get_LD_LIBRARY_PATH(ld_path + len, sizeof(ld_path) - len);
        android_update_LD_LIBRARY_PATH(ld_path);
        sqlite = dlopen("libsqlite.so", RTLD_LAZY);

        // Revert LD_LIBRARY_PATH just in case
        android_update_LD_LIBRARY_PATH(ld_path + len);
    }
    DLERR(sqlite);

    DLOAD(sqlite, sqlite3_open_v2);
    DLOAD(sqlite, sqlite3_close);
    DLOAD(sqlite, sqlite3_prepare_v2);
    DLOAD(sqlite, sqlite3_bind_parameter_count);
    DLOAD(sqlite, sqlite3_bind_int);
    DLOAD(sqlite, sqlite3_bind_text);
    DLOAD(sqlite, sqlite3_step);
    DLOAD(sqlite, sqlite3_column_count);
    DLOAD(sqlite, sqlite3_column_name);
    DLOAD(sqlite, sqlite3_column_text);
    DLOAD(sqlite, sqlite3_finalize);
    DLOAD(sqlite, sqlite3_errstr);

    dl_init = 1;
    return true;
}

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

static void ver_cb(void *ver, auto, rust::Slice<rust::String> data) {
    *((int *) ver) = parse_int(data[0].c_str());
}

db_result::db_result(int code) : err(code == SQLITE_OK ? "" : (sqlite3_errstr(code) ?: "")) {}

bool db_result::check_err() {
    if (!err.empty()) {
        LOGE("sqlite3: %s\n", err.data());
        return true;
    }
    return false;
}

using StringVec = rust::Vec<rust::String>;
using StringSlice = rust::Slice<rust::String>;
using StrSlice = rust::Slice<rust::Str>;
using sqlite_row_callback = void(*)(void*, StringSlice, StringSlice);

#define fn_run_ret(fn, ...) if (int rc = fn(__VA_ARGS__); rc != SQLITE_OK) return rc

static int sql_exec(sqlite3 *db, rust::Str zSql, StrSlice args, sqlite_row_callback callback, void *v) {
    const char *sql = zSql.begin();
    auto arg_it = args.begin();
    unique_ptr<sqlite3_stmt, decltype(sqlite3_finalize)> stmt(nullptr, sqlite3_finalize);

    while (sql != zSql.end()) {
        // Step 1: prepare statement
        {
            sqlite3_stmt *st = nullptr;
            fn_run_ret(sqlite3_prepare_v2, db, sql, zSql.end() - sql, &st, &sql);
            if (st == nullptr) continue;
            stmt.reset(st);
        }

        // Step 2: bind arguments
        if (int count = sqlite3_bind_parameter_count(stmt.get())) {
            for (int i = 1; i <= count && arg_it != args.end(); ++i, ++arg_it) {
                fn_run_ret(sqlite3_bind_text, stmt.get(), i, arg_it->data(), arg_it->size(), nullptr);
            }
        }

        // Step 3: execute
        bool first = true;
        StringVec columns;
        for (;;) {
            int rc = sqlite3_step(stmt.get());
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) return rc;
            if (callback == nullptr) continue;
            if (first) {
                int count = sqlite3_column_count(stmt.get());
                for (int i = 0; i < count; ++i) {
                    columns.emplace_back(sqlite3_column_name(stmt.get(), i));
                }
                first = false;
            }
            StringVec data;
            for (int i = 0; i < columns.size(); ++i) {
                data.emplace_back(sqlite3_column_text(stmt.get(), i));
            }
            callback(v, StringSlice(columns), StringSlice(data));
        }
    }

    return SQLITE_OK;
}

static int sql_exec(sqlite3 *db, const char *sql, sqlite_row_callback callback = nullptr, void *v = nullptr) {
    return sql_exec(db, sql, {}, callback, v);
}

static db_result open_and_init_db() {
    if (!dload_sqlite())
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
        auto res = open_and_init_db();
        if (res.check_err()) {
            // Open fails, remove and reconstruct
            unlink(MAGISKDB);
            res = open_and_init_db();
            if (!res) return res;
        }
    }
    return {};
}

db_result db_exec(const char *sql) {
    if (auto res = ensure_db(); !res) return res;
    if (mDB) {
        return sql_exec(mDB, sql);
    }
    return {};
}

static void row_to_db_row(void *cb, rust::Slice<rust::String> columns, rust::Slice<rust::String> data) {
    auto &func = *static_cast<const db_row_cb*>(cb);
    db_row row;
    for (int i = 0; i < columns.size(); ++i)
        row[columns[i].c_str()] = data[i].c_str();
    func(row);
}

db_result db_exec(const char *sql, const db_row_cb &fn) {
    if (auto res = ensure_db(); !res) return res;
    if (mDB) {
        return sql_exec(mDB, sql, row_to_db_row, (void *) &fn);
    }
    return {};
}

int get_db_settings(db_settings &cfg, int key) {
    db_result res;
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
    return res.check_err() ? 1 : 0;
}

int set_db_settings(int key, int value) {
    char sql[128];
    ssprintf(sql, sizeof(sql), "INSERT OR REPLACE INTO settings VALUES ('%s', %d)",
             DB_SETTING_KEYS[key], value);
    return db_exec(sql).check_err() ? 1 : 0;
}

int get_db_strings(db_strings &str, int key) {
    db_result res;
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
    return res.check_err() ? 1 : 0;
}

void rm_db_strings(int key) {
    char query[128];
    ssprintf(query, sizeof(query), "DELETE FROM strings WHERE key == '%s'", DB_STRING_KEYS[key]);
    db_exec(query).check_err();
}

void exec_sql(owned_fd client) {
    string sql = read_string(client);
    auto res = db_exec(sql.data(), [fd = (int) client](db_row &row) -> bool {
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
    res.check_err();
}
