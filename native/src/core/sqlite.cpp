#include <dlfcn.h>

#include <consts.hpp>
#include <base.hpp>
#include <sqlite.hpp>

using namespace std;

#define DB_VERSION     12
#define DB_VERSION_STR "12"

// SQLite APIs

static int (*sqlite3_open_v2)(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
static int (*sqlite3_close)(sqlite3 *db);
const char *(*sqlite3_errstr)(int);
static int (*sqlite3_prepare_v2)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
static int (*sqlite3_bind_parameter_count)(sqlite3_stmt*);
static int (*sqlite3_bind_int64)(sqlite3_stmt*, int, int64_t);
static int (*sqlite3_bind_text)(sqlite3_stmt*,int,const char*,int,void(*)(void*));
static int (*sqlite3_column_count)(sqlite3_stmt *pStmt);
static const char *(*sqlite3_column_name)(sqlite3_stmt*, int N);
static const char *(*sqlite3_column_text)(sqlite3_stmt*, int iCol);
static int (*sqlite3_column_int)(sqlite3_stmt*, int iCol);
static int (*sqlite3_step)(sqlite3_stmt*);
static int (*sqlite3_finalize)(sqlite3_stmt *pStmt);

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

static bool load_sqlite() {
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
    DLOAD(sqlite, sqlite3_errstr);
    DLOAD(sqlite, sqlite3_prepare_v2);
    DLOAD(sqlite, sqlite3_bind_parameter_count);
    DLOAD(sqlite, sqlite3_bind_int64);
    DLOAD(sqlite, sqlite3_bind_text);
    DLOAD(sqlite, sqlite3_step);
    DLOAD(sqlite, sqlite3_column_count);
    DLOAD(sqlite, sqlite3_column_name);
    DLOAD(sqlite, sqlite3_column_text);
    DLOAD(sqlite, sqlite3_column_int);
    DLOAD(sqlite, sqlite3_finalize);

    dl_init = 1;
    return true;
}

using StringVec = rust::Vec<rust::String>;
using sql_bind_callback_real = int(*)(void*, int, sqlite3_stmt*);
using sql_exec_callback_real = void(*)(void*, StringSlice, sqlite3_stmt*);

#define sql_chk(fn, ...) if (int rc = fn(__VA_ARGS__); rc != SQLITE_OK) return rc

// Exports to Rust
extern "C" int sql_exec_impl(
        sqlite3 *db, rust::Str zSql,
        sql_bind_callback bind_cb = nullptr, void *bind_cookie = nullptr,
        sql_exec_callback exec_cb = nullptr, void *exec_cookie = nullptr) {
    const char *sql = zSql.begin();
    unique_ptr<sqlite3_stmt, decltype(sqlite3_finalize)> stmt(nullptr, sqlite3_finalize);

    while (sql != zSql.end()) {
        // Step 1: prepare statement
        {
            sqlite3_stmt *st = nullptr;
            sql_chk(sqlite3_prepare_v2, db, sql, zSql.end() - sql, &st, &sql);
            if (st == nullptr) continue;
            stmt.reset(st);
        }

        // Step 2: bind arguments
        if (bind_cb) {
            if (int count = sqlite3_bind_parameter_count(stmt.get())) {
                auto real_cb = reinterpret_cast<sql_bind_callback_real>(bind_cb);
                for (int i = 1; i <= count; ++i) {
                    sql_chk(real_cb, bind_cookie, i, stmt.get());
                }
            }
        }

        // Step 3: execute
        bool first = true;
        StringVec columns;
        for (;;) {
            int rc = sqlite3_step(stmt.get());
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) return rc;
            if (exec_cb == nullptr) continue;
            if (first) {
                int count = sqlite3_column_count(stmt.get());
                for (int i = 0; i < count; ++i) {
                    columns.emplace_back(sqlite3_column_name(stmt.get(), i));
                }
                first = false;
            }
            auto real_cb = reinterpret_cast<sql_exec_callback_real>(exec_cb);
            real_cb(exec_cookie, StringSlice(columns), stmt.get());
        }
    }

    return SQLITE_OK;
}

int DbValues::get_int(int index) const {
    return sqlite3_column_int((sqlite3_stmt*) this, index);
}

const char *DbValues::get_text(int index) const {
    return sqlite3_column_text((sqlite3_stmt*) this, index);
}

