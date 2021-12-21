//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <class... Types>
//   void swap(tuple<Types...>& x, tuple<Types...>& y);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <cassert>

#include "MoveOnly.h"

int main()
{
    {
        typedef std::tuple<> T;
        T t0;
        T t1;
        swap(t0, t1);
    }
    {
        typedef std::tuple<MoveOnly> T;
        T t0(MoveOnly(0));
        T t1(MoveOnly(1));
        swap(t0, t1);
        assert(std::get<0>(t0) == 1);
        assert(std::get<0>(t1) == 0);
    }
    {
        typedef std::tuple<MoveOnly, MoveOnly> T;
        T t0(MoveOnly(0), MoveOnly(1));
        T t1(MoveOnly(2), MoveOnly(3));
        swap(t0, t1);
        assert(std::get<0>(t0) == 2);
        assert(std::get<1>(t0) == 3);
        assert(std::get<0>(t1) == 0);
        assert(std::get<1>(t1) == 1);
    }
    {
        typedef std::tuple<MoveOnly, MoveOnly, MoveOnly> T;
        T t0(MoveOnly(0), MoveOnly(1), MoveOnly(2));
        T t1(MoveOnly(3), MoveOnly(4), MoveOnly(5));
        swap(t0, t1);
        assert(std::get<0>(t0) == 3);
        assert(std::get<1>(t0) == 4);
        assert(std::get<2>(t0) == 5);
        assert(std::get<0>(t1) == 0);
        assert(std::get<1>(t1) == 1);
        assert(std::get<2>(t1) == 2);
    }
}
