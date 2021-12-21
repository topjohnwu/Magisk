//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<RandomAccessIterator Iter>
//   requires ShuffleIterator<Iter> && LessThanComparable<Iter::value_type>
//   void
//   pop_heap(Iter first, Iter last);

#include <algorithm>
#include <random>
#include <cassert>

std::mt19937 randomness;

void test(int N)
{
    int* ia = new int [N];
    for (int i = 0; i < N; ++i)
        ia[i] = i;
    std::shuffle(ia, ia+N, randomness);
    std::make_heap(ia, ia+N);
    for (int i = N; i > 0; --i)
    {
        std::pop_heap(ia, ia+i);
        assert(std::is_heap(ia, ia+i-1));
    }
    std::pop_heap(ia, ia);
    delete [] ia;
}

int main()
{
    test(1000);
}
