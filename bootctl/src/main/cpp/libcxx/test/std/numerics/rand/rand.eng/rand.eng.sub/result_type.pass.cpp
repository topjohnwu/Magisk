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

#include <random>
#include <type_traits>

void
test1()
{
    static_assert((std::is_same<
        std::ranlux24_base::result_type,
        std::uint_fast32_t>::value), "");
}

void
test2()
{
    static_assert((std::is_same<
        std::ranlux48_base::result_type,
        std::uint_fast64_t>::value), "");
}

int main()
{
    test1();
    test2();
}
