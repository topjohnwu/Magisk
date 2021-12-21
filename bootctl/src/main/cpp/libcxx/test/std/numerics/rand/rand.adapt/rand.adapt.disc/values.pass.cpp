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
//
//     // engine characteristics
//     static constexpr size_t block_size = p;
//     static constexpr size_t used_block = r;
//     static constexpr result_type min() { return Engine::min(); }
//     static constexpr result_type max() { return Engine::max(); }

#include <random>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <class T>
void where(const T &) {}

void
test1()
{
    typedef std::ranlux24 E;
    static_assert((E::block_size == 223), "");
    static_assert((E::used_block == 23), "");
#if TEST_STD_VER >= 11
    static_assert((E::min() == 0), "");
    static_assert((E::max() == 0xFFFFFF), "");
#else
    assert((E::min() == 0));
    assert((E::max() == 0xFFFFFF));
#endif
    where(E::block_size);
    where(E::used_block);
}

void
test2()
{
    typedef std::ranlux48 E;
    static_assert((E::block_size == 389), "");
    static_assert((E::used_block == 11), "");
#if TEST_STD_VER >= 11
    static_assert((E::min() == 0), "");
    static_assert((E::max() == 0xFFFFFFFFFFFFull), "");
#else
    assert((E::min() == 0));
    assert((E::max() == 0xFFFFFFFFFFFFull));
#endif
    where(E::block_size);
    where(E::used_block);
}

int main()
{
    test1();
    test2();
}
