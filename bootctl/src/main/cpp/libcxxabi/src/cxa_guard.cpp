//===---------------------------- cxa_guard.cpp ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "__cxxabi_config.h"

#include "abort_message.h"
#include <__threading_support>

#include <stdint.h>

/*
    This implementation must be careful to not call code external to this file
    which will turn around and try to call __cxa_guard_acquire reentrantly.
    For this reason, the headers of this file are as restricted as possible.
    Previous implementations of this code for __APPLE__ have used
    std::__libcpp_mutex_lock and the abort_message utility without problem. This
    implementation also uses std::__libcpp_condvar_wait which has tested
    to not be a problem.
*/

namespace __cxxabiv1
{

namespace
{

#ifdef __arm__
// A 32-bit, 4-byte-aligned static data value. The least significant 2 bits must
// be statically initialized to 0.
typedef uint32_t guard_type;

inline void set_initialized(guard_type* guard_object) {
    *guard_object |= 1;
}
#else
typedef uint64_t guard_type;

void set_initialized(guard_type* guard_object) {
    char* initialized = (char*)guard_object;
    *initialized = 1;
}
#endif

#if defined(_LIBCXXABI_HAS_NO_THREADS) || (defined(__APPLE__) && !defined(__arm__))
#ifdef __arm__

// Test the lowest bit.
inline bool is_initialized(guard_type* guard_object) {
    return (*guard_object) & 1;
}

#else

bool is_initialized(guard_type* guard_object) {
    char* initialized = (char*)guard_object;
    return *initialized;
}

#endif
#endif

#ifndef _LIBCXXABI_HAS_NO_THREADS
std::__libcpp_mutex_t guard_mut = _LIBCPP_MUTEX_INITIALIZER;
std::__libcpp_condvar_t guard_cv = _LIBCPP_CONDVAR_INITIALIZER;
#endif

#if defined(__APPLE__) && !defined(__arm__)

typedef uint32_t lock_type;

#if __LITTLE_ENDIAN__

inline
lock_type
get_lock(uint64_t x)
{
    return static_cast<lock_type>(x >> 32);
}

inline
void
set_lock(uint64_t& x, lock_type y)
{
    x = static_cast<uint64_t>(y) << 32;
}

#else  // __LITTLE_ENDIAN__

inline
lock_type
get_lock(uint64_t x)
{
    return static_cast<lock_type>(x);
}

inline
void
set_lock(uint64_t& x, lock_type y)
{
    x = y;
}

#endif  // __LITTLE_ENDIAN__

#else  // !__APPLE__ || __arm__

typedef bool lock_type;

#if !defined(__arm__)
static_assert(std::is_same<guard_type, uint64_t>::value, "");

inline lock_type get_lock(uint64_t x)
{
    union
    {
        uint64_t guard;
        uint8_t lock[2];
    } f = {x};
    return f.lock[1] != 0;
}

inline void set_lock(uint64_t& x, lock_type y)
{
    union
    {
        uint64_t guard;
        uint8_t lock[2];
    } f = {0};
    f.lock[1] = y;
    x = f.guard;
}
#else // defined(__arm__)
static_assert(std::is_same<guard_type, uint32_t>::value, "");

inline lock_type get_lock(uint32_t x)
{
    union
    {
        uint32_t guard;
        uint8_t lock[2];
    } f = {x};
    return f.lock[1] != 0;
}

inline void set_lock(uint32_t& x, lock_type y)
{
    union
    {
        uint32_t guard;
        uint8_t lock[2];
    } f = {0};
    f.lock[1] = y;
    x = f.guard;
}

#endif // !defined(__arm__)

#endif  // __APPLE__ && !__arm__

}  // unnamed namespace

extern "C"
{

#ifndef _LIBCXXABI_HAS_NO_THREADS
_LIBCXXABI_FUNC_VIS int __cxa_guard_acquire(guard_type *guard_object) {
    char* initialized = (char*)guard_object;
    if (std::__libcpp_mutex_lock(&guard_mut))
        abort_message("__cxa_guard_acquire failed to acquire mutex");
    int result = *initialized == 0;
    if (result)
    {
#if defined(__APPLE__) && !defined(__arm__)
        // This is a special-case pthread dependency for Mac. We can't pull this
        // out into libcxx's threading API (__threading_support) because not all
        // supported Mac environments provide this function (in pthread.h). To
        // make it possible to build/use libcxx in those environments, we have to
        // keep this pthread dependency local to libcxxabi. If there is some
        // convenient way to detect precisely when pthread_mach_thread_np is
        // available in a given Mac environment, it might still be possible to
        // bury this dependency in __threading_support.
        #ifdef _LIBCPP_HAS_THREAD_API_PTHREAD
           const lock_type id = pthread_mach_thread_np(std::__libcpp_thread_get_current_id());
        #else
           #error "How do I pthread_mach_thread_np()?"
        #endif
        lock_type lock = get_lock(*guard_object);
        if (lock)
        {
            // if this thread set lock for this same guard_object, abort
            if (lock == id)
                abort_message("__cxa_guard_acquire detected deadlock");
            do
            {
                if (std::__libcpp_condvar_wait(&guard_cv, &guard_mut))
                    abort_message("__cxa_guard_acquire condition variable wait failed");
                lock = get_lock(*guard_object);
            } while (lock);
            result = !is_initialized(guard_object);
            if (result)
                set_lock(*guard_object, id);
        }
        else
            set_lock(*guard_object, id);
#else  // !__APPLE__ || __arm__
        while (get_lock(*guard_object))
            if (std::__libcpp_condvar_wait(&guard_cv, &guard_mut))
                abort_message("__cxa_guard_acquire condition variable wait failed");
        result = *initialized == 0;
        if (result)
            set_lock(*guard_object, true);
#endif  // !__APPLE__ || __arm__
    }
    if (std::__libcpp_mutex_unlock(&guard_mut))
        abort_message("__cxa_guard_acquire failed to release mutex");
    return result;
}

_LIBCXXABI_FUNC_VIS void __cxa_guard_release(guard_type *guard_object) {
    if (std::__libcpp_mutex_lock(&guard_mut))
        abort_message("__cxa_guard_release failed to acquire mutex");
    *guard_object = 0;
    set_initialized(guard_object);
    if (std::__libcpp_mutex_unlock(&guard_mut))
        abort_message("__cxa_guard_release failed to release mutex");
    if (std::__libcpp_condvar_broadcast(&guard_cv))
        abort_message("__cxa_guard_release failed to broadcast condition variable");
}

_LIBCXXABI_FUNC_VIS void __cxa_guard_abort(guard_type *guard_object) {
    if (std::__libcpp_mutex_lock(&guard_mut))
        abort_message("__cxa_guard_abort failed to acquire mutex");
    *guard_object = 0;
    if (std::__libcpp_mutex_unlock(&guard_mut))
        abort_message("__cxa_guard_abort failed to release mutex");
    if (std::__libcpp_condvar_broadcast(&guard_cv))
        abort_message("__cxa_guard_abort failed to broadcast condition variable");
}

#else // _LIBCXXABI_HAS_NO_THREADS

_LIBCXXABI_FUNC_VIS int __cxa_guard_acquire(guard_type *guard_object) {
    return !is_initialized(guard_object);
}

_LIBCXXABI_FUNC_VIS void __cxa_guard_release(guard_type *guard_object) {
    *guard_object = 0;
    set_initialized(guard_object);
}

_LIBCXXABI_FUNC_VIS void __cxa_guard_abort(guard_type *guard_object) {
    *guard_object = 0;
}

#endif // !_LIBCXXABI_HAS_NO_THREADS

}  // extern "C"

}  // __cxxabiv1
