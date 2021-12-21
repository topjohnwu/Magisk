/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

/**
 * @addtogroup Logging
 * @{
 */

/**
 * \file
 *
 * Support routines to send messages to the Android log buffer,
 * which can later be accessed through the `logcat` utility.
 *
 * Each log message must have
 *   - a priority
 *   - a log tag
 *   - some text
 *
 * The tag normally corresponds to the component that emits the log message,
 * and should be reasonably small.
 *
 * Log message text may be truncated to less than an implementation-specific
 * limit (1023 bytes).
 *
 * Note that a newline character ("\n") will be appended automatically to your
 * log message, if not already there. It is not possible to send several
 * messages and have them appear on a single line in logcat.
 *
 * Please use logging in moderation:
 *
 *  - Sending log messages eats CPU and slow down your application and the
 *    system.
 *
 *  - The circular log buffer is pretty small, so sending many messages
 *    will hide other important log messages.
 *
 *  - In release builds, only send log messages to account for exceptional
 *    conditions.
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

#if !defined(__BIONIC__) && !defined(__INTRODUCED_IN)
#define __INTRODUCED_IN(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Android log priority values, in increasing order of priority.
 */
typedef enum android_LogPriority {
  /** For internal use only.  */
  ANDROID_LOG_UNKNOWN = 0,
  /** The default priority, for internal use only.  */
  ANDROID_LOG_DEFAULT, /* only for SetMinPriority() */
  /** Verbose logging. Should typically be disabled for a release apk. */
  ANDROID_LOG_VERBOSE,
  /** Debug logging. Should typically be disabled for a release apk. */
  ANDROID_LOG_DEBUG,
  /** Informational logging. Should typically be disabled for a release apk. */
  ANDROID_LOG_INFO,
  /** Warning logging. For use with recoverable failures. */
  ANDROID_LOG_WARN,
  /** Error logging. For use with unrecoverable failures. */
  ANDROID_LOG_ERROR,
  /** Fatal logging. For use when aborting. */
  ANDROID_LOG_FATAL,
  /** For internal use only.  */
  ANDROID_LOG_SILENT, /* only for SetMinPriority(); must be last */
} android_LogPriority;

/**
 * Writes the constant string `text` to the log, with priority `prio` and tag
 * `tag`.
 */
int __android_log_write(int prio, const char* tag, const char* text);

/**
 * Writes a formatted string to the log, with priority `prio` and tag `tag`.
 * The details of formatting are the same as for
 * [printf(3)](http://man7.org/linux/man-pages/man3/printf.3.html).
 */
int __android_log_print(int prio, const char* tag, const char* fmt, ...)
    __attribute__((__format__(printf, 3, 4)));

/**
 * Equivalent to `__android_log_print`, but taking a `va_list`.
 * (If `__android_log_print` is like `printf`, this is like `vprintf`.)
 */
int __android_log_vprint(int prio, const char* tag, const char* fmt, va_list ap)
    __attribute__((__format__(printf, 3, 0)));

/**
 * Writes an assertion failure to the log (as `ANDROID_LOG_FATAL`) and to
 * stderr, before calling
 * [abort(3)](http://man7.org/linux/man-pages/man3/abort.3.html).
 *
 * If `fmt` is non-null, `cond` is unused. If `fmt` is null, the string
 * `Assertion failed: %s` is used with `cond` as the string argument.
 * If both `fmt` and `cond` are null, a default string is provided.
 *
 * Most callers should use
 * [assert(3)](http://man7.org/linux/man-pages/man3/assert.3.html) from
 * `&lt;assert.h&gt;` instead, or the `__assert` and `__assert2` functions
 * provided by bionic if more control is needed. They support automatically
 * including the source filename and line number more conveniently than this
 * function.
 */
void __android_log_assert(const char* cond, const char* tag, const char* fmt, ...)
    __attribute__((__noreturn__)) __attribute__((__format__(printf, 3, 4)));

/**
 * Identifies a specific log buffer for __android_log_buf_write()
 * and __android_log_buf_print().
 */
