//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// REQUIRES: long_tests
// UNSUPPORTED: c++98, c++03

// Not a portable test

// <__hash_table>

// size_t __next_hash_pow2(size_t n);

// If n <= 1, return n. If n is a power of 2, return n.
// Otherwise, return the next power of 2.

#include <__hash_table>
#include <unordered_map>
#include <cassert>

#include <iostream>

bool
is_power_of_two(unsigned long n)
{
    return __builtin_popcount(n) == 1;
}

void test_next_pow2_val(size_t n)
{
        std::size_t npow2 = std::__next_hash_pow2(n);
        assert(is_power_of_two(npow2) && npow2 > n);
}

void
test_next_pow2()
{
    assert(!is_power_of_two(0));
    assert(is_power_of_two(1));
    assert(is_power_of_two(2));
    assert(!is_power_of_two(3));

    assert(std::__next_hash_pow2(0) == 0);
    assert(std::__next_hash_pow2(1) == 1);

    for (std::size_t n = 2; n < (sizeof(std::size_t) * 8 - 1); ++n)
    {
        std::size_t pow2 = 1ULL << n;
        assert(std::__next_hash_pow2(pow2) == pow2);
    }

    test_next_pow2_val(3);
    test_next_pow2_val(7);
    test_next_pow2_val(9);
    test_next_pow2_val(15);
    test_next_pow2_val(127);
    test_next_pow2_val(129);
}

// Note: this is only really useful when run with -fsanitize=undefined.
void
fuzz_unordered_map_reserve(unsigned num_inserts,
                           unsigned num_reserve1,
                           unsigned num_reserve2)
{
    std::unordered_map<uint64_t, unsigned long> m;
    m.reserve(num_reserve1);
    for (unsigned I = 0; I < num_inserts; ++I) m[I] = 0;
    m.reserve(num_reserve2);
    assert(m.bucket_count() >= num_reserve2);
}

int main()
{
    test_next_pow2();

    for (unsigned num_inserts = 0; num_inserts <= 64; ++num_inserts)
        for (unsigned num_reserve1 = 1; num_reserve1 <= 64; ++num_reserve1)
            for (unsigned num_reserve2 = 1; num_reserve2 <= 64; ++num_reserve2)
                fuzz_unordered_map_reserve(num_inserts, num_reserve1, num_reserve2);

    return 0;
}
