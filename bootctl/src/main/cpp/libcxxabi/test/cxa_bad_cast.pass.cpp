//===----------------------- cxa_bad_cast.pass.cpp ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

#include <cxxabi.h>
#include <cassert>
#include <stdlib.h>
#include <exception>
#include <typeinfo>

class Base {
  virtual void foo() {};
};

class Derived : public Base {};

Derived &test_bad_cast(Base& b) {
  return dynamic_cast<Derived&>(b);
}

Base gB;

void my_terminate() { exit(0); }

int main ()
{
    // swap-out the terminate handler
    void (*default_handler)() = std::get_terminate(); 
    std::set_terminate(my_terminate);

#ifndef LIBCXXABI_HAS_NO_EXCEPTIONS
    try {
#endif
        Derived &d = test_bad_cast(gB);
        assert(false);
        ((void)d);
#ifndef LIBCXXABI_HAS_NO_EXCEPTIONS
    } catch (std::bad_cast) {
        // success
        return 0;
    } catch (...) {
        assert(false);
    }
#endif

    // failure, restore the default terminate handler and fire
    std::set_terminate(default_handler);
    std::terminate();
}
