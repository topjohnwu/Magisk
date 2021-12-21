//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t p, size_t r>
// class discard_block_engine
// {
// public:
//     // types
//     typedef typename Engine::result_type result_type;

#include <random>
#include <type_traits>

void
test1()
{
    static_assert((std::is_same<
        std::ranlux24::result_type,
        std::uint_fast32_t>::value), "");
}

void
test2()
{
    static_assert((std::is_same<
        std::ranlux48::result_type,
        std::uint_fast64_t>::value), "");
}

int main()
{
    test1();
    test2();
}
