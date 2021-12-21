//===------------------- uncaught_exceptions.pass.cpp ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <cxxabi.h>
#include <exception>
#include <cassert>

// namespace __cxxabiv1 {
//      extern bool          __cxa_uncaught_exception () throw();
//      extern unsigned int  __cxa_uncaught_exceptions() throw();
// }

struct A {
    ~A() { assert( __cxxabiv1::__cxa_uncaught_exception()); }
    };

struct B {
    B(unsigned cnt) : data_(cnt) {}
    ~B() { assert( data_ == __cxxabiv1::__cxa_uncaught_exceptions()); }
    unsigned data_;
    };

int main ()
{
    try { A a; throw 3; assert (false); }
    catch (int) {}
    
    try { B b(1); throw 3; assert (false); }
    catch (int) {}
}
