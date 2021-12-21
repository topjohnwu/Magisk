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
//
//     // engine characteristics
//     static constexpr result_type min() { return 0; }
//     static constexpr result_type max() { return 2^w - 1; }

#include <random>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

void
test1()
{
    typedef std::independent_bits_engine<std::ranlux24, 32, unsigned> E;
#if TEST_STD_VER >= 11
    static_assert((E::min() == 0), "");
    static_assert((E::max() == 0xFFFFFFFF), "");
#else
    assert((E::min() == 0));
    assert((E::max() == 0xFFFFFFFF));
#endif
}

void
test2()
{
    typedef std::independent_bits_engine<std::ranlux48, 64, unsigned long long> E;
#if TEST_STD_VER >= 11
    static_assert((E::min() == 0), "");
    static_assert((E::max() == 0xFFFFFFFFFFFFFFFFull), "");
#else
    assert((E::min() == 0));
    assert((E::max() == 0xFFFFFFFFFFFFFFFFull));
#endif
}

int main()
{
    test1();
    test2();
}
