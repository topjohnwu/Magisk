//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <cwchar>

#include <cwchar>
#include <ctime>
#include <cstdarg>
#include <type_traits>

#include "test_macros.h"

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
    std::mbstate_t mb = {};
    std::size_t s = 0;
    std::tm *tm = 0;
    std::wint_t w = 0;
    ::FILE* fp = 0;
    std::va_list va;

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

    ASSERT_SAME_TYPE(int,                decltype(std::fwprintf(fp, L"")));
    ASSERT_SAME_TYPE(int,                decltype(std::fwscanf(fp, L"")));
    ASSERT_SAME_TYPE(int,                decltype(std::swprintf(ws, s, L"")));
    ASSERT_SAME_TYPE(int,                decltype(std::swscanf(L"", L"")));
    ASSERT_SAME_TYPE(int,                decltype(std::vfwprintf(fp, L"", va)));
    ASSERT_SAME_TYPE(int,                decltype(std::vfwscanf(fp, L"", va)));
    ASSERT_SAME_TYPE(int,                decltype(std::vswprintf(ws, s, L"", va)));
    ASSERT_SAME_TYPE(int,                decltype(std::vswscanf(L"", L"", va)));
    ASSERT_SAME_TYPE(std::wint_t,        decltype(std::fgetwc(fp)));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::fgetws(ws, 0, fp)));
    ASSERT_SAME_TYPE(std::wint_t,        decltype(std::fputwc(L' ', fp)));
    ASSERT_SAME_TYPE(int,                decltype(std::fputws(L"", fp)));
    ASSERT_SAME_TYPE(int,                decltype(std::fwide(fp, 0)));
    ASSERT_SAME_TYPE(std::wint_t,        decltype(std::getwc(fp)));
    ASSERT_SAME_TYPE(std::wint_t,        decltype(std::putwc(L' ', fp)));
    ASSERT_SAME_TYPE(std::wint_t,        decltype(std::ungetwc(L' ', fp)));
    ASSERT_SAME_TYPE(double,             decltype(std::wcstod(L"", (wchar_t**)0)));
    ASSERT_SAME_TYPE(float,              decltype(std::wcstof(L"", (wchar_t**)0)));
    ASSERT_SAME_TYPE(long double,        decltype(std::wcstold(L"", (wchar_t**)0)));
    ASSERT_SAME_TYPE(long,               decltype(std::wcstol(L"", (wchar_t**)0, 0)));
    ASSERT_SAME_TYPE(long long,          decltype(std::wcstoll(L"", (wchar_t**)0, 0)));
    ASSERT_SAME_TYPE(unsigned long,      decltype(std::wcstoul(L"", (wchar_t**)0, 0)));
    ASSERT_SAME_TYPE(unsigned long long, decltype(std::wcstoull(L"", (wchar_t**)0, 0)));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcscpy(ws, L"")));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcsncpy(ws, L"", s)));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcscat(ws, L"")));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcsncat(ws, L"", s)));
    ASSERT_SAME_TYPE(int,                decltype(std::wcscmp(L"", L"")));
    ASSERT_SAME_TYPE(int,                decltype(std::wcscoll(L"", L"")));
    ASSERT_SAME_TYPE(int,                decltype(std::wcsncmp(L"", L"", s)));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::wcsxfrm(ws, L"", s)));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcschr((wchar_t*)0, L' ')));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::wcscspn(L"", L"")));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::wcslen(L"")));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcspbrk((wchar_t*)0, L"")));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcsrchr((wchar_t*)0, L' ')));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::wcsspn(L"", L"")));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcsstr((wchar_t*)0, L"")));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wcstok(ws, L"", (wchar_t**)0)));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wmemchr((wchar_t*)0, L' ', s)));
    ASSERT_SAME_TYPE(int,                decltype(std::wmemcmp(L"", L"", s)));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wmemcpy(ws, L"", s)));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wmemmove(ws, L"", s)));
    ASSERT_SAME_TYPE(wchar_t*,           decltype(std::wmemset(ws, L' ', s)));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::wcsftime(ws, s, L"", tm)));
    ASSERT_SAME_TYPE(wint_t,             decltype(std::btowc(0)));
    ASSERT_SAME_TYPE(int,                decltype(std::wctob(w)));
    ASSERT_SAME_TYPE(int,                decltype(std::mbsinit(&mb)));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::mbrlen("", s, &mb)));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::mbrtowc(ws, "", s, &mb)));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::wcrtomb(ns, L' ', &mb)));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::mbsrtowcs(ws, (const char**)0, s, &mb)));
    ASSERT_SAME_TYPE(std::size_t,        decltype(std::wcsrtombs(ns, (const wchar_t**)0, s, &mb)));

    // These tests fail on systems whose C library doesn't provide a correct overload
    // set for wcschr, wcspbrk, wcsrchr, wcsstr, and wmemchr, unless the compiler is
    // a suitably recent version of Clang.
#if !defined(__APPLE__) || defined(_LIBCPP_PREFERRED_OVERLOAD)
    ASSERT_SAME_TYPE(const wchar_t*,     decltype(std::wcschr((const wchar_t*)0, L' ')));
    ASSERT_SAME_TYPE(const wchar_t*,     decltype(std::wcspbrk((const wchar_t*)0, L"")));
    ASSERT_SAME_TYPE(const wchar_t*,     decltype(std::wcsrchr((const wchar_t*)0, L' ')));
    ASSERT_SAME_TYPE(const wchar_t*,     decltype(std::wcsstr((const wchar_t*)0, L"")));
    ASSERT_SAME_TYPE(const wchar_t*,     decltype(std::wmemchr((const wchar_t*)0, L' ', s)));
#endif

#ifndef _LIBCPP_HAS_NO_STDIN
    ASSERT_SAME_TYPE(std::wint_t,        decltype(std::getwchar()));
    ASSERT_SAME_TYPE(int,                decltype(std::vwscanf(L"", va)));
    ASSERT_SAME_TYPE(int,                decltype(std::wscanf(L"")));
#endif

#ifndef _LIBCPP_HAS_NO_STDOUT
    ASSERT_SAME_TYPE(std::wint_t,        decltype(std::putwchar(L' ')));
    ASSERT_SAME_TYPE(int,                decltype(std::vwprintf(L"", va)));
    ASSERT_SAME_TYPE(int,                decltype(std::wprintf(L"")));
#endif
}
