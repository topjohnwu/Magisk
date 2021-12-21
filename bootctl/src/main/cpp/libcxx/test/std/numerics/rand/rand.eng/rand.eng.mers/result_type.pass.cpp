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

#include <random>
#include <type_traits>

void
test1()
{
    static_assert((std::is_same<
        std::mt19937::result_type,
        std::uint_fast32_t>::value), "");
}

void
test2()
{
    static_assert((std::is_same<
        std::mt19937_64::result_type,
        std::uint_fast64_t>::value), "");
}

int main()
{
    test1();
    test2();
}
