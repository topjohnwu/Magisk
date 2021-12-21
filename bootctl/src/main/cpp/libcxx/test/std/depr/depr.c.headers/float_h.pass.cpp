//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

 // test <float.h>

#include <float.h>

#include "test_macros.h"

#ifndef FLT_ROUNDS
#error FLT_ROUNDS not defined
#endif

#ifndef FLT_EVAL_METHOD
#error FLT_EVAL_METHOD not defined
#endif

#ifndef FLT_RADIX
#error FLT_RADIX not defined
#endif

#if TEST_STD_VER > 14 && defined(TEST_HAS_C11_FEATURES) && 0
#ifndef FLT_HAS_SUBNORM
#error FLT_HAS_SUBNORM not defined
#endif

#ifndef DBL_HAS_SUBNORM
#error DBL_HAS_SUBNORM not defined
#endif

#ifndef LDBL_HAS_SUBNORM
#error LDBL_HAS_SUBNORM not defined
#endif
#endif

#ifndef FLT_MANT_DIG
#error FLT_MANT_DIG not defined
#endif

#ifndef DBL_MANT_DIG
#error DBL_MANT_DIG not defined
#endif

#ifndef LDBL_MANT_DIG
#error LDBL_MANT_DIG not defined
#endif

#ifndef DECIMAL_DIG
#error DECIMAL_DIG not defined
#endif

#if TEST_STD_VER > 14 && defined(TEST_HAS_C11_FEATURES) && 0
#ifndef FLT_DECIMAL_DIG
#error FLT_DECIMAL_DIG not defined
#endif

#ifndef DBL_DECIMAL_DIG
#error DBL_DECIMAL_DIG not defined
#endif

#ifndef LDBL_DECIMAL_DIG
#error LDBL_DECIMAL_DIG not defined
#endif
#endif

#ifndef FLT_DIG
#error FLT_DIG not defined
#endif

#ifndef DBL_DIG
#error DBL_DIG not defined
#endif

#ifndef LDBL_DIG
#error LDBL_DIG not defined
#endif

#ifndef FLT_MIN_EXP
#error FLT_MIN_EXP not defined
#endif

#ifndef DBL_MIN_EXP
#error DBL_MIN_EXP not defined
#endif

#ifndef LDBL_MIN_EXP
#error LDBL_MIN_EXP not defined
#endif

#ifndef FLT_MIN_10_EXP
#error FLT_MIN_10_EXP not defined
#endif

#ifndef DBL_MIN_10_EXP
#error DBL_MIN_10_EXP not defined
#endif

#ifndef LDBL_MIN_10_EXP
#error LDBL_MIN_10_EXP not defined
#endif

#ifndef FLT_MAX_EXP
#error FLT_MAX_EXP not defined
#endif

#ifndef DBL_MAX_EXP
#error DBL_MAX_EXP not defined
#endif

#ifndef LDBL_MAX_EXP
#error LDBL_MAX_EXP not defined
#endif

#ifndef FLT_MAX_10_EXP
#error FLT_MAX_10_EXP not defined
#endif

#ifndef DBL_MAX_10_EXP
#error DBL_MAX_10_EXP not defined
#endif

#ifndef LDBL_MAX_10_EXP
#error LDBL_MAX_10_EXP not defined
#endif

#ifndef FLT_MAX
#error FLT_MAX not defined
#endif

#ifndef DBL_MAX
#error DBL_MAX not defined
#endif

#ifndef LDBL_MAX
#error LDBL_MAX not defined
#endif

#ifndef FLT_EPSILON
#error FLT_EPSILON not defined
#endif

#ifndef DBL_EPSILON
#error DBL_EPSILON not defined
#endif

#ifndef LDBL_EPSILON
#error LDBL_EPSILON not defined
#endif

#ifndef FLT_MIN
#error FLT_MIN not defined
#endif

#ifndef DBL_MIN
#error DBL_MIN not defined
#endif

#ifndef LDBL_MIN
#error LDBL_MIN not defined
#endif

#if TEST_STD_VER > 14 && defined(TEST_HAS_C11_FEATURES) && 0
#ifndef FLT_TRUE_MIN
#error FLT_TRUE_MIN not defined
#endif

#ifndef DBL_TRUE_MIN
#error DBL_TRUE_MIN not defined
#endif

#ifndef LDBL_TRUE_MIN
#error LDBL_TRUE_MIN not defined
#endif
#endif

int main()
{
}
