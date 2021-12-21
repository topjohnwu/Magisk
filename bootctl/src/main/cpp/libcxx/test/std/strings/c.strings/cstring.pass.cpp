//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <cstring>

#include <cstring>
#include <type_traits>

#include "test_macros.h"

#ifndef NULL
#error NULL not defined
#endif

int main()
{
    std::size_t s = 0;
    void* vp = 0;
    const void* vpc = 0;
    char* cp = 0;
    const char* cpc = 0;

    ASSERT_SAME_TYPE(void*,       decltype(std::memcpy(vp, vpc, s)));
    ASSERT_SAME_TYPE(void*,       decltype(std::memmove(vp, vpc, s)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strcpy(cp, cpc)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strncpy(cp, cpc, s)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strcat(cp, cpc)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strncat(cp, cpc, s)));
    ASSERT_SAME_TYPE(int,         decltype(std::memcmp(vpc, vpc, s)));
    ASSERT_SAME_TYPE(int,         decltype(std::strcmp(cpc, cpc)));
    ASSERT_SAME_TYPE(int,         decltype(std::strncmp(cpc, cpc, s)));
    ASSERT_SAME_TYPE(int,         decltype(std::strcoll(cpc, cpc)));
    ASSERT_SAME_TYPE(std::size_t, decltype(std::strxfrm(cp, cpc, s)));
    ASSERT_SAME_TYPE(void*,       decltype(std::memchr(vp, 0, s)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strchr(cp, 0)));
    ASSERT_SAME_TYPE(std::size_t, decltype(std::strcspn(cpc, cpc)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strpbrk(cp, cpc)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strrchr(cp, 0)));
    ASSERT_SAME_TYPE(std::size_t, decltype(std::strspn(cpc, cpc)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strstr(cp, cpc)));
#ifndef _LIBCPP_HAS_NO_THREAD_UNSAFE_C_FUNCTIONS
    ASSERT_SAME_TYPE(char*,       decltype(std::strtok(cp, cpc)));
#endif
    ASSERT_SAME_TYPE(void*,       decltype(std::memset(vp, 0, s)));
    ASSERT_SAME_TYPE(char*,       decltype(std::strerror(0)));
    ASSERT_SAME_TYPE(std::size_t, decltype(std::strlen(cpc)));

    // These tests fail on systems whose C library doesn't provide a correct overload
    // set for strchr, strpbrk, strrchr, strstr, and memchr, unless the compiler is
    // a suitably recent version of Clang.
#if !defined(__APPLE__) || defined(_LIBCPP_PREFERRED_OVERLOAD)
    ASSERT_SAME_TYPE(const void*, decltype(std::memchr(vpc, 0, s)));
    ASSERT_SAME_TYPE(const char*, decltype(std::strchr(cpc, 0)));
    ASSERT_SAME_TYPE(const char*, decltype(std::strpbrk(cpc, cpc)));
    ASSERT_SAME_TYPE(const char*, decltype(std::strrchr(cpc, 0)));
    ASSERT_SAME_TYPE(const char*, decltype(std::strstr(cpc, cpc)));
#endif
}
