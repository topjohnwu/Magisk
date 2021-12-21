//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++98, c++03, c++11, c++14
// <numeric>

// template<class _M, class _N>
// constexpr common_type_t<_M,_N> lcm(_M __m, _N __n)

#include <numeric>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include "test_macros.h"

constexpr struct {
  int x;
  int y;
  int expect;
} Cases[] = {
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {1, 1, 1},
    {2, 3, 6},
    {2, 4, 4},
    {3, 17, 51},
    {36, 18, 36}
};

template <typename Input1, typename Input2, typename Output>
constexpr bool test0(int in1, int in2, int out)
{
    auto value1 = static_cast<Input1>(in1);
    auto value2 = static_cast<Input2>(in2);
    static_assert(std::is_same_v<Output, decltype(std::lcm(value1, value2))>, "");
    static_assert(std::is_same_v<Output, decltype(std::lcm(value2, value1))>, "");
    assert(static_cast<Output>(out) == std::lcm(value1, value2));
    return true;
}


template <typename Input1, typename Input2 = Input1>
constexpr bool do_test(int = 0)
{
    using S1 = std::make_signed_t<Input1>;
    using S2 = std::make_signed_t<Input2>;
    using U1 = std::make_unsigned_t<Input1>;
    using U2 = std::make_unsigned_t<Input2>;
    bool accumulate = true;
    for (auto TC : Cases) {
        { // Test with two signed types
            using Output = std::common_type_t<S1, S2>;
            accumulate &= test0<S1, S2, Output>(TC.x, TC.y, TC.expect);
            accumulate &= test0<S1, S2, Output>(-TC.x, TC.y, TC.expect);
            accumulate &= test0<S1, S2, Output>(TC.x, -TC.y, TC.expect);
            accumulate &= test0<S1, S2, Output>(-TC.x, -TC.y, TC.expect);
            accumulate &= test0<S2, S1, Output>(TC.x, TC.y, TC.expect);
            accumulate &= test0<S2, S1, Output>(-TC.x, TC.y, TC.expect);
            accumulate &= test0<S2, S1, Output>(TC.x, -TC.y, TC.expect);
            accumulate &= test0<S2, S1, Output>(-TC.x, -TC.y, TC.expect);
        }
        { // test with two unsigned types
            using Output = std::common_type_t<U1, U2>;
            accumulate &= test0<U1, U2, Output>(TC.x, TC.y, TC.expect);
            accumulate &= test0<U2, U1, Output>(TC.x, TC.y, TC.expect);
        }
        { // Test with mixed signs
            using Output = std::common_type_t<S1, U2>;
            accumulate &= test0<S1, U2, Output>(TC.x, TC.y, TC.expect);
            accumulate &= test0<U2, S1, Output>(TC.x, TC.y, TC.expect);
            accumulate &= test0<S1, U2, Output>(-TC.x, TC.y, TC.expect);
            accumulate &= test0<U2, S1, Output>(TC.x, -TC.y, TC.expect);
        }
        { // Test with mixed signs
            using Output = std::common_type_t<S2, U1>;
            accumulate &= test0<S2, U1, Output>(TC.x, TC.y, TC.expect);
            accumulate &= test0<U1, S2, Output>(TC.x, TC.y, TC.expect);
            accumulate &= test0<S2, U1, Output>(-TC.x, TC.y, TC.expect);
            accumulate &= test0<U1, S2, Output>(TC.x, -TC.y, TC.expect);
        }
    }
    return accumulate;
}

int main()
{
    auto non_cce = std::rand(); // a value that can't possibly be constexpr

    static_assert(do_test<signed char>(), "");
    static_assert(do_test<short>(), "");
    static_assert(do_test<int>(), "");
    static_assert(do_test<long>(), "");
    static_assert(do_test<long long>(), "");

    assert(do_test<signed char>(non_cce));
    assert(do_test<short>(non_cce));
    assert(do_test<int>(non_cce));
    assert(do_test<long>(non_cce));
    assert(do_test<long long>(non_cce));

    static_assert(do_test<std::int8_t>(), "");
    static_assert(do_test<std::int16_t>(), "");
    static_assert(do_test<std::int32_t>(), "");
    static_assert(do_test<std::int64_t>(), "");

    assert(do_test<std::int8_t>(non_cce));
    assert(do_test<std::int16_t>(non_cce));
    assert(do_test<std::int32_t>(non_cce));
    assert(do_test<std::int64_t>(non_cce));

    static_assert(do_test<signed char, int>(), "");
    static_assert(do_test<int, signed char>(), "");
    static_assert(do_test<short, int>(), "");
    static_assert(do_test<int, short>(), "");
    static_assert(do_test<int, long>(), "");
    static_assert(do_test<long, int>(), "");
    static_assert(do_test<int, long long>(), "");
    static_assert(do_test<long long, int>(), "");

    assert((do_test<signed char, int>(non_cce)));
    assert((do_test<int, signed char>(non_cce)));
    assert((do_test<short, int>(non_cce)));
    assert((do_test<int, short>(non_cce)));
    assert((do_test<int, long>(non_cce)));
    assert((do_test<long, int>(non_cce)));
    assert((do_test<int, long long>(non_cce)));
    assert((do_test<long long, int>(non_cce)));

//  LWG#2837
    {
    auto res1 = std::lcm(static_cast<std::int64_t>(1234), INT32_MIN);
    TEST_IGNORE_NODISCARD std::lcm(INT_MIN, 2UL);   // this used to trigger UBSAN
    static_assert(std::is_same_v<decltype(res1), std::int64_t>, "");
    assert(res1 == 1324997410816LL);
    }
}
