//===------------------------- dynamic_cast_stress.cpp --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

#include <cassert>
#include <tuple>
#include "support/timer.hpp"

template <std::size_t Indx, std::size_t Depth>
struct C
    : public virtual C<Indx, Depth-1>,
      public virtual C<Indx+1, Depth-1>
{
    virtual ~C() {}
};

template <std::size_t Indx>
struct C<Indx, 0>
{
    virtual ~C() {}
};

template <std::size_t Indx, std::size_t Depth>
struct B
    : public virtual C<Indx, Depth-1>,
      public virtual C<Indx+1, Depth-1>
{
};

template <class Indx, std::size_t Depth>
struct makeB;

template <std::size_t ...Indx, std::size_t Depth>
struct makeB<std::__tuple_indices<Indx...>, Depth>
    : public B<Indx, Depth>...
{
};

template <std::size_t Width, std::size_t Depth>
struct A
    : public makeB<typename std::__make_tuple_indices<Width>::type, Depth>
{
};

void test()
{
    const std::size_t Width = 10;
    const std::size_t Depth = 5;
    A<Width, Depth> a;
    typedef B<Width/2, Depth> Destination;
//    typedef A<Width, Depth> Destination;
    Destination *b = nullptr;
    {
        timer t;
        b = dynamic_cast<Destination*>((C<Width/2, 0>*)&a);
    }
    assert(b != 0);
}

int main()
{
    test();
}

/*
Timing results I'm seeing (median of 3 microseconds):

                          libc++abi    gcc's dynamic_cast
B<Width/2, Depth> -O3      48.334         93.190           libc++abi 93% faster
B<Width/2, Depth> -Os      58.535         94.103           libc++abi 61% faster
A<Width, Depth>   -O3      11.515         33.134           libc++abi 188% faster
A<Width, Depth>   -Os      12.631         31.553           libc++abi 150% faster

*/
