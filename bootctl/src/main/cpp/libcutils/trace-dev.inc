/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef __TRACE_DEV_INC
#define __TRACE_DEV_INC

#define LOG_TAG "cutils-trace"

#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <cutils/compiler.h>
#include <cutils/properties.h>
#include <cutils/trace.h>
#include <log/log.h>
#include <log/log_properties.h>

#if defined(__BIONIC__)
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

/**
 * Maximum size of a message that can be logged to the trace buffer.
 * Note this message includes a tag, the pid, and the string given as the name.
 * Names should be kept short to get the most use of the trace buffer.
 */
#define ATRACE_MESSAGE_LENGTH 1024

constexpr uint32_t kSeqNoNotInit = static_cast<uint32_t>(-1);

atomic_bool              atrace_is_ready      = ATOMIC_VAR_INIT(false);
int                      atrace_marker_fd     = -1;
uint64_t                 atrace_enabled_tags  = ATRACE_TAG_NOT_READY;
static atomic_bool       atrace_is_enabled    = ATOMIC_VAR_INIT(true);
static pthread_mutex_t   atrace_tags_mutex    = PTHREAD_MUTEX_INITIALIZER;

/**
 * Sequence number of debug.atrace.tags.enableflags the last time the enabled
 * tags were reloaded.
 **/
static _Atomic(uint32_t) last_sequence_number = ATOMIC_VAR_INIT(kSeqNoNotInit);

#if defined(__BIONIC__)
// All zero prop_info that has a sequence number of 0. This is easier than
// depending on implementation details of the property implementation.
//
// prop_info is static_assert-ed to be 96 bytes, which cannot change due to
// ABI compatibility.
alignas(uint64_t) static char empty_pi[96];
static const prop_info* atrace_property_info = reinterpret_cast<const prop_info*>(empty_pi);
#endif

/**
 * This is called when the sequence number of debug.atrace.tags.enableflags
 * changes and we need to reload the enabled tags.
 **/
static void atrace_seq_number_changed(uint32_t prev_seq_no, uint32_t seq_no);

void atrace_init() {
#if defined(__BIONIC__)
    uint32_t seq_no = __system_property_serial(atrace_property_info);  // Acquire semantics.
#else
    uint32_t seq_no = 0;
#endif
    uint32_t prev_seq_no = atomic_load_explicit(&last_sequence_number, memory_order_relaxed);
    if (CC_UNLIKELY(seq_no != prev_seq_no)) {
        atrace_seq_number_changed(prev_seq_no, seq_no);
    }
}

uint64_t atrace_get_enabled_tags()
{
    atrace_init();
    return atrace_enabled_tags;
}

// Check whether the given command line matches one of the comma-separated
// values listed in the app_cmdlines property.
static bool atrace_is_cmdline_match(const char* cmdline)
{
    int count = property_get_int32("debug.atrace.app_number", 0);

    char buf[PROPERTY_KEY_MAX];
    char value[PROPERTY_VALUE_MAX];

    for (int i = 0; i < count; i++) {
        snprintf(buf, sizeof(buf), "debug.atrace.app_%d", i);
        property_get(buf, value, "");
        if (fnmatch(value, cmdline, FNM_NOESCAPE) == 0) {
            return true;
        }
    }

    return false;
}

// Determine whether application-level tracing is enabled for this process.
static bool atrace_is_app_tracing_enabled()
{
    bool result = false;

    // Check whether tracing is enabled for this process.
    FILE * file = fopen("/proc/self/cmdline", "re");
    if (file) {
        char cmdline[4096];
        if (fgets(cmdline, sizeof(cmdline), file)) {
            result = atrace_is_cmdline_match(cmdline);
        } else {
            ALOGE("Error reading cmdline: %s (%d)", strerror(errno), errno);
        }
        fclose(file);
    } else {
        ALOGE("Error opening /proc/self/cmdline: %s (%d)", strerror(errno),
                errno);
    }

    return result;
}

// Read the sysprop and return the value tags should be set to
static uint64_t atrace_get_property()
{
    char value[PROPERTY_VALUE_MAX];
    char *endptr;
    uint64_t tags;

    property_get("debug.atrace.tags.enableflags", value, "0");
    errno = 0;
    tags = strtoull(value, &endptr, 0);
    if (value[0] == '\0' || *endptr != '\0') {
        ALOGE("Error parsing trace property: Not a number: %s", value);
        return 0;
    } else if (errno == ERANGE || tags == ULLONG_MAX) {
        ALOGE("Error parsing trace property: Number too large: %s", value);
        return 0;
    }

    // Only set the "app" tag if this process was selected for app-level debug
    // tracing.
    if (atrace_is_app_tracing_enabled()) {
        tags |= ATRACE_TAG_APP;
    } else {
        tags &= ~ATRACE_TAG_APP;
    }

    return (tags | ATRACE_TAG_ALWAYS) & ATRACE_TAG_VALID_MASK;
}

// Update tags if tracing is ready. Useful as a sysprop change callback.
void atrace_update_tags()
{
    uint64_t tags;
    if (atomic_load_explicit(&atrace_is_enabled, memory_order_acquire)) {
        tags = atrace_get_property();
        pthread_mutex_lock(&atrace_tags_mutex);
        atrace_enabled_tags = tags;
        pthread_mutex_unlock(&atrace_tags_mutex);
    } else {
        // Tracing is disabled for this process, so we simply don't
        // initialize the tags.
        pthread_mutex_lock(&atrace_tags_mutex);
        atrace_enabled_tags = ATRACE_TAG_NOT_READY;
        pthread_mutex_unlock(&atrace_tags_mutex);
    }
}

#define WRITE_MSG(format_begin, format_end, name, value) { \
    char buf[ATRACE_MESSAGE_LENGTH] __attribute__((uninitialized));     \
    int pid = getpid(); \
    int len = snprintf(buf, sizeof(buf), format_begin "%s" format_end, pid, \
        name, value); \
    if (len >= (int) sizeof(buf)) { \
        /* Given the sizeof(buf), and all of the current format buffers, \
         * it is impossible for name_len to be < 0 if len >= sizeof(buf). */ \
        int name_len = strlen(name) - (len - sizeof(buf)) - 1; \
        /* Truncate the name to make the message fit. */ \
        ALOGW("Truncated name in %s: %s\n", __FUNCTION__, name); \
        len = snprintf(buf, sizeof(buf), format_begin "%.*s" format_end, pid, \
            name_len, name, value); \
    } \
    write(atrace_marker_fd, buf, len); \
}

#endif  // __TRACE_DEV_INC
