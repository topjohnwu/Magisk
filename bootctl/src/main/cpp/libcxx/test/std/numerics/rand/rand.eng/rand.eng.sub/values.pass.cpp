//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class UIntType, size_t w, size_t s, size_t r>
// class subtract_with_carry_engine
// {
// public:
//     // types
//     typedef UIntType result_type;
//
//     // engine characteristics
//     static constexpr size_t word_size = w;
//     static constexpr size_t short_lag = s;
//     static constexpr size_t long_lag = r;
//     static constexpr result_type min() { return 0; }
//     static constexpr result_type max() { return m-1; }
//     static constexpr result_type default_seed = 19780503u;

#include <random>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <class T>
void where(const T &) {}

void
test1()
{
    typedef std::ranlux24_base E;
    static_assert((E::word_size == 24), "");
    static_assert((E::short_lag == 10), "");
    static_assert((E::long_lag == 24), "");
#if TEST_STD_VER >= 11
    static_assert((E::min() == 0), "");
    static_assert((E::max() == 0xFFFFFF), "");
#else
    assert((E::min() == 0));
    assert((E::max() == 0xFFFFFF));
#endif
    static_assert((E::default_seed == 19780503u), "");
    where(E::word_size);
    where(E::short_lag);
    where(E::long_lag);
    where(E::default_seed);
}

void
test2()
{
    typedef std::ranlux48_base E;
    static_assert((E::word_size == 48), "");
    static_assert((E::short_lag == 5), "");
    static_assert((E::long_lag == 12), "");
#if TEST_STD_VER >= 11
    static_assert((E::min() == 0), "");
    static_assert((E::max() == 0xFFFFFFFFFFFFull), "");
#else
    assert((E::min() == 0));
    assert((E::max() == 0xFFFFFFFFFFFFull));
#endif
    static_assert((E::default_seed == 19780503u), "");
    where(E::word_size);
    where(E::short_lag);
    where(E::long_lag);
    where(E::default_seed);
}

int main()
{
    test1();
    test2();
}
