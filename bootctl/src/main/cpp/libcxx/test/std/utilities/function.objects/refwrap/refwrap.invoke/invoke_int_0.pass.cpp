//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// reference_wrapper

// template <class... ArgTypes>
//   requires Callable<T, ArgTypes&&...>
//   Callable<T, ArgTypes&&...>::result_type
//   operator()(ArgTypes&&... args) const;

#include <functional>
#include <cassert>

// 0 args, return int

int count = 0;

int f_int_0()
{
    return 3;
}

struct A_int_0
{
    int operator()() {return 4;}
};

void
test_int_0()
{
    // function
    {
    std::reference_wrapper<int ()> r1(f_int_0);
    assert(r1() == 3);
    }
    // function pointer
    {
    int (*fp)() = f_int_0;
    std::reference_wrapper<int (*)()> r1(fp);
    assert(r1() == 3);
    }
    // functor
    {
    A_int_0 a0;
    std::reference_wrapper<A_int_0> r1(a0);
    assert(r1() == 4);
    }
}

// 1 arg, return void

void f_void_1(int i)
{
    count += i;
}

struct A_void_1
{
    void operator()(int i)
    {
        count += i;
    }
};

int main()
{
    test_int_0();
}
