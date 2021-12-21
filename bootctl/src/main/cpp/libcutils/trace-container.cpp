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

#include <cutils/trace.h>

#include "trace-dev.inc"

#include <cutils/sockets.h>
#include <sys/stat.h>
#include <time.h>

/**
 * For tracing in container, tags are written into a socket
 * instead of ftrace. Additional data is appended so we need extra space.
 */
#define CONTAINER_ATRACE_MESSAGE_LENGTH (ATRACE_MESSAGE_LENGTH + 512)

static pthread_once_t atrace_once_control = PTHREAD_ONCE_INIT;

// Variables used for tracing in container with socket.
// Note that we need to manually close and reopen socket when Zygote is forking. This requires
// writing and closing sockets on multiple threads. A rwlock is used for avoiding concurrent
// operation on the file descriptor.
static bool             atrace_use_container_sock    = false;
static int              atrace_container_sock_fd     = -1;
static pthread_mutex_t  atrace_enabling_mutex        = PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t atrace_container_sock_rwlock = PTHREAD_RWLOCK_INITIALIZER;

static void atrace_seq_number_changed(uint32_t, uint32_t seq_no) {
    pthread_once(&atrace_once_control, atrace_init_once);
    atomic_store_explicit(&last_sequence_number, seq_no, memory_order_relaxed);
}

static bool atrace_init_container_sock()
{
    pthread_rwlock_wrlock(&atrace_container_sock_rwlock);
    atrace_container_sock_fd =
        socket_local_client("trace", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_SEQPACKET);
    if (atrace_container_sock_fd < 0) {
        ALOGE("Error opening container trace socket: %s (%d)", strerror(errno), errno);
    }
    pthread_rwlock_unlock(&atrace_container_sock_rwlock);
    return atrace_container_sock_fd != -1;
}

static void atrace_close_container_sock()
{
    pthread_rwlock_wrlock(&atrace_container_sock_rwlock);
    if (atrace_container_sock_fd != -1) close(atrace_container_sock_fd);
    atrace_container_sock_fd = -1;
    pthread_rwlock_unlock(&atrace_container_sock_rwlock);
}

// Set whether tracing is enabled in this process.  This is used to prevent
// the Zygote process from tracing.  We need to close the socket in the container when tracing is
// disabled, and reopen it again after Zygote forking.
void atrace_set_tracing_enabled(bool enabled)
{
    pthread_mutex_lock(&atrace_enabling_mutex);
    if (atrace_use_container_sock) {
        bool already_enabled = atomic_load_explicit(&atrace_is_enabled, memory_order_acquire);
        if (enabled && !already_enabled) {
            // Trace was disabled previously. Re-initialize container socket.
            atrace_init_container_sock();
        } else if (!enabled && already_enabled) {
            // Trace was enabled previously. Close container socket.
            atrace_close_container_sock();
        }
    }
    atomic_store_explicit(&atrace_is_enabled, enabled, memory_order_release);
    pthread_mutex_unlock(&atrace_enabling_mutex);
    atrace_update_tags();
}

static void atrace_init_once()
{
    atrace_marker_fd = open("/sys/kernel/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
    if (atrace_marker_fd < 0) {
        // try debugfs
        atrace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY | O_CLOEXEC);
        if (atrace_marker_fd < 0) {
            // We're in container, ftrace may be disabled. In such case, we use the
            // socket to write trace event.

            // Protect the initialization of container socket from
            // atrace_set_tracing_enabled.
            pthread_mutex_lock(&atrace_enabling_mutex);
            atrace_use_container_sock = true;
            bool success = false;
            if (atomic_load_explicit(&atrace_is_enabled, memory_order_acquire)) {
                success = atrace_init_container_sock();
            }
            pthread_mutex_unlock(&atrace_enabling_mutex);

            if (!success) {
                atrace_enabled_tags = 0;
                goto done;
            }
        }
    }
    atrace_enabled_tags = atrace_get_property();

done:
    atomic_store_explicit(&atrace_is_ready, true, memory_order_release);
}

void atrace_setup()
{
    pthread_once(&atrace_once_control, atrace_init_once);
}

