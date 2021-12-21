//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <experimental/type_traits>

#include <experimental/type_traits>
#include <string>

#include "test_macros.h"

namespace ex = std::experimental;

template <typename T>
  using callFoo = decltype(std::declval<T&>().Foo());

struct yesFoo {
    int Foo() { return 0; }
};

struct noFoo {
};

struct wrongFoo {
    std::string Foo() { return ""; }
};

struct convertibleFoo {
    long Foo() { return 0; }
};


template <typename T, typename Res>
void test() {
    static_assert( std::is_same<Res, typename ex::detected_t<callFoo, T>>::value, "" );
}

int main () {
    test<yesFoo, int>();
    test<noFoo, ex::nonesuch>();  // lookup failure returns nonesuch
    test<wrongFoo, std::string>();
}
