/*
 * Copyright (C) 2007 The Android Open Source Project
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

#ifndef ANDROID_CALLSTACK_H
#define ANDROID_CALLSTACK_H

#include <memory>

#include <android/log.h>
#include <backtrace/backtrace_constants.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <stdint.h>
#include <sys/types.h>

#if !defined(__APPLE__) && !defined(_WIN32)
# define WEAKS_AVAILABLE 1
#endif
#ifndef CALLSTACK_WEAK
# ifdef WEAKS_AVAILABLE
#   define CALLSTACK_WEAK __attribute__((weak))
# else // !WEAKS_AVAILABLE
#   define CALLSTACK_WEAK
# endif // !WEAKS_AVAILABLE
#endif // CALLSTACK_WEAK predefined

#define ALWAYS_INLINE __attribute__((always_inline))

namespace android {

class Printer;

// Collect/print the call stack (function, file, line) traces for a single thread.
class CallStack {
public:
    // Create an empty call stack. No-op.
    CallStack();
    // Create a callstack with the current thread's stack trace.
    // Immediately dump it to logcat using the given logtag.
    CallStack(const char* logtag, int32_t ignoreDepth = 1);
    ~CallStack();

    // Reset the stack frames (same as creating an empty call stack).
    void clear() { mFrameLines.clear(); }

    // Immediately collect the stack traces for the specified thread.
    // The default is to dump the stack of the current call.
    void update(int32_t ignoreDepth = 1, pid_t tid = BACKTRACE_CURRENT_THREAD);

    // Dump a stack trace to the log using the supplied logtag.
    void log(const char* logtag,
             android_LogPriority priority = ANDROID_LOG_DEBUG,
             const char* prefix = nullptr) const;

    // Dump a stack trace to the specified file descriptor.
    void dump(int fd, int indent = 0, const char* prefix = nullptr) const;

    // Return a string (possibly very long) containing the complete stack trace.
    String8 toString(const char* prefix = nullptr) const;

    // Dump a serialized representation of the stack trace to the specified printer.
    void print(Printer& printer) const;

    // Get the count of stack frames that are in this call stack.
    size_t size() const { return mFrameLines.size(); }

    // DO NOT USE ANYTHING BELOW HERE. The following public members are expected
    // to disappear again shortly, once a better replacement facility exists.
    // The replacement facility will be incompatible!

    // Debugging accesses to some basic functionality. These use weak symbols to
    // avoid introducing a dependency on libutilscallstack. Such a dependency from
    // libutils results in a cyclic build dependency. These routines can be called
    // from within libutils. But if the actual library is unavailable, they do
    // nothing.
    //
    // DO NOT USE THESE. They will disappear.
    struct StackDeleter {
#ifdef WEAKS_AVAILABLE
        void operator()(CallStack* stack) {
            deleteStack(stack);
        }
#else
        void operator()(CallStack*) {}
#endif
    };

    typedef std::unique_ptr<CallStack, StackDeleter> CallStackUPtr;

    // Return current call stack if possible, nullptr otherwise.
#ifdef WEAKS_AVAILABLE
    static CallStackUPtr ALWAYS_INLINE getCurrent(int32_t ignoreDepth = 1) {
        if (reinterpret_cast<uintptr_t>(getCurrentInternal) == 0) {
            ALOGW("CallStack::getCurrentInternal not linked, returning null");
            return CallStackUPtr(nullptr);
        } else {
            return getCurrentInternal(ignoreDepth);
        }
    }
#else // !WEAKS_AVAILABLE
    static CallStackUPtr ALWAYS_INLINE getCurrent(int32_t = 1) {
        return CallStackUPtr(nullptr);
    }
#endif // !WEAKS_AVAILABLE

#ifdef WEAKS_AVAILABLE
    static void ALWAYS_INLINE logStack(const char* logtag, CallStack* stack = getCurrent().get(),
                                       android_LogPriority priority = ANDROID_LOG_DEBUG) {
        if (reinterpret_cast<uintptr_t>(logStackInternal) != 0 && stack != nullptr) {
            logStackInternal(logtag, stack, priority);
        } else {
            ALOG(LOG_WARN, logtag, "CallStack::logStackInternal not linked");
        }
    }

#else
    static void ALWAYS_INLINE logStack(const char* logtag, CallStack* = getCurrent().get(),
                                       android_LogPriority = ANDROID_LOG_DEBUG) {
        ALOG(LOG_WARN, logtag, "CallStack::logStackInternal not linked");
    }
#endif // !WEAKS_AVAILABLE

#ifdef WEAKS_AVAILABLE
    static String8 ALWAYS_INLINE stackToString(const char* prefix = nullptr,
                                               const CallStack* stack = getCurrent().get()) {
        if (reinterpret_cast<uintptr_t>(stackToStringInternal) != 0 && stack != nullptr) {
            return stackToStringInternal(prefix, stack);
        } else {
            return String8::format("%s<CallStack package not linked>", (prefix ? prefix : ""));
        }
    }
#else // !WEAKS_AVAILABLE
    static String8 ALWAYS_INLINE stackToString(const char* prefix = nullptr,
                                               const CallStack* = getCurrent().get()) {
        return String8::format("%s<CallStack package not linked>", (prefix ? prefix : ""));
    }
#endif // !WEAKS_AVAILABLE

  private:
#ifdef WEAKS_AVAILABLE
    static CallStackUPtr CALLSTACK_WEAK getCurrentInternal(int32_t ignoreDepth);
    static void CALLSTACK_WEAK logStackInternal(const char* logtag, const CallStack* stack,
                                                android_LogPriority priority);
    static String8 CALLSTACK_WEAK stackToStringInternal(const char* prefix, const CallStack* stack);
    // The deleter is only invoked on non-null pointers. Hence it will never be
    // invoked if CallStack is not linked.
    static void CALLSTACK_WEAK deleteStack(CallStack* stack);
#endif // WEAKS_AVAILABLE

    Vector<String8> mFrameLines;
};

}  // namespace android

#endif // ANDROID_CALLSTACK_H
