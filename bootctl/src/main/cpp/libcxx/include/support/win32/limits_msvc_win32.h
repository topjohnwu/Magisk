// -*- C++ -*-
//===------------------ support/win32/limits_msvc_win32.h -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_SUPPORT_WIN32_LIMITS_MSVC_WIN32_H
#define _LIBCPP_SUPPORT_WIN32_LIMITS_MSVC_WIN32_H

#if !defined(_LIBCPP_MSVCRT)
#error "This header complements the Microsoft C Runtime library, and should not be included otherwise."
#endif
#if defined(__clang__)
#error "This header should only be included when using Microsofts C1XX frontend"
#endif

#include <limits.h> // CHAR_BIT
#include <float.h> // limit constants
#include <math.h> // HUGE_VAL
#include <ymath.h> // internal MSVC header providing the needed functionality

#define __CHAR_BIT__       CHAR_BIT

#define __FLT_MANT_DIG__   FLT_MANT_DIG
#define __FLT_DIG__        FLT_DIG
#define __FLT_RADIX__      FLT_RADIX
#define __FLT_MIN_EXP__    FLT_MIN_EXP
#define __FLT_MIN_10_EXP__ FLT_MIN_10_EXP
#define __FLT_MAX_EXP__    FLT_MAX_EXP
#define __FLT_MAX_10_EXP__ FLT_MAX_10_EXP
#define __FLT_MIN__        FLT_MIN
#define __FLT_MAX__        FLT_MAX
#define __FLT_EPSILON__    FLT_EPSILON
// predefined by MinGW GCC
#define __FLT_DENORM_MIN__ 1.40129846432481707092e-45F

#define __DBL_MANT_DIG__   DBL_MANT_DIG
#define __DBL_DIG__        DBL_DIG
#define __DBL_RADIX__      DBL_RADIX
#define __DBL_MIN_EXP__    DBL_MIN_EXP
#define __DBL_MIN_10_EXP__ DBL_MIN_10_EXP
#define __DBL_MAX_EXP__    DBL_MAX_EXP
#define __DBL_MAX_10_EXP__ DBL_MAX_10_EXP
#define __DBL_MIN__        DBL_MIN
#define __DBL_MAX__        DBL_MAX
#define __DBL_EPSILON__    DBL_EPSILON
// predefined by MinGW GCC
#define __DBL_DENORM_MIN__ double(4.94065645841246544177e-324L)

#define __LDBL_MANT_DIG__   LDBL_MANT_DIG
#define __LDBL_DIG__        LDBL_DIG
#define __LDBL_RADIX__      LDBL_RADIX
#define __LDBL_MIN_EXP__    LDBL_MIN_EXP
#define __LDBL_MIN_10_EXP__ LDBL_MIN_10_EXP
#define __LDBL_MAX_EXP__    LDBL_MAX_EXP
#define __LDBL_MAX_10_EXP__ LDBL_MAX_10_EXP
#define __LDBL_MIN__        LDBL_MIN
#define __LDBL_MAX__        LDBL_MAX
#define __LDBL_EPSILON__    LDBL_EPSILON
// predefined by MinGW GCC
#define __LDBL_DENORM_MIN__ 3.64519953188247460253e-4951L

// __builtin replacements/workarounds
#define __builtin_huge_vall()    _LInf._Long_double
#define __builtin_nanl(__dummmy) _LNan._Long_double
#define __builtin_nansl(__dummy) _LSnan._Long_double

#endif // _LIBCPP_SUPPORT_WIN32_LIMITS_MSVC_WIN32_H
