//===----------------------- noexception2.pass.cpp ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03
// REQUIRES: libcxxabi-no-exceptions

#include <cxxabi.h>
#include <exception>
#include <cassert>
#include <stdlib.h>

// namespace __cxxabiv1 {
//      void __cxa_decrement_exception_refcount(void *thrown_object) throw();
// }

unsigned gCounter = 0;

void my_terminate() { exit(0); }

int main ()
{
    // should not call std::terminate()
    __cxxabiv1::__cxa_decrement_exception_refcount(nullptr);

    std::set_terminate(my_terminate);

    // should call std::terminate()
    __cxxabiv1::__cxa_decrement_exception_refcount((void*) &gCounter);
    assert(false);

    return 0;
}
