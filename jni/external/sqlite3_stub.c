#include <stdlib.h>
#include "sqlite3.h"
SQLITE_API const char *sqlite3_libversion(void) { return 0; }
SQLITE_API const char *sqlite3_sourceid(void) { return 0; }
SQLITE_API int sqlite3_libversion_number(void) { return 0; }
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
SQLITE_API int sqlite3_compileoption_used(const char *zOptName) { return 0; }
SQLITE_API const char *sqlite3_compileoption_get(int N) { return 0; }
#endif
SQLITE_API int sqlite3_threadsafe(void) { return 0; }
SQLITE_API int sqlite3_close(sqlite3 *db) { return 0; }
SQLITE_API int sqlite3_close_v2(sqlite3 *db) { return 0; }
SQLITE_API int sqlite3_exec(
  sqlite3 *db,
  const char *sql,
  int (*callback)(void*,int,char**,char**),
  void *v,
  char **errmsg
) { return 0; }
SQLITE_API int sqlite3_initialize(void) { return 0; }
SQLITE_API int sqlite3_shutdown(void) { return 0; }
SQLITE_API int sqlite3_os_init(void) { return 0; }
SQLITE_API int sqlite3_os_end(void) { return 0; }
SQLITE_API int sqlite3_config(int i, ...) { return 0; }
SQLITE_API int sqlite3_db_config(sqlite3 *db, int op, ...) { return 0; }
SQLITE_API int sqlite3_extended_result_codes(sqlite3 *db, int onoff) { return 0; }
SQLITE_API sqlite3_int64 sqlite3_last_insert_rowid(sqlite3 *db) { return 0; }
SQLITE_API void sqlite3_set_last_insert_rowid(sqlite3 *db,sqlite3_int64 i) { }
SQLITE_API int sqlite3_changes(sqlite3 *db) { return 0; }
SQLITE_API int sqlite3_total_changes(sqlite3 *db) { return 0; }
SQLITE_API void sqlite3_interrupt(sqlite3 *db) { }
SQLITE_API int sqlite3_complete(const char *sql) { return 0; }
SQLITE_API int sqlite3_complete16(const void *sql) { return 0; }
SQLITE_API int sqlite3_busy_handler(sqlite3 *db, int(*f) (void *v,int i), void* v) { return 0; }
SQLITE_API int sqlite3_busy_timeout(sqlite3 *db, int ms) { return 0; }
SQLITE_API int sqlite3_get_table(
  sqlite3 *db,
  const char *zSql,
  char ***pazResult,
  int *pnRow,
  int *pnColumn,
  char **pzErrmsg
) { return 0; }
SQLITE_API void sqlite3_free_table(char **result) { }
SQLITE_API char *sqlite3_mprintf(const char* s,...) { return 0; }
SQLITE_API char *sqlite3_vmprintf(const char* s, va_list v) { return 0; }
SQLITE_API char *sqlite3_snprintf(int i, char* s, const char* st, ...) { return 0; }
SQLITE_API char *sqlite3_vsnprintf(int i, char* s, const char* st, va_list v) { return 0; }
SQLITE_API void *sqlite3_malloc(int i) { return 0; }
SQLITE_API void *sqlite3_malloc64(sqlite3_uint64 i) { return 0; }
SQLITE_API void *sqlite3_realloc(void* v, int i) { return 0; }
SQLITE_API void *sqlite3_realloc64(void* v, sqlite3_uint64 i) { return 0; }
SQLITE_API void sqlite3_free(void* v) { }
SQLITE_API sqlite3_uint64 sqlite3_msize(void* v) { return 0; }
SQLITE_API sqlite3_int64 sqlite3_memory_used(void) { return 0; }
SQLITE_API sqlite3_int64 sqlite3_memory_highwater(int resetFlag) { return 0; }
SQLITE_API void sqlite3_randomness(int N, void *P) { }
SQLITE_API int sqlite3_set_authorizer(
  sqlite3 *db,
  int (*xAuth)(void *v,int i,const char*,const char*,const char*,const char*),
  void *pUserData
) { return 0; }
SQLITE_API SQLITE_DEPRECATED void *sqlite3_trace(sqlite3 *db,
   void(*xTrace)(void *v,const char*), void* v) { return 0; }
SQLITE_API SQLITE_DEPRECATED void *sqlite3_profile(sqlite3 *db,
   void(*xProfile)(void *v,const char*,sqlite3_uint64), void* v) { return 0; }
