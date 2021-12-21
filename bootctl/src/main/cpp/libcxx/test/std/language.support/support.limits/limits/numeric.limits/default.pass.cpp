//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// The default numeric_limits<T> template shall have all members, but with
// 0 or false values.

#include <limits>
#include <cassert>

struct A
{
    A(int i = 0) : data_(i) {}
    int data_;
};

bool operator == (const A& x, const A& y) {return x.data_ == y.data_;}

int main()
{
    static_assert(std::numeric_limits<A>::is_specialized == false,
                 "std::numeric_limits<A>::is_specialized == false");
    assert(std::numeric_limits<A>::min() == A());
    assert(std::numeric_limits<A>::max() == A());
    assert(std::numeric_limits<A>::lowest() == A());
    static_assert(std::numeric_limits<A>::digits == 0,
                 "std::numeric_limits<A>::digits == 0");
    static_assert(std::numeric_limits<A>::digits10 == 0,
                 "std::numeric_limits<A>::digits10 == 0");
    static_assert(std::numeric_limits<A>::max_digits10 == 0,
                 "std::numeric_limits<A>::max_digits10 == 0");
    static_assert(std::numeric_limits<A>::is_signed == false,
                 "std::numeric_limits<A>::is_signed == false");
    static_assert(std::numeric_limits<A>::is_integer == false,
                 "std::numeric_limits<A>::is_integer == false");
    static_assert(std::numeric_limits<A>::is_exact == false,
                 "std::numeric_limits<A>::is_exact == false");
    static_assert(std::numeric_limits<A>::radix == 0,
                 "std::numeric_limits<A>::radix == 0");
    assert(std::numeric_limits<A>::epsilon() == A());
    assert(std::numeric_limits<A>::round_error() == A());
    static_assert(std::numeric_limits<A>::min_exponent == 0,
                 "std::numeric_limits<A>::min_exponent == 0");
    static_assert(std::numeric_limits<A>::min_exponent10 == 0,
                 "std::numeric_limits<A>::min_exponent10 == 0");
    static_assert(std::numeric_limits<A>::max_exponent == 0,
                 "std::numeric_limits<A>::max_exponent == 0");
    static_assert(std::numeric_limits<A>::max_exponent10 == 0,
                 "std::numeric_limits<A>::max_exponent10 == 0");
    static_assert(std::numeric_limits<A>::has_infinity == false,
                 "std::numeric_limits<A>::has_infinity == false");
    static_assert(std::numeric_limits<A>::has_quiet_NaN == false,
                 "std::numeric_limits<A>::has_quiet_NaN == false");
    static_assert(std::numeric_limits<A>::has_signaling_NaN == false,
                 "std::numeric_limits<A>::has_signaling_NaN == false");
    static_assert(std::numeric_limits<A>::has_denorm == std::denorm_absent,
                 "std::numeric_limits<A>::has_denorm == std::denorm_absent");
    static_assert(std::numeric_limits<A>::has_denorm_loss == false,
                 "std::numeric_limits<A>::has_denorm_loss == false");
    assert(std::numeric_limits<A>::infinity() == A());
    assert(std::numeric_limits<A>::quiet_NaN() == A());
    assert(std::numeric_limits<A>::signaling_NaN() == A());
    assert(std::numeric_limits<A>::denorm_min() == A());
    static_assert(std::numeric_limits<A>::is_iec559 == false,
                 "std::numeric_limits<A>::is_iec559 == false");
    static_assert(std::numeric_limits<A>::is_bounded == false,
                 "std::numeric_limits<A>::is_bounded == false");
    static_assert(std::numeric_limits<A>::is_modulo == false,
                 "std::numeric_limits<A>::is_modulo == false");
    static_assert(std::numeric_limits<A>::traps == false,
                 "std::numeric_limits<A>::traps == false");
    static_assert(std::numeric_limits<A>::tinyness_before == false,
                 "std::numeric_limits<A>::tinyness_before == false");
    static_assert(std::numeric_limits<A>::round_style == std::round_toward_zero,
                 "std::numeric_limits<A>::round_style == std::round_toward_zero");
}
