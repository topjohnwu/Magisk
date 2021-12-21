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
//   sort(Iter first, Iter last);

#include <algorithm>
#include <iterator>
#include <random>
#include <cassert>

std::mt19937 randomness;

template <class RI>
void
test_sort_helper(RI f, RI l)
{
    typedef typename std::iterator_traits<RI>::value_type value_type;
    typedef typename std::iterator_traits<RI>::difference_type difference_type;

    if (f != l)
    {
        difference_type len = l - f;
        value_type* save(new value_type[len]);
        do
        {
            std::copy(f, l, save);
            std::sort(save, save+len);
            assert(std::is_sorted(save, save+len));
        } while (std::next_permutation(f, l));
        delete [] save;
    }
}

template <class RI>
void
test_sort_driver_driver(RI f, RI l, int start, RI real_last)
{
    for (RI i = l; i > f + start;)
    {
        *--i = start;
        if (f == i)
        {
            test_sort_helper(f, real_last);
        }
    if (start > 0)
        test_sort_driver_driver(f, i, start-1, real_last);
    }
}

template <class RI>
void
test_sort_driver(RI f, RI l, int start)
{
    test_sort_driver_driver(f, l, start, l);
}

template <int sa>
void
test_sort_()
{
    int ia[sa];
    for (int i = 0; i < sa; ++i)
    {
        test_sort_driver(ia, ia+sa, i);
    }
}

void
test_larger_sorts(int N, int M)
{
    assert(N != 0);
    assert(M != 0);
    // create array length N filled with M different numbers
    int* array = new int[N];
    int x = 0;
    for (int i = 0; i < N; ++i)
    {
        array[i] = x;
        if (++x == M)
            x = 0;
    }
    // test saw tooth pattern
    std::sort(array, array+N);
    assert(std::is_sorted(array, array+N));
    // test random pattern
    std::shuffle(array, array+N, randomness);
    std::sort(array, array+N);
    assert(std::is_sorted(array, array+N));
    // test sorted pattern
    std::sort(array, array+N);
    assert(std::is_sorted(array, array+N));
    // test reverse sorted pattern
    std::reverse(array, array+N);
    std::sort(array, array+N);
    assert(std::is_sorted(array, array+N));
    // test swap ranges 2 pattern
    std::swap_ranges(array, array+N/2, array+N/2);
    std::sort(array, array+N);
    assert(std::is_sorted(array, array+N));
    // test reverse swap ranges 2 pattern
    std::reverse(array, array+N);
    std::swap_ranges(array, array+N/2, array+N/2);
    std::sort(array, array+N);
    assert(std::is_sorted(array, array+N));
    delete [] array;
}

void
test_larger_sorts(int N)
{
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
    // test null range
    int d = 0;
    std::sort(&d, &d);
    // exhaustively test all possibilities up to length 8
    test_sort_<1>();
    test_sort_<2>();
    test_sort_<3>();
    test_sort_<4>();
    test_sort_<5>();
    test_sort_<6>();
    test_sort_<7>();
    test_sort_<8>();

    test_larger_sorts(256);
    test_larger_sorts(257);
    test_larger_sorts(499);
    test_larger_sorts(500);
    test_larger_sorts(997);
    test_larger_sorts(1000);
    test_larger_sorts(1009);
}
