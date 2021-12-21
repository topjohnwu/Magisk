//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// <optional>

// constexpr const T& optional<T>::value() const &;

#include <optional>
#include <type_traits>
#include <cassert>

using std::optional;

struct X
{
    constexpr int test() const {return 3;}
    int test() {return 4;}
};

int main()
{
    {
        constexpr optional<X> opt;
        static_assert(opt.value().test() == 3, "");
    }
}
