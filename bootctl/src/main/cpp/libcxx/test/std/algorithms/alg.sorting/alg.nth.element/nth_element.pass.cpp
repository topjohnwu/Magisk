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
//   nth_element(Iter first, Iter nth, Iter last);

#include <algorithm>
#include <random>
#include <cassert>

std::mt19937 randomness;

void
test_one(int N, int M)
{
    assert(N != 0);
    assert(M < N);
    int* array = new int[N];
    for (int i = 0; i < N; ++i)
        array[i] = i;
    std::shuffle(array, array+N, randomness);
    std::nth_element(array, array+M, array+N);
    assert(array[M] == M);
    std::nth_element(array, array+N, array+N); // begin, end, end
    delete [] array;
}

void
test(int N)
{
    test_one(N, 0);
    test_one(N, 1);
    test_one(N, 2);
    test_one(N, 3);
    test_one(N, N/2-1);
    test_one(N, N/2);
    test_one(N, N/2+1);
    test_one(N, N-3);
    test_one(N, N-2);
    test_one(N, N-1);
}

int main()
{
    int d = 0;
    std::nth_element(&d, &d, &d);
    assert(d == 0);
    test(256);
    test(257);
    test(499);
    test(500);
    test(997);
    test(1000);
    test(1009);
}
