//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <filesystem>

// #define __cpp_lib_filesystem 201703L

#include <filesystem>
#include "test_macros.h"

#if TEST_STD_VER >= 17
#ifndef __cpp_lib_filesystem
#error Filesystem feature test macro is not defined  (__cpp_lib_filesystem)
#elif __cpp_lib_filesystem != 201703L
#error Filesystem feature test macro has an incorrect value (__cpp_lib_filesystem)
#endif
#else // TEST_STD_VER < 17
#ifdef __cpp_lib_filesystem
#error Filesystem feature test macro should not be defined before C++17
#endif
#endif

int main() { }
