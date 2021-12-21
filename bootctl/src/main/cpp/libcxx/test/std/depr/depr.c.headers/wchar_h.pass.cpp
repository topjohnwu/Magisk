//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <wchar.h>

#include <wchar.h>
#include <stdarg.h>
#include <type_traits>

#ifndef NULL
#error NULL not defined
#endif

#ifndef WCHAR_MAX
#error WCHAR_MAX not defined
#endif

#ifndef WCHAR_MIN
#error WCHAR_MIN not defined
#endif

#ifndef WEOF
#error WEOF not defined
#endif

int main()
{
// mbstate_t comes from the underlying C library; it is defined (in C99) as:
//    a complete object type other than an array type that can hold the conversion
//    state information necessary to convert between sequences of multibyte
//    characters and wide characters
    mbstate_t mb = {};
    size_t s = 0;
    tm *tm = 0;
    wint_t w = 0;
    ::FILE* fp = 0;
    ::va_list va;
    char* ns = 0;
    wchar_t* ws = 0;
    ((void)mb); // Prevent unused warning
    ((void)s); // Prevent unused warning
    ((void)tm); // Prevent unused warning
    ((void)w); // Prevent unused warning
    ((void)fp); // Prevent unused warning
    ((void)va); // Prevent unused warning
    ((void)ns); // Prevent unused warning
    ((void)ws); // Prevent unused warning
    static_assert((std::is_same<decltype(fwprintf(fp, L"")), int>::value), "");
    static_assert((std::is_same<decltype(fwscanf(fp, L"")), int>::value), "");
    static_assert((std::is_same<decltype(swprintf(ws, s, L"")), int>::value), "");
    static_assert((std::is_same<decltype(swscanf(L"", L"")), int>::value), "");
    static_assert((std::is_same<decltype(vfwprintf(fp, L"", va)), int>::value), "");
    static_assert((std::is_same<decltype(vfwscanf(fp, L"", va)), int>::value), "");
    static_assert((std::is_same<decltype(vswprintf(ws, s, L"", va)), int>::value), "");
    static_assert((std::is_same<decltype(vswscanf(L"", L"", va)), int>::value), "");
    static_assert((std::is_same<decltype(fgetwc(fp)), wint_t>::value), "");
    static_assert((std::is_same<decltype(fgetws(ws, 0, fp)), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(fputwc(L' ', fp)), wint_t>::value), "");
    static_assert((std::is_same<decltype(fputws(L"", fp)), int>::value), "");
    static_assert((std::is_same<decltype(fwide(fp, 0)), int>::value), "");
    static_assert((std::is_same<decltype(getwc(fp)), wint_t>::value), "");
    static_assert((std::is_same<decltype(putwc(L' ', fp)), wint_t>::value), "");
    static_assert((std::is_same<decltype(ungetwc(L' ', fp)), wint_t>::value), "");
    static_assert((std::is_same<decltype(wcstod(L"", (wchar_t**)0)), double>::value), "");
    static_assert((std::is_same<decltype(wcstof(L"", (wchar_t**)0)), float>::value), "");
    static_assert((std::is_same<decltype(wcstold(L"", (wchar_t**)0)), long double>::value), "");
    static_assert((std::is_same<decltype(wcstol(L"", (wchar_t**)0, 0)), long>::value), "");
    static_assert((std::is_same<decltype(wcstoll(L"", (wchar_t**)0, 0)), long long>::value), "");
    static_assert((std::is_same<decltype(wcstoul(L"", (wchar_t**)0, 0)), unsigned long>::value), "");
    static_assert((std::is_same<decltype(wcstoull(L"", (wchar_t**)0, 0)), unsigned long long>::value), "");
    static_assert((std::is_same<decltype(wcscpy(ws, L"")), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcsncpy(ws, L"", s)), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcscat(ws, L"")), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcsncat(ws, L"", s)), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcscmp(L"", L"")), int>::value), "");
    static_assert((std::is_same<decltype(wcscoll(L"", L"")), int>::value), "");
    static_assert((std::is_same<decltype(wcsncmp(L"", L"", s)), int>::value), "");
    static_assert((std::is_same<decltype(wcsxfrm(ws, L"", s)), size_t>::value), "");
    static_assert((std::is_same<decltype(wcschr((wchar_t*)0, L' ')), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcscspn(L"", L"")), size_t>::value), "");
    static_assert((std::is_same<decltype(wcslen(L"")), size_t>::value), "");
    static_assert((std::is_same<decltype(wcspbrk((wchar_t*)0, L"")), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcsrchr((wchar_t*)0, L' ')), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcsspn(L"", L"")), size_t>::value), "");
    static_assert((std::is_same<decltype(wcsstr((wchar_t*)0, L"")), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcstok(ws, L"", (wchar_t**)0)), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wmemchr((wchar_t*)0, L' ', s)), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wmemcmp(L"", L"", s)), int>::value), "");
    static_assert((std::is_same<decltype(wmemcpy(ws, L"", s)), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wmemmove(ws, L"", s)), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wmemset(ws, L' ', s)), wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcsftime(ws, s, L"", tm)), size_t>::value), "");
    static_assert((std::is_same<decltype(btowc(0)), wint_t>::value), "");
    static_assert((std::is_same<decltype(wctob(w)), int>::value), "");
    static_assert((std::is_same<decltype(mbsinit(&mb)), int>::value), "");
    static_assert((std::is_same<decltype(mbrlen("", s, &mb)), size_t>::value), "");
    static_assert((std::is_same<decltype(mbrtowc(ws, "", s, &mb)), size_t>::value), "");
    static_assert((std::is_same<decltype(wcrtomb(ns, L' ', &mb)), size_t>::value), "");
    static_assert((std::is_same<decltype(mbsrtowcs(ws, (const char**)0, s, &mb)), size_t>::value), "");
    static_assert((std::is_same<decltype(wcsrtombs(ns, (const wchar_t**)0, s, &mb)), size_t>::value), "");

    // These tests fail on systems whose C library doesn't provide a correct overload
    // set for wcschr, wcspbrk, wcsrchr, wcsstr, and wmemchr, unless the compiler is
    // a suitably recent version of Clang.
#if !defined(__APPLE__) || defined(_LIBCPP_PREFERRED_OVERLOAD)
    static_assert((std::is_same<decltype(wcschr((const wchar_t*)0, L' ')), const wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcspbrk((const wchar_t*)0, L"")), const wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcsrchr((const wchar_t*)0, L' ')), const wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wcsstr((const wchar_t*)0, L"")), const wchar_t*>::value), "");
    static_assert((std::is_same<decltype(wmemchr((const wchar_t*)0, L' ', s)), const wchar_t*>::value), "");
#endif

#ifndef _LIBCPP_HAS_NO_STDIN
    static_assert((std::is_same<decltype(getwchar()), wint_t>::value), "");
    static_assert((std::is_same<decltype(vwscanf(L"", va)), int>::value), "");
    static_assert((std::is_same<decltype(wscanf(L"")), int>::value), "");
#endif

#ifndef _LIBCPP_HAS_NO_STDOUT
    static_assert((std::is_same<decltype(putwchar(L' ')), wint_t>::value), "");
    static_assert((std::is_same<decltype(vwprintf(L"", va)), int>::value), "");
    static_assert((std::is_same<decltype(wprintf(L"")), int>::value), "");
#endif
}
