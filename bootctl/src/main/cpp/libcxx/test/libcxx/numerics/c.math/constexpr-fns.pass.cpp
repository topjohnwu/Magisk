//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Check that the overloads of std::__libcpp_{isnan,isinf,isfinite} that take
// floating-point values are evaluatable from constexpr contexts.
//
// These functions need to be constexpr in order to be called from CUDA, see
// https://reviews.llvm.org/D25403.  They don't actually need to be
// constexpr-evaluatable, but that's what we check here, since we can't check
// true constexpr-ness.
//
// This fails with gcc because __builtin_isnan and friends, which libcpp_isnan
// and friends call, are not themselves constexpr-evaluatable.
//
// UNSUPPORTED: c++98, c++03
// XFAIL: gcc

#include <cmath>

static_assert(std::__libcpp_isnan_or_builtin(0.) == false, "");
static_assert(std::__libcpp_isinf_or_builtin(0.0) == false, "");
static_assert(std::__libcpp_isfinite_or_builtin(0.0) == true, "");

int main()
{
}
