//===----------------------- noexception3.pass.cpp ------------------------===//
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
//      void __cxa_rethrow_primary_exception(void* thrown_object);
// }

unsigned gCounter = 0;

void my_terminate() { exit(0); }

int main ()
{
    // should not call std::terminate()
    __cxxabiv1::__cxa_rethrow_primary_exception(nullptr);

    std::set_terminate(my_terminate);

    // should call std::terminate()
    __cxxabiv1::__cxa_rethrow_primary_exception((void*) &gCounter);
    assert(false);

    return 0;
}
