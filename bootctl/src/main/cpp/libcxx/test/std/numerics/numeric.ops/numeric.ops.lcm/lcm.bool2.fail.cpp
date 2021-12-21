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

// Remarks: If either M or N is not an integer type,
// or if either is (a possibly cv-qualified) bool, the program is ill-formed.

#include <numeric>


int main()
{
    std::lcm(2, true);
}
