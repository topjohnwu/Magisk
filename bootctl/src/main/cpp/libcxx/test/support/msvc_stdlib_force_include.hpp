//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SUPPORT_MSVC_STDLIB_FORCE_INCLUDE_HPP
#define SUPPORT_MSVC_STDLIB_FORCE_INCLUDE_HPP

// This header is force-included when running the libc++ tests against the
// MSVC standard library.

#ifndef _LIBCXX_IN_DEVCRT
    // Silence warnings about CRT machinery.
    #define _CRT_SECURE_NO_WARNINGS

    // Avoid assertion dialogs.
    #define _CRT_SECURE_INVALID_PARAMETER(EXPR) ::abort()
#endif // _LIBCXX_IN_DEVCRT

#include <crtdbg.h>
#include <stdlib.h>

#if defined(_LIBCPP_VERSION)
    #error This header may not be used when targeting libc++
#endif

#ifndef _LIBCXX_IN_DEVCRT
struct AssertionDialogAvoider {
    AssertionDialogAvoider() {
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    }
};

const AssertionDialogAvoider assertion_dialog_avoider{};
#endif // _LIBCXX_IN_DEVCRT

// MSVC frontend only configurations
#if !defined(__clang__)
    // Simulate feature-test macros.
    #define __has_feature(X) _MSVC_HAS_FEATURE_ ## X
    #define _MSVC_HAS_FEATURE_cxx_exceptions    1
    #define _MSVC_HAS_FEATURE_cxx_rtti          1
    #define _MSVC_HAS_FEATURE_address_sanitizer 0
    #define _MSVC_HAS_FEATURE_memory_sanitizer  0
    #define _MSVC_HAS_FEATURE_thread_sanitizer  0

    #define __has_attribute(X) _MSVC_HAS_ATTRIBUTE_ ## X
    #define _MSVC_HAS_ATTRIBUTE_vector_size     0

    // Silence compiler warnings.
    #pragma warning(disable: 4180) // qualifier applied to function type has no meaning; ignored
    #pragma warning(disable: 4324) // structure was padded due to alignment specifier
    #pragma warning(disable: 4521) // multiple copy constructors specified
    #pragma warning(disable: 4702) // unreachable code
    #pragma warning(disable: 28251) // Inconsistent annotation for 'new': this instance has no annotations.
#endif // !defined(__clang__)

// MSVC doesn't have __int128_t.
#define _LIBCPP_HAS_NO_INT128

// MSVC has quick_exit() and at_quick_exit().
#define _LIBCPP_HAS_QUICK_EXIT

#ifndef _LIBCXX_IN_DEVCRT
    // atomic_is_lock_free.pass.cpp needs this VS 2015 Update 2 fix.
    #define _ENABLE_ATOMIC_ALIGNMENT_FIX

    // Silence warnings about features that are deprecated in C++17.
    #define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif // _LIBCXX_IN_DEVCRT

#include <ciso646>

#if _HAS_CXX17
    #define TEST_STD_VER 17
#else // _HAS_CXX17
    #define TEST_STD_VER 14
#endif // _HAS_CXX17

#endif // SUPPORT_MSVC_STDLIB_FORCE_INCLUDE_HPP