SQLITE_API int sqlite3_trace_v2(
  sqlite3 *db,
  unsigned uMask,
  int(*xCallback)(unsigned,void *v,void *vv,void*),
  void *pCtx
) { return 0; }
SQLITE_API void sqlite3_progress_handler(sqlite3 *db, int i, int(*f)(void*), void* v) { }
SQLITE_API int sqlite3_open(
  const char *filename,
  sqlite3 **ppDb
) { return 0; }
SQLITE_API int sqlite3_open16(
  const void *filename,
  sqlite3 **ppDb
) { return 0; }
SQLITE_API int sqlite3_open_v2(
  const char *filename,
  sqlite3 **ppDb,
  int flags,
  const char *zVfs
) { return 0; }
SQLITE_API const char *sqlite3_uri_parameter(const char *zFilename, const char *zParam) { return 0; }
SQLITE_API int sqlite3_uri_boolean(const char *zFile, const char *zParam, int bDefault) { return 0; }
SQLITE_API sqlite3_int64 sqlite3_uri_int64(const char* s, const char* st, sqlite3_int64 i) { return 0; }
SQLITE_API int sqlite3_errcode(sqlite3 *db) { return 0; }
SQLITE_API int sqlite3_extended_errcode(sqlite3 *db) { return 0; }
SQLITE_API const char *sqlite3_errmsg(sqlite3 *db) { return 0; }
SQLITE_API const void *sqlite3_errmsg16(sqlite3 *db) { return 0; }
SQLITE_API const char *sqlite3_errstr(int i) { return 0; }
SQLITE_API int sqlite3_limit(sqlite3 *db, int id, int newVal) { return 0; }
SQLITE_API int sqlite3_prepare(
  sqlite3 *db,
  const char *zSql,
  int nByte,
  sqlite3_stmt **ppStmt,
  const char **pzTail
) { return 0; }
SQLITE_API int sqlite3_prepare_v2(
  sqlite3 *db,
  const char *zSql,
  int nByte,
  sqlite3_stmt **ppStmt,
  const char **pzTail
) { return 0; }
SQLITE_API int sqlite3_prepare16(
  sqlite3 *db,
  const void *zSql,
  int nByte,
  sqlite3_stmt **ppStmt,
  const void **pzTail
) { return 0; }
SQLITE_API int sqlite3_prepare16_v2(
  sqlite3 *db,
  const void *zSql,
  int nByte,
  sqlite3_stmt **ppStmt,
  const void **pzTail
) { return 0; }
SQLITE_API const char *sqlite3_sql(sqlite3_stmt *pStmt) { return 0; }
SQLITE_API char *sqlite3_expanded_sql(sqlite3_stmt *pStmt) { return 0; }
SQLITE_API int sqlite3_stmt_readonly(sqlite3_stmt *pStmt) { return 0; }
SQLITE_API int sqlite3_stmt_busy(sqlite3_stmt* s) { return 0; }
SQLITE_API int sqlite3_bind_blob(sqlite3_stmt* s, int i, const void*v, int n, void(*f)(void*)) { return 0; }
SQLITE_API int sqlite3_bind_blob64(sqlite3_stmt* s, int i, const void *v, sqlite3_uint64 ii,
                        void(*f)(void*)) { return 0; }
SQLITE_API int sqlite3_bind_double(sqlite3_stmt* s, int i, double d) { return 0; }
SQLITE_API int sqlite3_bind_int(sqlite3_stmt* s, int i, int ii) { return 0; }
SQLITE_API int sqlite3_bind_int64(sqlite3_stmt* s, int i, sqlite3_int64 ii) { return 0; }
SQLITE_API int sqlite3_bind_null(sqlite3_stmt* s, int i) { return 0; }
SQLITE_API int sqlite3_bind_text(sqlite3_stmt* s,int i,const char* str, int ii, void(*f)(void*)) { return 0; }
SQLITE_API int sqlite3_bind_text16(sqlite3_stmt* s, int i, const void *v, int ii, void(*f)(void*)) { return 0; }
SQLITE_API int sqlite3_bind_text64(sqlite3_stmt* s, int i, const char* str, sqlite3_uint64 ii,
                         void(*f)(void*), unsigned char encoding) { return 0; }