int DbStatement::bind_int64(int index, int64_t val) {
    return sqlite3_bind_int64(reinterpret_cast<sqlite3_stmt*>(this), index, val);
}

int DbStatement::bind_text(int index, rust::Str val) {
    return sqlite3_bind_text(reinterpret_cast<sqlite3_stmt*>(this), index, val.data(), val.size(), nullptr);
}

#define sql_chk_log_ret(ret, fn, ...) if (int rc = fn(__VA_ARGS__); rc != SQLITE_OK) { \
    LOGE("sqlite3(line:%d): %s\n", __LINE__, sqlite3_errstr(rc));                      \
    return ret;                                                                        \
}

#define sql_chk_log(fn, ...) sql_chk_log_ret(nullptr, fn, __VA_ARGS__)

sqlite3 *open_and_init_db() {
    if (!load_sqlite()) {
        LOGE("sqlite3: Cannot load libsqlite.so\n");
        return nullptr;
    }

    unique_ptr<sqlite3, decltype(sqlite3_close)> db(nullptr, sqlite3_close);
    {
        sqlite3 *sql;
        // We open the connection with SQLITE_OPEN_NOMUTEX because we are guarding it ourselves
        sql_chk_log(sqlite3_open_v2, MAGISKDB, &sql,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, nullptr);
        db.reset(sql);
    }

    int ver = 0;
    bool upgrade = false;
    auto ver_cb = [](void *ver, auto, const DbValues &values) {
        *static_cast<int *>(ver) = values.get_int(0);
    };
    sql_chk_log(sql_exec_impl, db.get(), "PRAGMA user_version", nullptr, nullptr, ver_cb, &ver);
    if (ver > DB_VERSION) {
        // Don't support downgrading database, delete and retry
        LOGE("sqlite3: Downgrading database is not supported\n");
        unlink(MAGISKDB);
        return open_and_init_db();
    }

    auto create_policy = [&] {
        return sql_exec_impl(db.get(),
                "CREATE TABLE IF NOT EXISTS policies "
                "(uid INT, policy INT, until INT, logging INT, "
                "notification INT, PRIMARY KEY(uid))");
    };
    auto create_settings = [&] {
        return sql_exec_impl(db.get(),
                "CREATE TABLE IF NOT EXISTS settings "
                "(key TEXT, value INT, PRIMARY KEY(key))");
    };
    auto create_strings = [&] {
        return sql_exec_impl(db.get(),
                "CREATE TABLE IF NOT EXISTS strings "
                "(key TEXT, value TEXT, PRIMARY KEY(key))");
    };
    auto create_denylist = [&] {
        return sql_exec_impl(db.get(),
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
        sql_chk_log(sql_exec_impl, db.get(),
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
        sql_chk_log(sql_exec_impl, db.get(),
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
        sql_chk_log(sql_exec_impl, db.get(), "DROP TABLE IF EXISTS logs", nullptr, nullptr);
        ver = 10;
        upgrade = true;
    }
    if (ver == 10) {
        sql_chk_log(sql_exec_impl, db.get(),
                "DROP TABLE IF EXISTS hidelist;"
                "DELETE FROM settings WHERE key='magiskhide';");
        sql_chk_log(create_denylist);
        ver = 11;
        upgrade = true;
    }
    if (ver == 11) {
        sql_chk_log(sql_exec_impl, db.get(),
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
        sql_chk_log(sql_exec_impl, db.get(), "PRAGMA user_version=" DB_VERSION_STR);
    }

    return db.release();
}

// Exported from Rust
extern "C" int sql_exec_rs(
        rust::Str zSql,
        sql_bind_callback bind_cb, void *bind_cookie,
        sql_exec_callback exec_cb, void *exec_cookie);

bool db_exec(const char *sql, DbArgs args, db_exec_callback exec_fn) {
    using db_bind_callback = std::function<int(int, DbStatement&)>;

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
        exec_cb = [](void *v, StringSlice columns, const DbValues &values) {
            auto fn = static_cast<db_exec_callback*>(v);
            fn->operator()(columns, values);
        };
    }
    sql_chk_log_ret(false, sql_exec_rs, sql, bind_cb, &bind_fn, exec_cb, &exec_fn);
    return true;
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
