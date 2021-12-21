//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, size_t w, size_t n, size_t m, size_t r,
//           UIntType a, size_t u, UIntType d, size_t s,
//           UIntType b, size_t t, UIntType c, size_t l, UIntType f>
// class mersenne_twister_engine
// {
// public:
//     // types
//     typedef UIntType result_type;
//
//     // engine characteristics
//     static constexpr size_t word_size = w;
//     static constexpr size_t state_size = n;
//     static constexpr size_t shift_size = m;
//     static constexpr size_t mask_bits = r;
//     static constexpr result_type xor_mask = a;
//     static constexpr size_t tempering_u = u;
//     static constexpr result_type tempering_d = d;
//     static constexpr size_t tempering_s = s;
//     static constexpr result_type tempering_b = b;
//     static constexpr size_t tempering_t = t;
//     static constexpr result_type tempering_c = c;
//     static constexpr size_t tempering_l = l;
//     static constexpr result_type initialization_multiplier = f;
//     static constexpr result_type min () { return 0; }
//     static constexpr result_type max() { return 2^w - 1; }
//     static constexpr result_type default_seed = 5489u;

#include <random>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <class T>
void where(const T &) {}

void
test1()
{
    typedef std::mt19937 E;
    static_assert((E::word_size == 32), "");
    static_assert((E::state_size == 624), "");
    static_assert((E::shift_size == 397), "");
    static_assert((E::mask_bits == 31), "");
    static_assert((E::xor_mask == 0x9908b0df), "");
    static_assert((E::tempering_u == 11), "");
    static_assert((E::tempering_d == 0xffffffff), "");
    static_assert((E::tempering_s == 7), "");
    static_assert((E::tempering_b == 0x9d2c5680), "");
    static_assert((E::tempering_t == 15), "");
    static_assert((E::tempering_c == 0xefc60000), "");
    static_assert((E::tempering_l == 18), "");
    static_assert((E::initialization_multiplier == 1812433253), "");
#if TEST_STD_VER >= 11
    static_assert((E::min() == 0), "");
    static_assert((E::max() == 0xFFFFFFFF), "");
#else
    assert((E::min() == 0));
    assert((E::max() == 0xFFFFFFFF));
#endif
    static_assert((E::default_seed == 5489u), "");
    where(E::word_size);
    where(E::state_size);
    where(E::shift_size);
    where(E::mask_bits);
    where(E::xor_mask);
    where(E::tempering_u);
    where(E::tempering_d);
    where(E::tempering_s);
    where(E::tempering_b);
    where(E::tempering_t);
    where(E::tempering_c);
    where(E::tempering_l);
    where(E::initialization_multiplier);
    where(E::default_seed);
}

void
test2()
{
    typedef std::mt19937_64 E;
    static_assert((E::word_size == 64), "");
    static_assert((E::state_size == 312), "");
    static_assert((E::shift_size == 156), "");
    static_assert((E::mask_bits == 31), "");
    static_assert((E::xor_mask == 0xb5026f5aa96619e9ull), "");
    static_assert((E::tempering_u == 29), "");
    static_assert((E::tempering_d == 0x5555555555555555ull), "");
    static_assert((E::tempering_s == 17), "");
    static_assert((E::tempering_b == 0x71d67fffeda60000ull), "");
    static_assert((E::tempering_t == 37), "");
    static_assert((E::tempering_c == 0xfff7eee000000000ull), "");
    static_assert((E::tempering_l == 43), "");
    static_assert((E::initialization_multiplier == 6364136223846793005ull), "");
#if TEST_STD_VER >= 11
    static_assert((E::min() == 0), "");
    static_assert((E::max() == 0xFFFFFFFFFFFFFFFFull), "");
#else
    assert((E::min() == 0));
    assert((E::max() == 0xFFFFFFFFFFFFFFFFull));
#endif
    static_assert((E::default_seed == 5489u), "");
    where(E::word_size);
    where(E::state_size);
    where(E::shift_size);
    where(E::mask_bits);
    where(E::xor_mask);
    where(E::tempering_u);
    where(E::tempering_d);
    where(E::tempering_s);
    where(E::tempering_b);
    where(E::tempering_t);
    where(E::tempering_c);
    where(E::tempering_l);
    where(E::initialization_multiplier);
    where(E::default_seed);
}

int main()
{
    test1();
    test2();
}
