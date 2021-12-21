//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t k>
// class shuffle_order_engine

// result_type operator()();

#include <random>
#include <cassert>

#include "test_macros.h"

template <class UIntType, UIntType Min, UIntType Max>
class rand1
{
public:
    // types
    typedef UIntType result_type;

private:
    result_type x_;

    static_assert(Min < Max, "rand1 invalid parameters");
public:

#if TEST_STD_VER < 11 && defined(_LIBCPP_VERSION)
    // Workaround for lack of constexpr in C++03
    static const result_type _Min = Min;
    static const result_type _Max = Max;
#endif

    static TEST_CONSTEXPR result_type min() {return Min;}
    static TEST_CONSTEXPR result_type max() {return Max;}

    explicit rand1(result_type sd = Min) : x_(sd)
    {
        if (x_ > Max)
            x_ = Max;
    }

    result_type operator()()
    {
        result_type r = x_;
        if (x_ < Max)
            ++x_;
        else
            x_ = Min;
        return r;
    }
};

void
test1()
{
   typedef std::knuth_b E;

    E e;
    assert(e() == 152607844u);
}

void
test2()
{
    typedef rand1<unsigned long long, 0, 0xFFFFFFFFFFFFFFFFull> E0;
    typedef std::shuffle_order_engine<E0, 101> E;
    E e;
    e.discard(400);
    assert(e() == 501);
}

void
test3()
{
    typedef rand1<unsigned long long, 0, 0xFFFFFFFFFFFFFFFFull> E0;
    typedef std::shuffle_order_engine<E0, 100> E;
    E e;
    e.discard(400);
    assert(e() == 500);
}

int main()
{
    test1();
    test2();
    test3();
}