typedef enum log_id {
  LOG_ID_MIN = 0,

  /** The main log buffer. This is the only log buffer available to apps. */
  LOG_ID_MAIN = 0,
  /** The radio log buffer. */
  LOG_ID_RADIO = 1,
  /** The event log buffer. */
  LOG_ID_EVENTS = 2,
  /** The system log buffer. */
  LOG_ID_SYSTEM = 3,
  /** The crash log buffer. */
  LOG_ID_CRASH = 4,
  /** The statistics log buffer. */
  LOG_ID_STATS = 5,
  /** The security log buffer. */
  LOG_ID_SECURITY = 6,
  /** The kernel log buffer. */
  LOG_ID_KERNEL = 7,

  LOG_ID_MAX,

  /** Let the logging function choose the best log target. */
  LOG_ID_DEFAULT = 0x7FFFFFFF
} log_id_t;

/**
 * Writes the constant string `text` to the log buffer `id`,
 * with priority `prio` and tag `tag`.
 *
 * Apps should use __android_log_write() instead.
 */
int __android_log_buf_write(int bufID, int prio, const char* tag, const char* text);

/**
 * Writes a formatted string to log buffer `id`,
 * with priority `prio` and tag `tag`.
 * The details of formatting are the same as for
 * [printf(3)](http://man7.org/linux/man-pages/man3/printf.3.html).
 *
 * Apps should use __android_log_print() instead.
 */
int __android_log_buf_print(int bufID, int prio, const char* tag, const char* fmt, ...)
    __attribute__((__format__(printf, 4, 5)));

/**
 * Logger data struct used for writing log messages to liblog via __android_log_write_logger_data()
 * and sending log messages to user defined loggers specified in __android_log_set_logger().
 */
struct __android_log_message {
  /** Must be set to sizeof(__android_log_message) and is used for versioning. */
  size_t struct_size;

  /** {@link log_id_t} values. */
  int32_t buffer_id;

  /** {@link android_LogPriority} values. */
  int32_t priority;

  /** The tag for the log message. */
  const char* tag;

  /** Optional file name, may be set to nullptr. */
  const char* file;

  /** Optional line number, ignore if file is nullptr. */
  uint32_t line;

  /** The log message itself. */
  const char* message;
};

/**
 * Prototype for the 'logger' function that is called for every log message.
 */
typedef void (*__android_logger_function)(const struct __android_log_message* log_message);
/**
 * Prototype for the 'abort' function that is called when liblog will abort due to
 * __android_log_assert() failures.
 */
typedef void (*__android_aborter_function)(const char* abort_message);

/**
 * Writes the log message specified by log_message.  log_message includes additional file name and
 * line number information that a logger may use.  log_message is versioned for backwards
 * compatibility.
 * This assumes that loggability has already been checked through __android_log_is_loggable().
 * Higher level logging libraries, such as libbase, first check loggability, then format their
 * buffers, then pass the message to liblog via this function, and therefore we do not want to
 * duplicate the loggability check here.
 *
 * @param log_message the log message itself, see __android_log_message.
 *
 * Available since API level 30.
 */
void __android_log_write_log_message(struct __android_log_message* log_message) __INTRODUCED_IN(30);

/**
 * Sets a user defined logger function.  All log messages sent to liblog will be set to the
 * function pointer specified by logger for processing.  It is not expected that log messages are
 * already terminated with a new line.  This function should add new lines if required for line
 * separation.
 *
 * @param logger the new function that will handle log messages.
 *
 * Available since API level 30.
 */
void __android_log_set_logger(__android_logger_function logger) __INTRODUCED_IN(30);

/**
 * Writes the log message to logd.  This is an __android_logger_function and can be provided to
 * __android_log_set_logger().  It is the default logger when running liblog on a device.
 *
 * @param log_message the log message to write, see __android_log_message.
 *
 * Available since API level 30.
 */
void __android_log_logd_logger(const struct __android_log_message* log_message) __INTRODUCED_IN(30);

/**
 * Writes the log message to stderr.  This is an __android_logger_function and can be provided to
 * __android_log_set_logger().  It is the default logger when running liblog on host.
 *
 * @param log_message the log message to write, see __android_log_message.
 *
 * Available since API level 30.
 */
void __android_log_stderr_logger(const struct __android_log_message* log_message)
    __INTRODUCED_IN(30);

