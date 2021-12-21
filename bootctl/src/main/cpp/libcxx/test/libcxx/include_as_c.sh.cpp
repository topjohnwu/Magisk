// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that the C wrapper headers can be included when compiling them as C.

// NOTE: It's not common or recommended to have libc++ in the header search
// path when compiling C files, but it does happen often enough.

// RUN: %cxx -c -xc %s -fsyntax-only %flags %compile_flags -std=c99

#include <complex.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <wchar.h>
#include <wctype.h>

int main() {}
