//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <ctime>

#include <ctime>
#include <type_traits>
#include "test_macros.h"

#ifndef NULL
#error NULL not defined
#endif

#ifndef CLOCKS_PER_SEC
#error CLOCKS_PER_SEC not defined
#endif

#if TEST_STD_VER > 14 && defined(TEST_HAS_C11_FEATURES)
#ifndef TIME_UTC
#error TIME_UTC not defined
#endif
#endif

int main()
{
    std::clock_t c = 0;
    std::size_t s = 0;
    std::time_t t = 0;
    std::tm tm = {};
#if TEST_STD_VER > 14 && defined(TEST_HAS_C11_FEATURES)
    std::timespec tmspec = {};
    ((void)tmspec); // Prevent unused warning
#endif
    ((void)c); // Prevent unused warning
    ((void)s); // Prevent unused warning
    ((void)t); // Prevent unused warning
    ((void)tm); // Prevent unused warning
    static_assert((std::is_same<decltype(std::clock()), std::clock_t>::value), "");
    static_assert((std::is_same<decltype(std::difftime(t,t)), double>::value), "");
    static_assert((std::is_same<decltype(std::mktime(&tm)), std::time_t>::value), "");
    static_assert((std::is_same<decltype(std::time(&t)), std::time_t>::value), "");
#if TEST_STD_VER > 14 && defined(TEST_HAS_TIMESPEC_GET)
    static_assert((std::is_same<decltype(std::timespec_get(nullptr, 0)), int>::value), "");
#endif
#ifndef _LIBCPP_HAS_NO_THREAD_UNSAFE_C_FUNCTIONS
    static_assert((std::is_same<decltype(std::asctime(&tm)), char*>::value), "");
    static_assert((std::is_same<decltype(std::ctime(&t)), char*>::value), "");
    static_assert((std::is_same<decltype(std::gmtime(&t)), std::tm*>::value), "");
    static_assert((std::is_same<decltype(std::localtime(&t)), std::tm*>::value), "");
#endif
    char* c1 = 0;
    const char* c2 = 0;
    ((void)c1); // Prevent unused warning
    ((void)c2); // Prevent unused warning
    static_assert((std::is_same<decltype(std::strftime(c1,s,c2,&tm)), std::size_t>::value), "");
}