/**
 * Sets a user defined aborter function that is called for __android_log_assert() failures.  This
 * user defined aborter function is highly recommended to abort and be noreturn, but is not strictly
 * required to.
 *
 * @param aborter the new aborter function, see __android_aborter_function.
 *
 * Available since API level 30.
 */
void __android_log_set_aborter(__android_aborter_function aborter) __INTRODUCED_IN(30);

/**
 * Calls the stored aborter function.  This allows for other logging libraries to use the same
 * aborter function by calling this function in liblog.
 *
 * @param abort_message an additional message supplied when aborting, for example this is used to
 *                      call android_set_abort_message() in __android_log_default_aborter().
 *
 * Available since API level 30.
 */
void __android_log_call_aborter(const char* abort_message) __INTRODUCED_IN(30);

/**
 * Sets android_set_abort_message() on device then aborts().  This is the default aborter.
 *
 * @param abort_message an additional message supplied when aborting.  This functions calls
 *                      android_set_abort_message() with its contents.
 *
 * Available since API level 30.
 */
void __android_log_default_aborter(const char* abort_message) __attribute__((noreturn))
__INTRODUCED_IN(30);

/**
 * Use the per-tag properties "log.tag.<tagname>" along with the minimum priority from
 * __android_log_set_minimum_priority() to determine if a log message with a given prio and tag will
 * be printed.  A non-zero result indicates yes, zero indicates false.
 *
 * If both a priority for a tag and a minimum priority are set by
 * __android_log_set_minimum_priority(), then the lowest of the two values are to determine the
 * minimum priority needed to log.  If only one is set, then that value is used to determine the
 * minimum priority needed.  If none are set, then default_priority is used.
 *
 * @param prio         the priority to test, takes android_LogPriority values.
 * @param tag          the tag to test.
 * @param default_prio the default priority to use if no properties or minimum priority are set.
 * @return an integer where 1 indicates that the message is loggable and 0 indicates that it is not.
 *
 * Available since API level 30.
 */
int __android_log_is_loggable(int prio, const char* tag, int default_prio) __INTRODUCED_IN(30);

/**
 * Use the per-tag properties "log.tag.<tagname>" along with the minimum priority from
 * __android_log_set_minimum_priority() to determine if a log message with a given prio and tag will
 * be printed.  A non-zero result indicates yes, zero indicates false.
 *
 * If both a priority for a tag and a minimum priority are set by
 * __android_log_set_minimum_priority(), then the lowest of the two values are to determine the
 * minimum priority needed to log.  If only one is set, then that value is used to determine the
 * minimum priority needed.  If none are set, then default_priority is used.
 *
 * @param prio         the priority to test, takes android_LogPriority values.
 * @param tag          the tag to test.
 * @param len          the length of the tag.
 * @param default_prio the default priority to use if no properties or minimum priority are set.
 * @return an integer where 1 indicates that the message is loggable and 0 indicates that it is not.
 *
 * Available since API level 30.
 */
int __android_log_is_loggable_len(int prio, const char* tag, size_t len, int default_prio)
    __INTRODUCED_IN(30);

/**
 * Sets the minimum priority that will be logged for this process.
 *
 * @param priority the new minimum priority to set, takes android_LogPriority values.
 * @return the previous set minimum priority as android_LogPriority values, or
 *         ANDROID_LOG_DEFAULT if none was set.
 *
 * Available since API level 30.
 */
int32_t __android_log_set_minimum_priority(int32_t priority) __INTRODUCED_IN(30);

/**
 * Gets the minimum priority that will be logged for this process.  If none has been set by a
 * previous __android_log_set_minimum_priority() call, this returns ANDROID_LOG_DEFAULT.
 *
 * @return the current minimum priority as android_LogPriority values, or
 *         ANDROID_LOG_DEFAULT if none is set.
 *
 * Available since API level 30.
 */
int32_t __android_log_get_minimum_priority(void) __INTRODUCED_IN(30);

/**
 * Sets the default tag if no tag is provided when writing a log message.  Defaults to
 * getprogname().  This truncates tag to the maximum log message size, though appropriate tags
 * should be much smaller.
 *
 * @param tag the new log tag.
 *
 * Available since API level 30.
 */
void __android_log_set_default_tag(const char* tag) __INTRODUCED_IN(30);

#ifdef __cplusplus
}
#endif

/** @} */
