/*
 * Copyright (C) 2010 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _LIBC_LOGGING_H
#define _LIBC_LOGGING_H

#include <sys/cdefs.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

__BEGIN_DECLS

enum {
  ANDROID_LOG_UNKNOWN = 0,
  ANDROID_LOG_DEFAULT,    /* only for SetMinPriority() */

  ANDROID_LOG_VERBOSE,
  ANDROID_LOG_DEBUG,
  ANDROID_LOG_INFO,
  ANDROID_LOG_WARN,
  ANDROID_LOG_ERROR,
  ANDROID_LOG_FATAL,

  ANDROID_LOG_SILENT,     /* only for SetMinPriority(); must be last */
};

enum {
  LOG_ID_MIN = 0,

  LOG_ID_MAIN = 0,
  LOG_ID_RADIO = 1,
  LOG_ID_EVENTS = 2,
  LOG_ID_SYSTEM = 3,
  LOG_ID_CRASH = 4,

  LOG_ID_MAX
};

struct abort_msg_t {
  size_t size;
  char msg[0];
};

//
// Formats a message to the log (priority 'fatal'), then aborts.
//

__LIBC_HIDDEN__ __noreturn void __libc_fatal(const char* format, ...) __printflike(1, 2);

//
// Formats a message to the log (priority 'fatal'), but doesn't abort.
// Used by the malloc implementation to ensure that debuggerd dumps memory
// around the bad address.
//

__LIBC_HIDDEN__ void __libc_fatal_no_abort(const char* format, ...)
    __printflike(1, 2);

//
// Formatting routines for the C library's internal debugging.
// Unlike the usual alternatives, these don't allocate, and they don't drag in all of stdio.
//

__LIBC_HIDDEN__ int __libc_format_buffer(char* buffer, size_t buffer_size, const char* format, ...)
    __printflike(3, 4);

__LIBC_HIDDEN__ int __libc_format_fd(int fd, const char* format, ...)
    __printflike(2, 3);

__LIBC_HIDDEN__ int __libc_format_log(int priority, const char* tag, const char* format, ...)
    __printflike(3, 4);

__LIBC_HIDDEN__ int __libc_format_log_va_list(int priority, const char* tag, const char* format,
                                              va_list ap);

__LIBC_HIDDEN__ int __libc_write_log(int priority, const char* tag, const char* msg);

//
// Event logging.
//

__LIBC_HIDDEN__ void __libc_android_log_event_int(int32_t tag, int value);
__LIBC_HIDDEN__ void __libc_android_log_event_uid(int32_t tag);

__LIBC_HIDDEN__ __noreturn void __fortify_chk_fail(const char* msg, uint32_t event_tag);

__END_DECLS

#endif
