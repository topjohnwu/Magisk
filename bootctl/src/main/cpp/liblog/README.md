Android liblog
--------------

Public Functions and Macros
---------------------------

    /*
     * Please limit to 24 characters for runtime is loggable,
     * 16 characters for persist is loggable, and logcat pretty
     * alignment with limit of 7 characters.
    */
    #define LOG_TAG "yourtag"
    #include <log/log.h>

    ALOG(android_priority, tag, format, ...)
    IF_ALOG(android_priority, tag)
    LOG_PRI(priority, tag, format, ...)
    LOG_PRI_VA(priority, tag, format, args)
    #define LOG_TAG NULL
    ALOGV(format, ...)
    SLOGV(format, ...)
    RLOGV(format, ...)
    ALOGV_IF(cond, format, ...)
    SLOGV_IF(cond, format, ...)
    RLOGV_IF(cond, format, ...)
    IF_ALOGC()
    ALOGD(format, ...)
    SLOGD(format, ...)
    RLOGD(format, ...)
    ALOGD_IF(cond, format, ...)
    SLOGD_IF(cond, format, ...)
    RLOGD_IF(cond, format, ...)
    IF_ALOGD()
    ALOGI(format, ...)
    SLOGI(format, ...)
    RLOGI(format, ...)
    ALOGI_IF(cond, format, ...)
    SLOGI_IF(cond, format, ...)
    RLOGI_IF(cond, format, ...)
    IF_ALOGI()
    ALOGW(format, ...)
    SLOGW(format, ...)
    RLOGW(format, ...)
    ALOGW_IF(cond, format, ...)
    SLOGW_IF(cond, format, ...)
    RLOGW_IF(cond, format, ...)
    IF_ALOGW()
    ALOGE(format, ...)
    SLOGE(format, ...)
    RLOGE(format, ...)
    ALOGE_IF(cond, format, ...)
    SLOGE_IF(cond, format, ...)
    RLOGE_IF(cond, format, ...)
    IF_ALOGE()
    LOG_FATAL(format, ...)
    LOG_ALWAYS_FATAL(format, ...)
    LOG_FATAL_IF(cond, format, ...)
    LOG_ALWAYS_FATAL_IF(cond, format, ...)
    ALOG_ASSERT(cond, format, ...)
    LOG_EVENT_INT(tag, value)
    LOG_EVENT_LONG(tag, value)

    log_id_t android_logger_get_id(struct logger *logger)
    int android_logger_clear(struct logger *logger)
    int android_logger_get_log_size(struct logger *logger)
    int android_logger_get_log_readable_size(struct logger *logger)
    int android_logger_get_log_version(struct logger *logger)

    struct logger_list *android_logger_list_alloc(int mode, unsigned int tail, pid_t pid)
    struct logger *android_logger_open(struct logger_list *logger_list, log_id_t id)
    struct logger_list *android_logger_list_open(log_id_t id, int mode, unsigned int tail, pid_t pid)
    int android_logger_list_read(struct logger_list *logger_list, struct log_msg *log_msg)
    void android_logger_list_free(struct logger_list *logger_list)

    log_id_t android_name_to_log_id(const char *logName)
    const char *android_log_id_to_name(log_id_t log_id)

    android_log_context create_android_logger(uint32_t tag)

    int android_log_write_list_begin(android_log_context ctx)
    int android_log_write_list_end(android_log_context ctx)

    int android_log_write_int32(android_log_context ctx, int32_t value)
    int android_log_write_int64(android_log_context ctx, int64_t value)
    int android_log_write_string8(android_log_context ctx, const char *value)
    int android_log_write_string8_len(android_log_context ctx, const char *value, size_t maxlen)
    int android_log_write_float32(android_log_context ctx, float value)

    int android_log_write_list(android_log_context ctx, log_id_t id = LOG_ID_EVENTS)

    android_log_context create_android_log_parser(const char *msg, size_t len)
    android_log_list_element android_log_read_next(android_log_context ctx)
    android_log_list_element android_log_peek_next(android_log_context ctx)

    int android_log_destroy(android_log_context *ctx)

