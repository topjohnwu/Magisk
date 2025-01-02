#include <dlfcn.h>

#include <base.hpp>
#include <sqlite.hpp>

using namespace std;

// SQLite APIs

int (*sqlite3_open_v2)(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
int (*sqlite3_close)(sqlite3 *db);
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

bool load_sqlite() {
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

int sql_exec(sqlite3 *db, rust::Str zSql, sql_bind_callback bind_cb, void *bind_cookie, sql_exec_callback exec_cb, void *exec_cookie) {
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

int DbValues::get_int(int index) {
    return sqlite3_column_int(reinterpret_cast<sqlite3_stmt*>(this), index);
}

const char *DbValues::get_text(int index) {
    return sqlite3_column_text(reinterpret_cast<sqlite3_stmt*>(this), index);
}

int DbStatement::bind_int64(int index, int64_t val) {
    return sqlite3_bind_int64(reinterpret_cast<sqlite3_stmt*>(this), index, val);
}

int DbStatement::bind_text(int index, rust::Str val) {
    return sqlite3_bind_text(reinterpret_cast<sqlite3_stmt*>(this), index, val.data(), val.size(), nullptr);
}
