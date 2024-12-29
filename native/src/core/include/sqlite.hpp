#pragma once

#include <cxx.h>

#define SQLITE_OPEN_READWRITE        0x00000002  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_CREATE           0x00000004  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_FULLMUTEX        0x00010000  /* Ok for sqlite3_open_v2() */

#define SQLITE_OK           0   /* Successful result */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */

struct sqlite3;
struct sqlite3_stmt;

extern int (*sqlite3_open_v2)(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
extern int (*sqlite3_close)(sqlite3 *db);
extern int (*sqlite3_prepare_v2)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
extern int (*sqlite3_bind_parameter_count)(sqlite3_stmt*);
extern int (*sqlite3_bind_int)(sqlite3_stmt*, int, int);
extern int (*sqlite3_bind_text)(sqlite3_stmt*,int,const char*,int,void(*)(void*));
extern int (*sqlite3_column_count)(sqlite3_stmt *pStmt);
extern const char *(*sqlite3_column_name)(sqlite3_stmt*, int N);
extern const char *(*sqlite3_column_text)(sqlite3_stmt*, int iCol);
extern int (*sqlite3_step)(sqlite3_stmt*);
extern int (*sqlite3_finalize)(sqlite3_stmt *pStmt);
extern const char *(*sqlite3_errstr)(int);

using StringVec = rust::Vec<rust::String>;
using StringSlice = rust::Slice<rust::String>;
using StrSlice = rust::Slice<rust::Str>;
using sqlite_row_callback = void(*)(void*, StringSlice, StringSlice);

#define fn_run_ret(fn, ...) if (int rc = fn(__VA_ARGS__); rc != SQLITE_OK) return rc

bool load_sqlite();
int sql_exec(sqlite3 *db, rust::Str zSql, StrSlice args, sqlite_row_callback callback, void *v);
