//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test forward

#include <utility>

#include "test_macros.h"

struct A
{
};

A source() {return A();}
const A csource() {return A();}

int main()
{
#if TEST_STD_VER >= 11
    {
        std::forward<A&>(source());  // expected-note {{requested here}}
        // expected-error-re@type_traits:* 1 {{static_assert failed{{.*}} "can not forward an rvalue as an lvalue"}}
    }
#else
    {
        std::forward<A&>(source()); // expected-error {{no matching function for call to 'forward'}}
    }
#endif
    {
        const A ca = A();
        std::forward<A&>(ca); // expected-error {{no matching function for call to 'forward'}}
    }
    {
        std::forward<A&>(csource());  // expected-error {{no matching function for call to 'forward'}}
    }
    {
        const A ca = A();
        std::forward<A>(ca); // expected-error {{no matching function for call to 'forward'}}
    }
    {
        std::forward<A>(csource()); // expected-error {{no matching function for call to 'forward'}}
    }
    {
        A a;
        std::forward(a); // expected-error {{no matching function for call to 'forward'}}
    }
}
