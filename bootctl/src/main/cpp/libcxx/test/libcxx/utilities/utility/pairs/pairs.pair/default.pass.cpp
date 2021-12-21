//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <utility>

// template <class T1, class T2> struct pair

// constexpr pair();

#include <utility>
#include <type_traits>


struct ThrowingDefault {
  ThrowingDefault() { }
};

struct NonThrowingDefault {
  NonThrowingDefault() noexcept { }
};

int main() {

    static_assert(!std::is_nothrow_default_constructible<std::pair<ThrowingDefault, ThrowingDefault>>::value, "");
    static_assert(!std::is_nothrow_default_constructible<std::pair<NonThrowingDefault, ThrowingDefault>>::value, "");
    static_assert(!std::is_nothrow_default_constructible<std::pair<ThrowingDefault, NonThrowingDefault>>::value, "");
    static_assert( std::is_nothrow_default_constructible<std::pair<NonThrowingDefault, NonThrowingDefault>>::value, "");
}