SQLITE_API int sqlite3_bind_value(sqlite3_stmt* s, int i, const sqlite3_value* v) { return 0; }
SQLITE_API int sqlite3_bind_zeroblob(sqlite3_stmt* s, int i, int n) { return 0; }
SQLITE_API int sqlite3_bind_zeroblob64(sqlite3_stmt* s, int i, sqlite3_uint64 ii) { return 0; }
SQLITE_API int sqlite3_bind_parameter_count(sqlite3_stmt* s) { return 0; }
SQLITE_API const char *sqlite3_bind_parameter_name(sqlite3_stmt* s, int i) { return 0; }
SQLITE_API int sqlite3_bind_parameter_index(sqlite3_stmt* s, const char *zName) { return 0; }
SQLITE_API int sqlite3_clear_bindings(sqlite3_stmt* s) { return 0; }
SQLITE_API int sqlite3_column_count(sqlite3_stmt *pStmt) { return 0; }
SQLITE_API const char *sqlite3_column_name(sqlite3_stmt* s, int N) { return 0; }
SQLITE_API const void *sqlite3_column_name16(sqlite3_stmt* s, int N) { return 0; }
SQLITE_API const char *sqlite3_column_database_name(sqlite3_stmt* s,int i) { return 0; }
SQLITE_API const void *sqlite3_column_database_name16(sqlite3_stmt* s,int i){ return 0; }
SQLITE_API const char *sqlite3_column_table_name(sqlite3_stmt* s,int i) { return 0; }
SQLITE_API const void *sqlite3_column_table_name16(sqlite3_stmt* s,int i) { return 0; }
SQLITE_API const char *sqlite3_column_origin_name(sqlite3_stmt* s,int i) { return 0; }
SQLITE_API const void *sqlite3_column_origin_name16(sqlite3_stmt* s,int i) { return 0; }
SQLITE_API const char *sqlite3_column_decltype(sqlite3_stmt* s,int i) { return 0; }
SQLITE_API const void *sqlite3_column_decltype16(sqlite3_stmt* s,int i) { return 0; }
SQLITE_API int sqlite3_step(sqlite3_stmt* s) { return 0; }
SQLITE_API int sqlite3_data_count(sqlite3_stmt *pStmt) { return 0; }
SQLITE_API const void *sqlite3_column_blob(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API int sqlite3_column_bytes(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API int sqlite3_column_bytes16(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API double sqlite3_column_double(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API int sqlite3_column_int(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API sqlite3_int64 sqlite3_column_int64(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API const unsigned char *sqlite3_column_text(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API const void *sqlite3_column_text16(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API int sqlite3_column_type(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API sqlite3_value *sqlite3_column_value(sqlite3_stmt* s, int iCol) { return 0; }
SQLITE_API int sqlite3_finalize(sqlite3_stmt *pStmt) { return 0; }
SQLITE_API int sqlite3_reset(sqlite3_stmt *pStmt) { return 0; }
SQLITE_API int sqlite3_create_function(
  sqlite3 *db,
  const char *zFunctionName,
  int nArg,
  int eTextRep,
  void *pApp,
  void (*xFunc)(sqlite3_context* c,int i,sqlite3_value**),
  void (*xStep)(sqlite3_context* c,int i,sqlite3_value**),
  void (*xFinal)(sqlite3_context* c)
) { return 0; }
SQLITE_API int sqlite3_create_function16(
  sqlite3 *db,
  const void *zFunctionName,
  int nArg,
  int eTextRep,
  void *pApp,
  void (*xFunc)(sqlite3_context* c,int i,sqlite3_value**),
  void (*xStep)(sqlite3_context* c,int i,sqlite3_value**),
  void (*xFinal)(sqlite3_context* c)
) { return 0; }
SQLITE_API int sqlite3_create_function_v2(
  sqlite3 *db,
  const char *zFunctionName,
  int nArg,
  int eTextRep,
  void *pApp,
  void (*xFunc)(sqlite3_context* c,int i,sqlite3_value**),
  void (*xStep)(sqlite3_context* c,int i,sqlite3_value**),
  void (*xFinal)(sqlite3_context* c),
  void(*xDestroy)(void*)
) { return 0; }
#ifndef SQLITE_OMIT_DEPRECATED
SQLITE_API SQLITE_DEPRECATED int sqlite3_aggregate_count(sqlite3_context* c) { return 0; }
SQLITE_API SQLITE_DEPRECATED int sqlite3_expired(sqlite3_stmt* s) { return 0; }
SQLITE_API SQLITE_DEPRECATED int sqlite3_transfer_bindings(sqlite3_stmt* s, sqlite3_stmt* ss) { return 0; }
SQLITE_API SQLITE_DEPRECATED int sqlite3_global_recover(void) { return 0; }
SQLITE_API SQLITE_DEPRECATED void sqlite3_thread_cleanup(void) { }
SQLITE_API SQLITE_DEPRECATED int sqlite3_memory_alarm(void(*f)(void *v,sqlite3_int64,int),
                      void *v,sqlite3_int64 i) { return 0; }
#endif
SQLITE_API const void *sqlite3_value_blob(sqlite3_value* v) { return 0; }
SQLITE_API int sqlite3_value_bytes(sqlite3_value* v) { return 0; }
SQLITE_API int sqlite3_value_bytes16(sqlite3_value* v) { return 0; }
SQLITE_API double sqlite3_value_double(sqlite3_value* v) { return 0; }
SQLITE_API int sqlite3_value_int(sqlite3_value* v) { return 0; }
SQLITE_API sqlite3_int64 sqlite3_value_int64(sqlite3_value* v) { return 0; }
SQLITE_API const unsigned char *sqlite3_value_text(sqlite3_value* v) { return 0; }
SQLITE_API const void *sqlite3_value_text16(sqlite3_value* v) { return 0; }
SQLITE_API const void *sqlite3_value_text16le(sqlite3_value* v) { return 0; }
SQLITE_API const void *sqlite3_value_text16be(sqlite3_value* v) { return 0; }
SQLITE_API int sqlite3_value_type(sqlite3_value* v) { return 0; }
SQLITE_API int sqlite3_value_numeric_type(sqlite3_value* v) { return 0; }
SQLITE_API unsigned int sqlite3_value_subtype(sqlite3_value* v) { return 0; }
SQLITE_API sqlite3_value *sqlite3_value_dup(const sqlite3_value* v) { return 0; }
SQLITE_API void sqlite3_value_free(sqlite3_value* v) { }
SQLITE_API void *sqlite3_aggregate_context(sqlite3_context* c, int nBytes) { return 0; }
SQLITE_API void *sqlite3_user_data(sqlite3_context* c) { return 0; }
SQLITE_API sqlite3 *sqlite3_context_db_handle(sqlite3_context* c) { return 0; }
SQLITE_API void *sqlite3_get_auxdata(sqlite3_context* c, int N) { return 0; }
SQLITE_API void sqlite3_set_auxdata(sqlite3_context* c, int N, void *v, void (*f)(void*)) { }
SQLITE_API void sqlite3_result_blob(sqlite3_context* c, const void *v, int i, void(*f)(void*)) { }
SQLITE_API void sqlite3_result_blob64(sqlite3_context* c,const void *v,
                           sqlite3_uint64 i,void(*f)(void*)) { }
SQLITE_API void sqlite3_result_double(sqlite3_context* c, double d) { }
SQLITE_API void sqlite3_result_error(sqlite3_context* c, const char* s, int i) { }
SQLITE_API void sqlite3_result_error16(sqlite3_context* c, const void *v, int i) { }
SQLITE_API void sqlite3_result_error_toobig(sqlite3_context* c) { }
SQLITE_API void sqlite3_result_error_nomem(sqlite3_context* c) { }
SQLITE_API void sqlite3_result_error_code(sqlite3_context* c, int i) { }
SQLITE_API void sqlite3_result_int(sqlite3_context* c, int i) { }
SQLITE_API void sqlite3_result_int64(sqlite3_context* c, sqlite3_int64 i) { }
SQLITE_API void sqlite3_result_null(sqlite3_context* c) { }
SQLITE_API void sqlite3_result_text(sqlite3_context* c, const char* s, int i, void(*f)(void*)) { }
SQLITE_API void sqlite3_result_text64(sqlite3_context* c, const char* s, sqlite3_uint64 i,
                           void(*f)(void*), unsigned char encoding) { }
SQLITE_API void sqlite3_result_text16(sqlite3_context* c, const void *v, int i, void(*f)(void*)) { }
SQLITE_API void sqlite3_result_text16le(sqlite3_context* c, const void *v, int i,void(*f)(void*)) { }
SQLITE_API void sqlite3_result_text16be(sqlite3_context* c, const void *v, int i,void(*f)(void*)) { }
SQLITE_API void sqlite3_result_value(sqlite3_context* c, sqlite3_value* v) { }
SQLITE_API void sqlite3_result_zeroblob(sqlite3_context* c, int n) { }
SQLITE_API int sqlite3_result_zeroblob64(sqlite3_context* c, sqlite3_uint64 n) { return 0; }
SQLITE_API void sqlite3_result_subtype(sqlite3_context* c,unsigned int i) { }
SQLITE_API int sqlite3_create_collation(
  sqlite3 *db,
  const char *zName,
  int eTextRep,
  void *pArg,
  int(*xCompare)(void *,int,const void *,int,const void*)
) { return 0; }
SQLITE_API int sqlite3_create_collation_v2(
  sqlite3 *db,
  const char *zName,
  int eTextRep,
  void *pArg,
  int(*xCompare)(void *,int,const void*,int,const void*),
  void(*xDestroy)(void*)
) { return 0; }
SQLITE_API int sqlite3_create_collation16(
  sqlite3 *db,
  const void *zName,
  int eTextRep,
  void *pArg,
  int(*xCompare)(void *,int,const void *,int,const void*)
) { return 0; }
SQLITE_API int sqlite3_collation_needed(
  sqlite3 *db,
  void *v,
  void(*f)(void *v,sqlite3 *db,int eTextRep,const char*)
) { return 0; }
SQLITE_API int sqlite3_collation_needed16(
  sqlite3 *db,
  void *v,
  void(*f)(void *v,sqlite3 *db,int eTextRep,const void*)
) { return 0; }
#ifdef SQLITE_HAS_CODEC
SQLITE_API int sqlite3_key(
  sqlite3 *db,
  const void *pKey, int nKey
) { return 0; }
SQLITE_API int sqlite3_key_v2(
  sqlite3 *db,
  const char *zDbName,
  const void *pKey, int nKey
) { return 0; }
SQLITE_API int sqlite3_rekey(
  sqlite3 *db,
  const void *pKey, int nKey
) { return 0; }
SQLITE_API int sqlite3_rekey_v2(
  sqlite3 *db,
  const char *zDbName,
  const void *pKey, int nKey
) { return 0; }
SQLITE_API void sqlite3_activate_see(
  const char *zPassPhrase
) { return 0; }
#endif
#ifdef SQLITE_ENABLE_CEROD
SQLITE_API void sqlite3_activate_cerod(
  const char *zPassPhrase
) { return 0; }
#endif
SQLITE_API int sqlite3_sleep(int i) { return 0; }
SQLITE_API int sqlite3_get_autocommit(sqlite3 *db) { return 0; }
SQLITE_API sqlite3 *sqlite3_db_handle(sqlite3_stmt *s) { return 0; }
SQLITE_API const char *sqlite3_db_filename(sqlite3 *db, const char *zDbName) { return 0; }
SQLITE_API int sqlite3_db_readonly(sqlite3 *db, const char *zDbName) { return 0; }
SQLITE_API sqlite3_stmt *sqlite3_next_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt) { return 0; }
SQLITE_API void *sqlite3_commit_hook(sqlite3 *db, int(*f)(void*), void *v) { return 0; }
SQLITE_API void *sqlite3_rollback_hook(sqlite3 *db, void(*f)(void *), void *v) { return 0; }
SQLITE_API void *sqlite3_update_hook(
  sqlite3 *db,
  void(*f)(void *,int ,char const *,char const *,sqlite3_int64),
  void *v
) { return 0; }
SQLITE_API int sqlite3_enable_shared_cache(int i) { return 0; }
SQLITE_API int sqlite3_release_memory(int i) { return 0; }
SQLITE_API int sqlite3_db_release_memory(sqlite3 *db) { return 0; }
SQLITE_API sqlite3_int64 sqlite3_soft_heap_limit64(sqlite3_int64 N) { return 0; }
SQLITE_API SQLITE_DEPRECATED void sqlite3_soft_heap_limit(int N) { }
SQLITE_API int sqlite3_table_column_metadata(
  sqlite3 *db,
  const char *zDbName,
  const char *zTableName,
  const char *zColumnName,
  char const **pzDataType,
  char const **pzCollSeq,
  int *pNotNull,
  int *pPrimaryKey,
  int *pAutoinc
) { return 0; }
SQLITE_API int sqlite3_load_extension(
  sqlite3 *db,
  const char *zFile,
  const char *zProc,
  char **pzErrMsg
) { return 0; }
SQLITE_API int sqlite3_enable_load_extension(sqlite3 *db, int onoff) { return 0; }
SQLITE_API int sqlite3_auto_extension(void(*xEntryPoint)(void)) { return 0; }
SQLITE_API int sqlite3_cancel_auto_extension(void(*xEntryPoint)(void)) { return 0; }
SQLITE_API void sqlite3_reset_auto_extension(void) { }
SQLITE_API int sqlite3_create_module(
  sqlite3 *db,
  const char *zName,
  const sqlite3_module *p,
  void *pClientData
) { return 0; }
SQLITE_API int sqlite3_create_module_v2(
  sqlite3 *db,
  const char *zName,
  const sqlite3_module *p,
  void *pClientData,
  void(*xDestroy)(void*)
) { return 0; }
SQLITE_API int sqlite3_declare_vtab(sqlite3 *db, const char *zSQL) { return 0; }
SQLITE_API int sqlite3_overload_function(sqlite3 *db, const char *zFuncName, int nArg) { return 0; }
SQLITE_API int sqlite3_blob_open(
  sqlite3 *db,
  const char *zDb,
  const char *zTable,
  const char *zColumn,
  sqlite3_int64 iRow,
  int flags,
  sqlite3_blob **ppBlob
) { return 0; }
SQLITE_API int sqlite3_blob_reopen(sqlite3_blob *b, sqlite3_int64 i) { return 0; }
SQLITE_API int sqlite3_blob_close(sqlite3_blob *b) { return 0; }
SQLITE_API int sqlite3_blob_bytes(sqlite3_blob *b) { return 0; }
SQLITE_API int sqlite3_blob_read(sqlite3_blob *b, void *Z, int N, int iOffset) { return 0; }
SQLITE_API int sqlite3_blob_write(sqlite3_blob *b, const void *z, int n, int iOffset) { return 0; }
SQLITE_API sqlite3_vfs *sqlite3_vfs_find(const char *zVfsName) { return 0; }
SQLITE_API int sqlite3_vfs_register(sqlite3_vfs *v, int makeDflt) { return 0; }
SQLITE_API int sqlite3_vfs_unregister(sqlite3_vfs *v) { return 0; }
SQLITE_API sqlite3_mutex *sqlite3_mutex_alloc(int i) { return 0; }
SQLITE_API void sqlite3_mutex_free(sqlite3_mutex *m) { }
SQLITE_API void sqlite3_mutex_enter(sqlite3_mutex *m) { }
SQLITE_API int sqlite3_mutex_try(sqlite3_mutex *m) { return 0; }
SQLITE_API void sqlite3_mutex_leave(sqlite3_mutex *m) { }
#ifndef NDEBUG
SQLITE_API int sqlite3_mutex_held(sqlite3_mutex *m) { return 0; }
SQLITE_API int sqlite3_mutex_notheld(sqlite3_mutex *m) { return 0; }
#endif
SQLITE_API sqlite3_mutex *sqlite3_db_mutex(sqlite3* db) { return 0; }
SQLITE_API int sqlite3_file_control(sqlite3 *db, const char *zDbName, int op, void* v) { return 0; }
SQLITE_API int sqlite3_test_control(int op, ...) { return 0; }
SQLITE_API int sqlite3_status(int op, int *pCurrent, int *pHighwater, int resetFlag) { return 0; }
SQLITE_API int sqlite3_status64(
  int op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  int resetFlag
) { return 0; }
SQLITE_API int sqlite3_db_status(sqlite3 *db, int op, int *pCur, int *pHiwtr, int resetFlg) { return 0; }
SQLITE_API int sqlite3_stmt_status(sqlite3_stmt* s, int op,int resetFlg) { return 0; }
SQLITE_API sqlite3_backup *sqlite3_backup_init(
  sqlite3 *pDest,
  const char *zDestName,
  sqlite3 *pSource,
  const char *zSourceName
) { return 0; }
SQLITE_API int sqlite3_backup_step(sqlite3_backup *p, int nPage) { return 0; }
SQLITE_API int sqlite3_backup_finish(sqlite3_backup *p) { return 0; }
SQLITE_API int sqlite3_backup_remaining(sqlite3_backup *p) { return 0; }
SQLITE_API int sqlite3_backup_pagecount(sqlite3_backup *p) { return 0; }
SQLITE_API int sqlite3_unlock_notify(
  sqlite3 *pBlocked,
  void (*xNotify)(void **apArg, int nArg),
  void *pNotifyArg
) { return 0; }
SQLITE_API int sqlite3_stricmp(const char *s, const char *ss) { return 0; }
SQLITE_API int sqlite3_strnicmp(const char *s, const char *ss, int i) { return 0; }
SQLITE_API int sqlite3_strglob(const char *zGlob, const char *zStr) { return 0; }
SQLITE_API int sqlite3_strlike(const char *zGlob, const char *zStr, unsigned int cEsc) { return 0; }
SQLITE_API void sqlite3_log(int iErrCode, const char *zFormat, ...) { }
SQLITE_API void *sqlite3_wal_hook(
  sqlite3 *db,
  int(*f)(void *,sqlite3 *db,const char*,int i),
  void *v
) { return 0; }
SQLITE_API int sqlite3_wal_autocheckpoint(sqlite3 *db, int N) { return 0; }
SQLITE_API int sqlite3_wal_checkpoint(sqlite3 *db, const char *zDb) { return 0; }
SQLITE_API int sqlite3_wal_checkpoint_v2(
  sqlite3 *db,
  const char *zDb,
  int eMode,
  int *pnLog,
  int *pnCkpt
) { return 0; }
#define SQLITE_CHECKPOINT_PASSIVE  0
#define SQLITE_CHECKPOINT_FULL     1
#define SQLITE_CHECKPOINT_RESTART  2
#define SQLITE_CHECKPOINT_TRUNCATE 3
SQLITE_API int sqlite3_vtab_config(sqlite3 *db, int op, ...) { return 0; }
#define SQLITE_VTAB_CONSTRAINT_SUPPORT 1
SQLITE_API int sqlite3_vtab_on_conflict(sqlite3 *db) { return 0; }
#define SQLITE_ROLLBACK 1
#define SQLITE_FAIL     3
#define SQLITE_REPLACE  5
#define SQLITE_SCANSTAT_NLOOP    0
#define SQLITE_SCANSTAT_NVISIT   1
#define SQLITE_SCANSTAT_EST      2
#define SQLITE_SCANSTAT_NAME     3
#define SQLITE_SCANSTAT_EXPLAIN  4
#define SQLITE_SCANSTAT_SELECTID 5
SQLITE_API int sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,
  int idx,
  int iScanStatusOp,
  void *pOut
) { return 0; }
SQLITE_API void sqlite3_stmt_scanstatus_reset(sqlite3_stmt *s) { }
SQLITE_API int sqlite3_db_cacheflush(sqlite3* db) { return 0; }
#if defined(SQLITE_ENABLE_PREUPDATE_HOOK)
SQLITE_API void *sqlite3_preupdate_hook(
  sqlite3 *db,
  void(*xPreUpdate)(
    void *pCtx,
    sqlite3 *db,
    int op,
    char const *zDb,
    char const *zName,
    sqlite3_int64 iKey1,
    sqlite3_int64 iKey2
  ),
  void*
) { return 0; }
SQLITE_API int sqlite3_preupdate_old(sqlite3 *, int i, sqlite3_value **) { return 0; }
SQLITE_API int sqlite3_preupdate_count(sqlite3 *) { return 0; }
SQLITE_API int sqlite3_preupdate_depth(sqlite3 *) { return 0; }
SQLITE_API int sqlite3_preupdate_new(sqlite3 *, int i, sqlite3_value **) { return 0; }
#endif
SQLITE_API int sqlite3_system_errno(sqlite3 *db) { return 0; }
SQLITE_API SQLITE_EXPERIMENTAL int sqlite3_snapshot_get(
  sqlite3 *db,
  const char *zSchema,
  sqlite3_snapshot **ppSnapshot
) { return 0; }
SQLITE_API SQLITE_EXPERIMENTAL int sqlite3_snapshot_open(
  sqlite3 *db,
  const char *zSchema,
  sqlite3_snapshot *pSnapshot
) { return 0; }
SQLITE_API SQLITE_EXPERIMENTAL void sqlite3_snapshot_free(sqlite3_snapshot* s) { }
SQLITE_API SQLITE_EXPERIMENTAL int sqlite3_snapshot_cmp(
  sqlite3_snapshot *p1,
  sqlite3_snapshot *p2
) { return 0; }
SQLITE_API SQLITE_EXPERIMENTAL int sqlite3_snapshot_recover(sqlite3 *db, const char *zDb) { return 0; }
SQLITE_API int sqlite3_rtree_geometry_callback(
  sqlite3 *db,
  const char *zGeom,
  int (*xGeom)(sqlite3_rtree_geometry*, int i, sqlite3_rtree_dbl*,int*),
  void *pContext
) { return 0; }
SQLITE_API int sqlite3_rtree_query_callback(
  sqlite3 *db,
  const char *zQueryFunc,
  int (*xQueryFunc)(sqlite3_rtree_query_info*),
  void *pContext,
  void (*xDestructor)(void*)
) { return 0; }
typedef struct sqlite3_session sqlite3_session;
typedef struct sqlite3_changeset_iter sqlite3_changeset_iter;
SQLITE_API int sqlite3session_create(
  sqlite3 *db,
  const char *zDb,
  sqlite3_session **ppSession
) { return 0; }
SQLITE_API void sqlite3session_delete(sqlite3_session *pSession) { }
SQLITE_API int sqlite3session_enable(sqlite3_session *pSession, int bEnable) { return 0; }
SQLITE_API int sqlite3session_indirect(sqlite3_session *pSession, int bIndirect) { return 0; }
SQLITE_API int sqlite3session_attach(
  sqlite3_session *pSession,
  const char *zTab
) { return 0; }
SQLITE_API void sqlite3session_table_filter(
  sqlite3_session *pSession,
  int(*xFilter)(
    void *pCtx,
    const char *zTab
  ),
  void *pCtx
) { }
SQLITE_API int sqlite3session_changeset(
  sqlite3_session *pSession,
  int *pnChangeset,
  void **ppChangeset
) { return 0; }
SQLITE_API int sqlite3session_diff(
  sqlite3_session *pSession,
  const char *zFromDb,
  const char *zTbl,
  char **pzErrMsg
) { return 0; }
SQLITE_API int sqlite3session_patchset(
  sqlite3_session *pSession,
  int *pnPatchset,
  void **ppPatchset
) { return 0; }
SQLITE_API int sqlite3session_isempty(sqlite3_session *pSession) { return 0; }
SQLITE_API int sqlite3changeset_start(
  sqlite3_changeset_iter **pp,
  int nChangeset,
  void *pChangeset
) { return 0; }
SQLITE_API int sqlite3changeset_next(sqlite3_changeset_iter *pIter) { return 0; }
SQLITE_API int sqlite3changeset_op(
  sqlite3_changeset_iter *pIter,
  const char **pzTab,
  int *pnCol,
  int *pOp,
  int *pbIndirect
) { return 0; }
SQLITE_API int sqlite3changeset_pk(
  sqlite3_changeset_iter *pIter,
  unsigned char **pabPK,
  int *pnCol
) { return 0; }
SQLITE_API int sqlite3changeset_old(
  sqlite3_changeset_iter *pIter,
  int iVal,
  sqlite3_value **ppValue
) { return 0; }
SQLITE_API int sqlite3changeset_new(
  sqlite3_changeset_iter *pIter,
  int iVal,
  sqlite3_value **ppValue
) { return 0; }
SQLITE_API int sqlite3changeset_conflict(
  sqlite3_changeset_iter *pIter,
  int iVal,
  sqlite3_value **ppValue
) { return 0; }
SQLITE_API int sqlite3changeset_fk_conflicts(
  sqlite3_changeset_iter *pIter,
  int *pnOut
) { return 0; }
SQLITE_API int sqlite3changeset_finalize(sqlite3_changeset_iter *pIter) { return 0; }
SQLITE_API int sqlite3changeset_invert(
  int nIn, const void *pIn,
  int *pnOut, void **ppOut
) { return 0; }
SQLITE_API int sqlite3changeset_concat(
  int nA,
  void *pA,
  int nB,
  void *pB,
  int *pnOut,
  void **ppOut
) { return 0; }
typedef struct sqlite3_changegroup sqlite3_changegroup;
int sqlite3changegroup_new(sqlite3_changegroup **pp) { return 0; }
int sqlite3changegroup_add(sqlite3_changegroup *c, int nData, void *pData) { return 0; }
int sqlite3changegroup_output(
  sqlite3_changegroup *c,
  int *pnData,
  void **ppData
) { return 0; }
void sqlite3changegroup_delete(sqlite3_changegroup *c) { }
SQLITE_API int sqlite3changeset_apply(
  sqlite3 *db,
  int nChangeset,
  void *pChangeset,
  int(*xFilter)(
    void *pCtx,
    const char *zTab
  ),
  int(*xConflict)(
    void *pCtx,
    int eConflict,
    sqlite3_changeset_iter *p
  ),
  void *pCtx
) { return 0; }
SQLITE_API int sqlite3changeset_apply_strm(
  sqlite3 *db,
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int(*xFilter)(
    void *pCtx,
    const char *zTab
  ),
  int(*xConflict)(
    void *pCtx,
    int eConflict,
    sqlite3_changeset_iter *p
  ),
  void *pCtx
) { return 0; }
SQLITE_API int sqlite3changeset_concat_strm(
  int (*xInputA)(void *pIn, void *pData, int *pnData),
  void *pInA,
  int (*xInputB)(void *pIn, void *pData, int *pnData),
  void *pInB,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
) { return 0; }
SQLITE_API int sqlite3changeset_invert_strm(
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
) { return 0; }
SQLITE_API int sqlite3changeset_start_strm(
  sqlite3_changeset_iter **pp,
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn
) { return 0; }
SQLITE_API int sqlite3session_changeset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
) { return 0; }
SQLITE_API int sqlite3session_patchset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
) { return 0; }
int sqlite3changegroup_add_strm(sqlite3_changegroup *c,
    int (*xInput)(void *pIn, void *pData, int *pnData),
    void *pIn
) { return 0; }
int sqlite3changegroup_output_strm(sqlite3_changegroup *c,
    int (*xOutput)(void *pOut, const void *pData, int nData),
    void *pOut
) { return 0; }
