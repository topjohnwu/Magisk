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
// {
// public:
//     // types
//     typedef UIntType result_type;

#include <random>
#include <type_traits>

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
        if (x_ < Min)
            x_ = Min;
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
    static_assert((std::is_same<
        std::independent_bits_engine<rand1<unsigned long, 0, 10>, 16, unsigned>::result_type,
        unsigned>::value), "");
}

void
test2()
{
    static_assert((std::is_same<
        std::independent_bits_engine<rand1<unsigned long, 0, 10>, 16, unsigned long long>::result_type,
        unsigned long long>::value), "");
}

int main()
{
    test1();
    test2();
}
