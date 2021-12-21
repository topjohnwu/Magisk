//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <atomic>

// Test that including <atomic> fails to compile when _LIBCPP_HAS_NO_THREADS
// is defined.

// MODULES_DEFINES: _LIBCPP_HAS_NO_THREADS
#ifndef _LIBCPP_HAS_NO_THREADS
#define _LIBCPP_HAS_NO_THREADS
#endif

#include <atomic>

int main()
{
}