Description
-----------

liblog represents an interface to the volatile Android Logging system for NDK (Native) applications
and libraries.  Interfaces for either writing or reading logs.  The log buffers are divided up in
Main, System, Radio and Events sub-logs.

The logging interfaces are a series of macros, all of which can be overridden individually in order
to control the verbosity of the application or library.  `[ASR]LOG[VDIWE]` calls are used to log to
BAsic, System or Radio sub-logs in either the Verbose, Debug, Info, Warning or Error priorities.
`[ASR]LOG[VDIWE]_IF` calls are used to perform thus based on a condition being true.
`IF_ALOG[VDIWE]` calls are true if the current `LOG_TAG` is enabled at the specified priority.
`LOG_ALWAYS_FATAL` is used to `ALOG` a message, then kill the process.  `LOG_FATAL` call is a
variant of `LOG_ALWAYS_FATAL`, only enabled in engineering, and not release builds.  `ALOG_ASSERT`
is used to `ALOG` a message if the condition is false; the condition is part of the logged message.
`LOG_EVENT_(INT|LONG)` is used to drop binary content into the Events sub-log.

The log reading interfaces permit opening the logs either singly or multiply, retrieving a log entry
at a time in time sorted order, optionally limited to a specific pid and tail of the log(s) and
finally a call closing the logs.  A single log can be opened with `android_logger_list_open()`; or
multiple logs can be opened with `android_logger_list_alloc()`, calling in turn the
`android_logger_open()` for each log id.  Each entry can be retrieved with
`android_logger_list_read()`.  The log(s) can be closed with `android_logger_list_free()`.
`ANDROID_LOG_NONBLOCK` mode will report when the log reading is done with an `EAGAIN` error return
code, otherwise the `android_logger_list_read()` call will block for new entries.

The `ANDROID_LOG_WRAP` mode flag to the `android_logger_list_alloc_time()` signals logd to quiesce
the reader until the buffer is about to prune at the start time then proceed to dumping content.

The `ANDROID_LOG_PSTORE` mode flag to the `android_logger_open()` is used to switch from the active
logs to the persistent logs from before the last reboot.

The value returned by `android_logger_open()` can be used as a parameter to the
`android_logger_clear()` function to empty the sub-log.

The value returned by `android_logger_open()` can be used as a parameter to the
`android_logger_get_log_(size|readable_size|version)` to retrieve the sub-log maximum size, readable
size and log buffer format protocol version respectively.  `android_logger_get_id()` returns the id
that was used when opening the sub-log.

Errors
------

If messages fail, a negative error code will be returned to the caller.

The `-ENOTCONN` return code indicates that the logger daemon is stopped.

The `-EBADF` return code indicates that the log access point can not be opened, or the log buffer id
is out of range.

For the `-EAGAIN` return code, this means that the logging message was temporarily backed-up either
because of Denial Of Service (DOS) logging pressure from some chatty application or service in the
Android system, or if too small of a value is set in /proc/sys/net/unix/max_dgram_qlen.  To aid in
diagnosing the occurence of this, a binary event from liblog will be sent to the log daemon once a
new message can get through indicating how many messages were dropped as a result.  Please take
action to resolve the structural problems at the source.

It is generally not advised for the caller to retry the `-EAGAIN` return code as this will only make
the problem(s) worse and cause your application to temporarily drop to the logger daemon priority,
BATCH scheduling policy and background task cgroup. If you require a group of messages to be passed
atomically, merge them into one message with embedded newlines to the maximum length
`LOGGER_ENTRY_MAX_PAYLOAD`.

Other return codes from writing operation can be returned.  Since the library retries on `EINTR`,
`-EINTR` should never be returned.
