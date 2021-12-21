//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string.h>

#include <string.h>
#include <type_traits>

#ifndef NULL
#error NULL not defined
#endif

int main()
{
    size_t s = 0;
    void* vp = 0;
    const void* vpc = 0;
    char* cp = 0;
    const char* cpc = 0;
    static_assert((std::is_same<decltype(memcpy(vp, vpc, s)), void*>::value), "");
    static_assert((std::is_same<decltype(memmove(vp, vpc, s)), void*>::value), "");
    static_assert((std::is_same<decltype(strcpy(cp, cpc)), char*>::value), "");
    static_assert((std::is_same<decltype(strncpy(cp, cpc, s)), char*>::value), "");
    static_assert((std::is_same<decltype(strcat(cp, cpc)), char*>::value), "");
    static_assert((std::is_same<decltype(strncat(cp, cpc, s)), char*>::value), "");
    static_assert((std::is_same<decltype(memcmp(vpc, vpc, s)), int>::value), "");
    static_assert((std::is_same<decltype(strcmp(cpc, cpc)), int>::value), "");
    static_assert((std::is_same<decltype(strncmp(cpc, cpc, s)), int>::value), "");
    static_assert((std::is_same<decltype(strcoll(cpc, cpc)), int>::value), "");
    static_assert((std::is_same<decltype(strxfrm(cp, cpc, s)), size_t>::value), "");
    static_assert((std::is_same<decltype(memchr(vp, 0, s)), void*>::value), "");
    static_assert((std::is_same<decltype(strchr(cp, 0)), char*>::value), "");
    static_assert((std::is_same<decltype(strcspn(cpc, cpc)), size_t>::value), "");
    static_assert((std::is_same<decltype(strpbrk(cp, cpc)), char*>::value), "");
    static_assert((std::is_same<decltype(strrchr(cp, 0)), char*>::value), "");
    static_assert((std::is_same<decltype(strspn(cpc, cpc)), size_t>::value), "");
    static_assert((std::is_same<decltype(strstr(cp, cpc)), char*>::value), "");
#ifndef _LIBCPP_HAS_NO_THREAD_UNSAFE_C_FUNCTIONS
    static_assert((std::is_same<decltype(strtok(cp, cpc)), char*>::value), "");
#endif
    static_assert((std::is_same<decltype(memset(vp, 0, s)), void*>::value), "");
    static_assert((std::is_same<decltype(strerror(0)), char*>::value), "");
    static_assert((std::is_same<decltype(strlen(cpc)), size_t>::value), "");

    // These tests fail on systems whose C library doesn't provide a correct overload
    // set for strchr, strpbrk, strrchr, strstr, and memchr, unless the compiler is
    // a suitably recent version of Clang.
#if !defined(__APPLE__) || defined(_LIBCPP_PREFERRED_OVERLOAD)
    static_assert((std::is_same<decltype(strchr(cpc, 0)), const char*>::value), "");
    static_assert((std::is_same<decltype(strpbrk(cpc, cpc)), const char*>::value), "");
    static_assert((std::is_same<decltype(strrchr(cpc, 0)), const char*>::value), "");
    static_assert((std::is_same<decltype(strstr(cpc, cpc)), const char*>::value), "");
    static_assert((std::is_same<decltype(memchr(vpc, 0, s)), const void*>::value), "");
#endif
}
