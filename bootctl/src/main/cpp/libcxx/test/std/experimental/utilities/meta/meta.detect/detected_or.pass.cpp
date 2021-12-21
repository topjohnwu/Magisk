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
  using hasFoo = typename T::Foo;

struct yesFoo {
    using Foo = int;
};

struct noFoo {
};


template <typename T, typename Res>
void test() {
    static_assert( std::is_same<Res, typename ex::detected_or  <double, hasFoo, T>::type>::value, "" );
    static_assert( std::is_same<Res, typename ex::detected_or_t<double, hasFoo, T>      >::value, "" );
}

int main () {
    test<yesFoo, int>();
    test<noFoo, double>();
}
