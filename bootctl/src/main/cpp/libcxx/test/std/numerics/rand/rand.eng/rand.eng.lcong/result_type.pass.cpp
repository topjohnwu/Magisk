//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, UIntType a, UIntType c, UIntType m>
// class linear_congruential_engine
// {
// public:
//     // types
//     typedef UIntType result_type;

#include <random>
#include <type_traits>

template <class T>
void
test()
{
    static_assert((std::is_same<
        typename std::linear_congruential_engine<T, 0, 0, 0>::result_type,
        T>::value), "");
}

int main()
{
    test<unsigned short>();
    test<unsigned int>();
    test<unsigned long>();
    test<unsigned long long>();
}
