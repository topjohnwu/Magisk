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
//   requires ShuffleIterator<Iter>
//         && LessThanComparable<Iter::value_type>
//   void
//   partial_sort(Iter first, Iter middle, Iter last);

#include <algorithm>
#include <random>
#include <cassert>

std::mt19937 randomness;

void
test_larger_sorts(int N, int M)
{
    assert(N != 0);
    assert(N >= M);
    int* array = new int[N];
    for (int i = 0; i < N; ++i)
        array[i] = i;
    std::shuffle(array, array+N, randomness);
    std::partial_sort(array, array+M, array+N);
    for (int i = 0; i < M; ++i)
    {
        assert(i < N); // quiet analysis warnings
        assert(array[i] == i);
    }
    delete [] array;
}

void
test_larger_sorts(int N)
{
    test_larger_sorts(N, 0);
    test_larger_sorts(N, 1);
    test_larger_sorts(N, 2);
    test_larger_sorts(N, 3);
    test_larger_sorts(N, N/2-1);
    test_larger_sorts(N, N/2);
    test_larger_sorts(N, N/2+1);
    test_larger_sorts(N, N-2);
    test_larger_sorts(N, N-1);
    test_larger_sorts(N, N);
}

int main()
{
    int i = 0;
    std::partial_sort(&i, &i, &i);
    assert(i == 0);
    test_larger_sorts(10);
    test_larger_sorts(256);
    test_larger_sorts(257);
    test_larger_sorts(499);
    test_larger_sorts(500);
    test_larger_sorts(997);
    test_larger_sorts(1000);
    test_larger_sorts(1009);
}
