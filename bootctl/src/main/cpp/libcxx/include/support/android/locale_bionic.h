// -*- C++ -*-
//===------------------- support/android/locale_bionic.h ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_SUPPORT_ANDROID_LOCALE_BIONIC_H
#define _LIBCPP_SUPPORT_ANDROID_LOCALE_BIONIC_H

#if defined(__BIONIC__)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <xlocale.h>

#ifdef __cplusplus
}
#endif

#if defined(__ANDROID__)

#include <support/xlocale/__posix_l_fallback.h>

// If we do not have this header, we are in a platform build rather than an NDK
// build, which will always be at least as new as the ToT NDK, in which case we
// don't need any of the inlines below since libc provides them.
#if __has_include(<android/ndk-version.h>)

#include <android/api-level.h>
#include <android/ndk-version.h>
// In NDK versions later than 16, locale-aware functions are provided by
// legacy_stdlib_inlines.h
#if __NDK_MAJOR__ <= 16
#if __ANDROID_API__ < 21
#include <support/xlocale/__strtonum_fallback.h>
#elif __ANDROID_API__ < 26

#if defined(__cplusplus)
extern "C" {
#endif

inline _LIBCPP_INLINE_VISIBILITY float strtof_l(const char* __nptr, char** __endptr,
                                                locale_t) {
  return ::strtof(__nptr, __endptr);
}

inline _LIBCPP_INLINE_VISIBILITY double strtod_l(const char* __nptr,
                                                 char** __endptr, locale_t) {
  return ::strtod(__nptr, __endptr);
}

inline _LIBCPP_INLINE_VISIBILITY long strtol_l(const char* __nptr, char** __endptr,
                                               int __base, locale_t) {
  return ::strtol(__nptr, __endptr, __base);
}

#if defined(__cplusplus)
}
#endif

#endif // __ANDROID_API__ < 26

#endif // __NDK_MAJOR__ <= 16
#endif // __has_include(<android/ndk-version.h>)
#endif // defined(__ANDROID__)

#endif // defined(__BIONIC__)
#endif // _LIBCPP_SUPPORT_ANDROID_LOCALE_BIONIC_H
