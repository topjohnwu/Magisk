//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <stdlib.h>

#include <stdlib.h>
#include <type_traits>

#include "test_macros.h"

// As of 1/10/2015 clang emits a -Wnonnull warnings even if the warning occurs
// in an unevaluated context. For this reason we manually suppress the warning.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wnonnull"
#endif

#ifdef abs
#error abs is defined
#endif

#ifdef labs
#error labs is defined
#endif

#ifdef llabs
#error llabs is defined
#endif

#ifdef div
#error div is defined
#endif

#ifdef ldiv
#error ldiv is defined
#endif

#ifdef lldiv
#error lldiv is defined
#endif

#ifndef EXIT_FAILURE
#error EXIT_FAILURE not defined
#endif

#ifndef EXIT_SUCCESS
#error EXIT_SUCCESS not defined
#endif

#ifndef MB_CUR_MAX
#error MB_CUR_MAX not defined
#endif

#ifndef NULL
#error NULL not defined
#endif

#ifndef RAND_MAX
#error RAND_MAX not defined
#endif

int main()
{
    size_t s = 0; ((void)s);
    div_t d; ((void)d);
    ldiv_t ld; ((void)ld);
    lldiv_t lld; ((void)lld);
    char** endptr = 0;
    static_assert((std::is_same<decltype(atof("")), double>::value), "");
    static_assert((std::is_same<decltype(atoi("")), int>::value), "");
    static_assert((std::is_same<decltype(atol("")), long>::value), "");
    static_assert((std::is_same<decltype(atoll("")), long long>::value), "");
    static_assert((std::is_same<decltype(getenv("")), char*>::value), "");
    static_assert((std::is_same<decltype(strtod("", endptr)), double>::value), "");
    static_assert((std::is_same<decltype(strtof("", endptr)), float>::value), "");
    static_assert((std::is_same<decltype(strtold("", endptr)), long double>::value), "");
    static_assert((std::is_same<decltype(strtol("", endptr,0)), long>::value), "");
    static_assert((std::is_same<decltype(strtoll("", endptr,0)), long long>::value), "");
    static_assert((std::is_same<decltype(strtoul("", endptr,0)), unsigned long>::value), "");
    static_assert((std::is_same<decltype(strtoull("", endptr,0)), unsigned long long>::value), "");
    static_assert((std::is_same<decltype(rand()), int>::value), "");
    static_assert((std::is_same<decltype(srand(0)), void>::value), "");

//  Microsoft does not implement aligned_alloc in their C library
#if TEST_STD_VER > 14 && defined(TEST_HAS_C11_FEATURES) && !defined(_WIN32)
    static_assert((std::is_same<decltype(aligned_alloc(0,0)), void*>::value), "");
#endif

    static_assert((std::is_same<decltype(calloc(0,0)), void*>::value), "");
    static_assert((std::is_same<decltype(free(0)), void>::value), "");
    static_assert((std::is_same<decltype(malloc(0)), void*>::value), "");
    static_assert((std::is_same<decltype(realloc(0,0)), void*>::value), "");
    static_assert((std::is_same<decltype(abort()), void>::value), "");
    static_assert((std::is_same<decltype(atexit(0)), int>::value), "");
    static_assert((std::is_same<decltype(exit(0)), void>::value), "");
    static_assert((std::is_same<decltype(_Exit(0)), void>::value), "");
    static_assert((std::is_same<decltype(getenv("")), char*>::value), "");
    static_assert((std::is_same<decltype(system("")), int>::value), "");
    static_assert((std::is_same<decltype(bsearch(0,0,0,0,0)), void*>::value), "");
    static_assert((std::is_same<decltype(qsort(0,0,0,0)), void>::value), "");
    static_assert((std::is_same<decltype(abs(0)), int>::value), "");
    static_assert((std::is_same<decltype(labs((long)0)), long>::value), "");
    static_assert((std::is_same<decltype(llabs((long long)0)), long long>::value), "");
    static_assert((std::is_same<decltype(div(0,0)), div_t>::value), "");
    static_assert((std::is_same<decltype(ldiv(0L,0L)), ldiv_t>::value), "");
    static_assert((std::is_same<decltype(lldiv(0LL,0LL)), lldiv_t>::value), "");
    wchar_t* pw = 0;
    const wchar_t* pwc = 0;
    char* pc = 0;
    static_assert((std::is_same<decltype(mblen("",0)), int>::value), "");
    static_assert((std::is_same<decltype(mbtowc(pw,"",0)), int>::value), "");
    static_assert((std::is_same<decltype(wctomb(pc,L' ')), int>::value), "");
    static_assert((std::is_same<decltype(mbstowcs(pw,"",0)), size_t>::value), "");
    static_assert((std::is_same<decltype(wcstombs(pc,pwc,0)), size_t>::value), "");
}
