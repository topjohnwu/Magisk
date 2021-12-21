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

// pair(pair&&) = default;

#include <utility>
#include <memory>
#include <cassert>

#include "test_macros.h"

struct Dummy {
  Dummy(Dummy const&) = delete;
  Dummy(Dummy &&) = default;
};

int main()
{
    {
        typedef std::pair<int, short> P1;
        static_assert(std::is_move_constructible<P1>::value, "");
        P1 p1(3, static_cast<short>(4));
        P1 p2 = std::move(p1);
        assert(p2.first == 3);
        assert(p2.second == 4);
    }
    {
        using P = std::pair<Dummy, int>;
        static_assert(!std::is_copy_constructible<P>::value, "");
        static_assert(std::is_move_constructible<P>::value, "");
    }
}
