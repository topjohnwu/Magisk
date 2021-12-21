//===----------------------- noexception4.pass.cpp ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: libcxxabi-no-exceptions

#include <cxxabi.h>
#include <exception>
#include <cassert>

// namespace __cxxabiv1 {
//      void *__cxa_current_primary_exception() throw();
//      extern bool          __cxa_uncaught_exception () throw();
//      extern unsigned int  __cxa_uncaught_exceptions() throw();
// }

int main ()
{
    // Trivially
    assert(nullptr == __cxxabiv1::__cxa_current_primary_exception());
    assert(!__cxxabiv1::__cxa_uncaught_exception());
    assert(0 == __cxxabiv1::__cxa_uncaught_exceptions());
    return 0;
}
