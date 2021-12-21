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

// UNSUPPORTED: c++98, c++03

// This is not a portable test

#include <tuple>

struct A {};

struct B {};

int main()
{
    {
        typedef std::tuple<int, A> T;
        static_assert((sizeof(T) == sizeof(int)), "");
    }
    {
        typedef std::tuple<A, int> T;
        static_assert((sizeof(T) == sizeof(int)), "");
    }
    {
        typedef std::tuple<A, int, B> T;
        static_assert((sizeof(T) == sizeof(int)), "");
    }
    {
        typedef std::tuple<A, B, int> T;
        static_assert((sizeof(T) == sizeof(int)), "");
    }
    {
        typedef std::tuple<int, A, B> T;
        static_assert((sizeof(T) == sizeof(int)), "");
    }
}
