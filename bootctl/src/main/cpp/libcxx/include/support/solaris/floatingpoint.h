//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define atof sun_atof
#define strtod sun_strtod
#include_next "floatingpoint.h"
#undef atof
#undef strtod
