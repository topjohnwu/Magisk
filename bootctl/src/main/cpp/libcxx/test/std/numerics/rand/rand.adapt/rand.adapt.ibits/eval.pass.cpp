//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t w, class UIntType>
// class independent_bits_engine

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
   typedef std::independent_bits_engine<rand1<unsigned, 0, 10>, 16, unsigned> E;

    E e;
    assert(e() == 6958);
}

void
test2()
{
    typedef std::independent_bits_engine<rand1<unsigned, 0, 100>, 16, unsigned> E;

    E e;
    assert(e() == 66);
}

void
test3()
{
    typedef std::independent_bits_engine<rand1<unsigned, 0, 0xFFFFFFFF>, 32, unsigned> E;

    E e(5);
    assert(e() == 5);
}

void
test4()
{
    typedef std::independent_bits_engine<rand1<unsigned, 0, 0xFFFFFFFF>, 7, unsigned> E;

    E e(129);
    assert(e() == 1);
}

void
test5()
{
    typedef std::independent_bits_engine<rand1<unsigned, 2, 3>, 1, unsigned> E;

    E e(6);
    assert(e() == 1);
}

void
test6()
{
    typedef std::independent_bits_engine<rand1<unsigned, 2, 3>, 11, unsigned> E;

    E e(6);
    assert(e() == 1365);
}

void
test7()
{
    typedef std::independent_bits_engine<rand1<unsigned, 2, 3>, 32, unsigned> E;

    E e(6);
    assert(e() == 2863311530u);
}

void
test8()
{
    typedef std::independent_bits_engine<std::mt19937, 64, unsigned long long> E;

    E e(6);
    assert(e() == 16470362623952407241ull);
}

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
}
