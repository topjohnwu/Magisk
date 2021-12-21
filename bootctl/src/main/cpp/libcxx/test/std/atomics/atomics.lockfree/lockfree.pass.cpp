//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <atomic>

// #define ATOMIC_BOOL_LOCK_FREE unspecified
// #define ATOMIC_CHAR_LOCK_FREE unspecified
// #define ATOMIC_CHAR16_T_LOCK_FREE unspecified
// #define ATOMIC_CHAR32_T_LOCK_FREE unspecified
// #define ATOMIC_WCHAR_T_LOCK_FREE unspecified
// #define ATOMIC_SHORT_LOCK_FREE unspecified
// #define ATOMIC_INT_LOCK_FREE unspecified
// #define ATOMIC_LONG_LOCK_FREE unspecified
// #define ATOMIC_LLONG_LOCK_FREE unspecified
// #define ATOMIC_POINTER_LOCK_FREE unspecified

#include <atomic>
#include <cassert>

int main()
{
    assert(ATOMIC_BOOL_LOCK_FREE == 0 ||
           ATOMIC_BOOL_LOCK_FREE == 1 ||
           ATOMIC_BOOL_LOCK_FREE == 2);
    assert(ATOMIC_CHAR_LOCK_FREE == 0 ||
           ATOMIC_CHAR_LOCK_FREE == 1 ||
           ATOMIC_CHAR_LOCK_FREE == 2);
    assert(ATOMIC_CHAR16_T_LOCK_FREE == 0 ||
           ATOMIC_CHAR16_T_LOCK_FREE == 1 ||
           ATOMIC_CHAR16_T_LOCK_FREE == 2);
    assert(ATOMIC_CHAR32_T_LOCK_FREE == 0 ||
           ATOMIC_CHAR32_T_LOCK_FREE == 1 ||
           ATOMIC_CHAR32_T_LOCK_FREE == 2);
    assert(ATOMIC_WCHAR_T_LOCK_FREE == 0 ||
           ATOMIC_WCHAR_T_LOCK_FREE == 1 ||
           ATOMIC_WCHAR_T_LOCK_FREE == 2);
    assert(ATOMIC_SHORT_LOCK_FREE == 0 ||
           ATOMIC_SHORT_LOCK_FREE == 1 ||
           ATOMIC_SHORT_LOCK_FREE == 2);
    assert(ATOMIC_INT_LOCK_FREE == 0 ||
           ATOMIC_INT_LOCK_FREE == 1 ||
           ATOMIC_INT_LOCK_FREE == 2);
    assert(ATOMIC_LONG_LOCK_FREE == 0 ||
           ATOMIC_LONG_LOCK_FREE == 1 ||
           ATOMIC_LONG_LOCK_FREE == 2);
    assert(ATOMIC_LLONG_LOCK_FREE == 0 ||
           ATOMIC_LLONG_LOCK_FREE == 1 ||
           ATOMIC_LLONG_LOCK_FREE == 2);
    assert(ATOMIC_POINTER_LOCK_FREE == 0 ||
           ATOMIC_POINTER_LOCK_FREE == 1 ||
           ATOMIC_POINTER_LOCK_FREE == 2);
}
