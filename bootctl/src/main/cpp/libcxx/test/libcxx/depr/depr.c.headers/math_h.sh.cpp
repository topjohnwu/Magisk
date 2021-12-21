//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// RUN: %compile -fsyntax-only

#ifdef _MSC_VER

#include <math.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#error M_PI not defined
#endif

#endif
