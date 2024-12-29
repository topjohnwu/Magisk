#include <dlfcn.h>

#include <base.hpp>
#include <sqlite.hpp>

using namespace std;

// SQLite APIs

int (*sqlite3_open_v2)(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
int (*sqlite3_close)(sqlite3 *db);
int (*sqlite3_prepare_v2)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
int (*sqlite3_bind_parameter_count)(sqlite3_stmt*);
int (*sqlite3_bind_int)(sqlite3_stmt*, int, int);
int (*sqlite3_bind_text)(sqlite3_stmt*,int,const char*,int,void(*)(void*));
int (*sqlite3_column_count)(sqlite3_stmt *pStmt);
const char *(*sqlite3_column_name)(sqlite3_stmt*, int N);
const char *(*sqlite3_column_text)(sqlite3_stmt*, int iCol);
int (*sqlite3_step)(sqlite3_stmt*);
int (*sqlite3_finalize)(sqlite3_stmt *pStmt);
const char *(*sqlite3_errstr)(int);

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

int sql_exec(sqlite3 *db, rust::Str zSql, StrSlice args, sqlite_row_callback callback, void *v) {
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
