//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// iterator erase(const_iterator first, const_iterator last);

#include <vector>
#include <iterator>
#include <cassert>

#include "min_allocator.h"
#include "asan_testing.h"

#ifndef TEST_HAS_NO_EXCEPTIONS
struct Throws {
    Throws() : v_(0) {}
    Throws(int v) : v_(v) {}
    Throws(const Throws  &rhs) : v_(rhs.v_) { if (sThrows) throw 1; }
    Throws(      Throws &&rhs) : v_(rhs.v_) { if (sThrows) throw 1; }
    Throws& operator=(const Throws  &rhs) { v_ = rhs.v_; return *this; }
    Throws& operator=(      Throws &&rhs) { v_ = rhs.v_; return *this; }
    int v_;
    static bool sThrows;
    };

bool Throws::sThrows = false;
#endif

int main()
{
    int a1[] = {1, 2, 3};
    {
        std::vector<int> l1(a1, a1+3);
        assert(is_contiguous_container_asan_correct(l1));
        std::vector<int>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        assert(l1.size() == 3);
        assert(distance(l1.cbegin(), l1.cend()) == 3);
        assert(i == l1.begin());
        assert(is_contiguous_container_asan_correct(l1));
    }
    {
        std::vector<int> l1(a1, a1+3);
        assert(is_contiguous_container_asan_correct(l1));
        std::vector<int>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin()));
        assert(l1.size() == 2);
        assert(distance(l1.cbegin(), l1.cend()) == 2);
        assert(i == l1.begin());
        assert(l1 == std::vector<int>(a1+1, a1+3));
        assert(is_contiguous_container_asan_correct(l1));
    }
    {
        std::vector<int> l1(a1, a1+3);
        assert(is_contiguous_container_asan_correct(l1));
        std::vector<int>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 2));
        assert(l1.size() == 1);
        assert(distance(l1.cbegin(), l1.cend()) == 1);
        assert(i == l1.begin());
        assert(l1 == std::vector<int>(a1+2, a1+3));
        assert(is_contiguous_container_asan_correct(l1));
    }
    {
        std::vector<int> l1(a1, a1+3);
        assert(is_contiguous_container_asan_correct(l1));
        std::vector<int>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 3));
        assert(l1.size() == 0);
        assert(distance(l1.cbegin(), l1.cend()) == 0);
        assert(i == l1.begin());
        assert(is_contiguous_container_asan_correct(l1));
    }
    {
        std::vector<std::vector<int> > outer(2, std::vector<int>(1));
        assert(is_contiguous_container_asan_correct(outer));
        assert(is_contiguous_container_asan_correct(outer[0]));
        assert(is_contiguous_container_asan_correct(outer[1]));
        outer.erase(outer.begin(), outer.begin());
        assert(outer.size() == 2);
        assert(outer[0].size() == 1);
        assert(outer[1].size() == 1);
        assert(is_contiguous_container_asan_correct(outer));
        assert(is_contiguous_container_asan_correct(outer[0]));
        assert(is_contiguous_container_asan_correct(outer[1]));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> l1(a1, a1+3);
        assert(is_contiguous_container_asan_correct(l1));
        std::vector<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        assert(l1.size() == 3);
        assert(distance(l1.cbegin(), l1.cend()) == 3);
        assert(i == l1.begin());
        assert(is_contiguous_container_asan_correct(l1));
    }
    {
        std::vector<int, min_allocator<int>> l1(a1, a1+3);
        assert(is_contiguous_container_asan_correct(l1));
        std::vector<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin()));
        assert(l1.size() == 2);
        assert(distance(l1.cbegin(), l1.cend()) == 2);
        assert(i == l1.begin());
        assert((l1 == std::vector<int, min_allocator<int>>(a1+1, a1+3)));
        assert(is_contiguous_container_asan_correct(l1));
    }
    {
        std::vector<int, min_allocator<int>> l1(a1, a1+3);
        assert(is_contiguous_container_asan_correct(l1));
        std::vector<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 2));
        assert(l1.size() == 1);
        assert(distance(l1.cbegin(), l1.cend()) == 1);
        assert(i == l1.begin());
        assert((l1 == std::vector<int, min_allocator<int>>(a1+2, a1+3)));
        assert(is_contiguous_container_asan_correct(l1));
    }
    {
        std::vector<int, min_allocator<int>> l1(a1, a1+3);
        assert(is_contiguous_container_asan_correct(l1));
        std::vector<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 3));
        assert(l1.size() == 0);
        assert(distance(l1.cbegin(), l1.cend()) == 0);
        assert(i == l1.begin());
        assert(is_contiguous_container_asan_correct(l1));
    }
    {
        std::vector<std::vector<int, min_allocator<int>>, min_allocator<std::vector<int, min_allocator<int>>>> outer(2, std::vector<int, min_allocator<int>>(1));
        assert(is_contiguous_container_asan_correct(outer));
        assert(is_contiguous_container_asan_correct(outer[0]));
        assert(is_contiguous_container_asan_correct(outer[1]));
        outer.erase(outer.begin(), outer.begin());
        assert(outer.size() == 2);
        assert(outer[0].size() == 1);
        assert(outer[1].size() == 1);
        assert(is_contiguous_container_asan_correct(outer));
        assert(is_contiguous_container_asan_correct(outer[0]));
        assert(is_contiguous_container_asan_correct(outer[1]));
    }
#endif
#ifndef TEST_HAS_NO_EXCEPTIONS
// Test for LWG2853:
// Throws: Nothing unless an exception is thrown by the assignment operator or move assignment operator of T.
    {
    Throws arr[] = {1, 2, 3};
    std::vector<Throws> v(arr, arr+3);
    Throws::sThrows = true;
    v.erase(v.begin(), --v.end());
    assert(v.size() == 1);
    v.erase(v.begin(), v.end());
    assert(v.size() == 0);
    }
#endif
}
