//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// float_round_style

#include <limits>

typedef char one;
struct two {one _[2];};

one test(std::float_round_style);
two test(int);

int main()
{
    static_assert(std::round_indeterminate == -1,
                 "std::round_indeterminate == -1");
    static_assert(std::round_toward_zero == 0,
                 "std::round_toward_zero == 0");
    static_assert(std::round_to_nearest == 1,
                 "std::round_to_nearest == 1");
    static_assert(std::round_toward_infinity == 2,
                 "std::round_toward_infinity == 2");
    static_assert(std::round_toward_neg_infinity == 3,
                 "std::round_toward_neg_infinity == 3");
    static_assert(sizeof(test(std::round_to_nearest)) == 1,
                 "sizeof(test(std::round_to_nearest)) == 1");
    static_assert(sizeof(test(1)) == 2,
                 "sizeof(test(1)) == 2");
}
