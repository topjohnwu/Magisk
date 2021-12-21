//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Define a bunch of macros that can be used in the tests instead of
//  implementation defined assumptions:
//   - locale names
//   - floating point number string output

#ifndef PLATFORM_SUPPORT_H
#define PLATFORM_SUPPORT_H

// locale names
#ifdef _WIN32
// WARNING: Windows does not support UTF-8 codepages.
// Locales are "converted" using https://docs.moodle.org/dev/Table_of_locales
#define LOCALE_en_US           "en-US"
#define LOCALE_en_US_UTF_8     "en-US"
#define LOCALE_cs_CZ_ISO8859_2 "cs-CZ"
#define LOCALE_fr_FR_UTF_8     "fr-FR"
#define LOCALE_fr_CA_ISO8859_1 "fr-CA"
#define LOCALE_ru_RU_UTF_8     "ru-RU"
#define LOCALE_zh_CN_UTF_8     "zh-CN"
#elif defined(__CloudABI__)
// Timezones are integrated into locales through LC_TIMEZONE_MASK on
// CloudABI. LC_ALL_MASK can only be used if a timezone has also been
// provided. UTC should be all right.
#define LOCALE_en_US           "en_US"
#define LOCALE_en_US_UTF_8     "en_US.UTF-8@UTC"
#define LOCALE_fr_FR_UTF_8     "fr_FR.UTF-8@UTC"
#define LOCALE_fr_CA_ISO8859_1 "fr_CA.ISO-8859-1@UTC"
#define LOCALE_cs_CZ_ISO8859_2 "cs_CZ.ISO-8859-2@UTC"
#define LOCALE_ru_RU_UTF_8     "ru_RU.UTF-8@UTC"
#define LOCALE_zh_CN_UTF_8     "zh_CN.UTF-8@UTC"
#else
#define LOCALE_en_US           "en_US"
#define LOCALE_en_US_UTF_8     "en_US.UTF-8"
#define LOCALE_fr_FR_UTF_8     "fr_FR.UTF-8"
#ifdef __linux__
#define LOCALE_fr_CA_ISO8859_1 "fr_CA.ISO-8859-1"
#define LOCALE_cs_CZ_ISO8859_2 "cs_CZ.ISO-8859-2"
#else
#define LOCALE_fr_CA_ISO8859_1 "fr_CA.ISO8859-1"
#define LOCALE_cs_CZ_ISO8859_2 "cs_CZ.ISO8859-2"
#endif
#define LOCALE_ru_RU_UTF_8     "ru_RU.UTF-8"
#define LOCALE_zh_CN_UTF_8     "zh_CN.UTF-8"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <codecvt>
#include <locale>
#include <string>
#if defined(_WIN32) || defined(__MINGW32__)
#include <io.h> // _mktemp_s
#else
#include <unistd.h> // close
#endif

#if defined(_NEWLIB_VERSION) && defined(__STRICT_ANSI__)
// Newlib provides this, but in the header it's under __STRICT_ANSI__
extern "C" {
  int mkstemp(char*);
}
#endif

#ifndef __CloudABI__
inline
std::string get_temp_file_name()
{
#if defined(__MINGW32__)
    char Path[MAX_PATH + 1];
    char FN[MAX_PATH + 1];
    do { } while (0 == GetTempPath(MAX_PATH+1, Path));
    do { } while (0 == GetTempFileName(Path, "libcxx", 0, FN));
    return FN;
#elif defined(_WIN32)
    char Name[] = "libcxx.XXXXXX";
    if (_mktemp_s(Name, sizeof(Name)) != 0) abort();
    return Name;
#else
    std::string Name;
    int FD = -1;
    do {
        Name = "libcxx.XXXXXX";
        FD = mkstemp(&Name[0]);
        if (FD == -1 && errno == EINVAL) {
            perror("mkstemp");
            abort();
        }
    } while (FD == -1);
    close(FD);
    return Name;
#endif
}

#ifdef _LIBCPP_HAS_OPEN_WITH_WCHAR
inline
std::wstring get_wide_temp_file_name()
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> >().from_bytes(
        get_temp_file_name());
}
#endif // _LIBCPP_HAS_OPEN_WITH_WCHAR

#endif // __CloudABI__

#endif // PLATFORM_SUPPORT_H