static inline uint64_t gettime(clockid_t clk_id)
{
    struct timespec ts;
    clock_gettime(clk_id, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

// Write trace events to container trace file. Note that we need to amend tid and time information
// here comparing to normal ftrace, where those informations are added by kernel.
#define WRITE_MSG_IN_CONTAINER_LOCKED(ph, sep_before_name, value_format, name, value) { \
    char buf[CONTAINER_ATRACE_MESSAGE_LENGTH]; \
    int pid = getpid(); \
    int tid = gettid(); \
    uint64_t ts = gettime(CLOCK_MONOTONIC); \
    uint64_t tts = gettime(CLOCK_THREAD_CPUTIME_ID); \
    int len = snprintf( \
            buf, sizeof(buf), \
            ph "|%d|%d|%" PRIu64 "|%" PRIu64 sep_before_name "%s" value_format, \
            pid, tid, ts, tts, name, value); \
    if (len >= (int) sizeof(buf)) { \
        int name_len = strlen(name) - (len - sizeof(buf)) - 1; \
        /* Truncate the name to make the message fit. */ \
        if (name_len > 0) { \
            ALOGW("Truncated name in %s: %s\n", __FUNCTION__, name); \
            len = snprintf( \
                    buf, sizeof(buf), \
                    ph "|%d|%d|%" PRIu64 "|%" PRIu64 sep_before_name "%.*s" value_format, \
                    pid, tid, ts, tts, name_len, name, value); \
        } else { \
            /* Data is still too long. Drop it. */ \
            ALOGW("Data is too long in %s: %s\n", __FUNCTION__, name); \
            len = 0; \
        } \
    } \
    if (len > 0) { \
        write(atrace_container_sock_fd, buf, len); \
    } \
}

#define WRITE_MSG_IN_CONTAINER(ph, sep_before_name, value_format, name, value) { \
    pthread_rwlock_rdlock(&atrace_container_sock_rwlock); \
    if (atrace_container_sock_fd != -1) { \
       WRITE_MSG_IN_CONTAINER_LOCKED(ph, sep_before_name, value_format, name, value); \
    } \
    pthread_rwlock_unlock(&atrace_container_sock_rwlock); \
}

void atrace_begin_body(const char* name)
{
    if (CC_LIKELY(atrace_use_container_sock)) {
        WRITE_MSG_IN_CONTAINER("B", "|", "%s", name, "");
        return;
    }

    if (atrace_marker_fd < 0) return;

    WRITE_MSG("B|%d|", "%s", name, "");
}

void atrace_end_body()
{
    if (CC_LIKELY(atrace_use_container_sock)) {
        WRITE_MSG_IN_CONTAINER("E", "", "%s", "", "");
        return;
    }

    if (atrace_marker_fd < 0) return;

    WRITE_MSG("E|%d", "%s", "", "");
}

void atrace_async_begin_body(const char* name, int32_t cookie)
{
    if (CC_LIKELY(atrace_use_container_sock)) {
        WRITE_MSG_IN_CONTAINER("S", "|", "|%d", name, cookie);
        return;
    }

    if (atrace_marker_fd < 0) return;

    WRITE_MSG("S|%d|", "|%" PRId32, name, cookie);
}

void atrace_async_end_body(const char* name, int32_t cookie)
{
    if (CC_LIKELY(atrace_use_container_sock)) {
        WRITE_MSG_IN_CONTAINER("F", "|", "|%d", name, cookie);
        return;
    }

    if (atrace_marker_fd < 0) return;

    WRITE_MSG("F|%d|", "|%" PRId32, name, cookie);
}

void atrace_int_body(const char* name, int32_t value)
{
    if (CC_LIKELY(atrace_use_container_sock)) {
        WRITE_MSG_IN_CONTAINER("C", "|", "|%" PRId32, name, value);
        return;
    }

    if (atrace_marker_fd < 0) return;

    WRITE_MSG("C|%d|", "|%" PRId32, name, value);
}

void atrace_int64_body(const char* name, int64_t value)
{
    if (CC_LIKELY(atrace_use_container_sock)) {
        WRITE_MSG_IN_CONTAINER("C", "|", "|%" PRId64, name, value);
        return;
    }

    if (atrace_marker_fd < 0) return;

    WRITE_MSG("C|%d|", "|%" PRId64, name, value);
}
