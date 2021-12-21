//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, UIntType a, UIntType c, UIntType m>
// class linear_congruential_engine
// {
// public:
//     engine characteristics
//     static constexpr result_type multiplier = a;
//     static constexpr result_type increment = c;
//     static constexpr result_type modulus = m;
//     static constexpr result_type min() { return c == 0u ? 1u: 0u;}
//     static constexpr result_type max() { return m - 1u;}
//     static constexpr result_type default_seed = 1u;

#include <random>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <class T>
void where(const T &) {}

template <class T, T a, T c, T m>
void
test1()
{
    typedef std::linear_congruential_engine<T, a, c, m> LCE;
    typedef typename LCE::result_type result_type;
    static_assert((LCE::multiplier == a), "");
    static_assert((LCE::increment == c), "");
    static_assert((LCE::modulus == m), "");
#if TEST_STD_VER >= 11
    static_assert((LCE::min() == (c == 0u ? 1u: 0u)), "");
#else
    assert((LCE::min() == (c == 0u ? 1u: 0u)));
#endif

#ifdef TEST_COMPILER_C1XX
    #pragma warning(push)
    #pragma warning(disable: 4310) // cast truncates constant value
#endif // TEST_COMPILER_C1XX

#if TEST_STD_VER >= 11
    static_assert((LCE::max() == result_type(m - 1u)), "");
#else
    assert((LCE::max() == result_type(m - 1u)));
#endif

#ifdef TEST_COMPILER_C1XX
    #pragma warning(pop)
#endif // TEST_COMPILER_C1XX

    static_assert((LCE::default_seed == 1), "");
    where(LCE::multiplier);
    where(LCE::increment);
    where(LCE::modulus);
    where(LCE::default_seed);
}

template <class T>
void
test()
{
    test1<T, 0, 0, 0>();
    test1<T, 0, 1, 2>();
    test1<T, 1, 1, 2>();
    const T M(static_cast<T>(-1));
    test1<T, 0, 0, M>();
    test1<T, 0, M-2, M>();
    test1<T, 0, M-1, M>();
    test1<T, M-2, 0, M>();
    test1<T, M-2, M-2, M>();
    test1<T, M-2, M-1, M>();
    test1<T, M-1, 0, M>();
    test1<T, M-1, M-2, M>();
    test1<T, M-1, M-1, M>();
}

int main()
{
    test<unsigned short>();
    test<unsigned int>();
    test<unsigned long>();
    test<unsigned long long>();
}
