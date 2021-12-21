//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// iterator erase(const_iterator position);

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
    {
    int a1[] = {1, 2, 3};
    std::vector<int> l1(a1, a1+3);
    std::vector<int>::const_iterator i = l1.begin();
    assert(is_contiguous_container_asan_correct(l1));
    ++i;
    std::vector<int>::iterator j = l1.erase(i);
    assert(l1.size() == 2);
    assert(distance(l1.begin(), l1.end()) == 2);
    assert(*j == 3);
    assert(*l1.begin() == 1);
    assert(*next(l1.begin()) == 3);
    assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(j);
    assert(j == l1.end());
    assert(l1.size() == 1);
    assert(distance(l1.begin(), l1.end()) == 1);
    assert(*l1.begin() == 1);
    assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(l1.begin());
    assert(j == l1.end());
    assert(l1.size() == 0);
    assert(distance(l1.begin(), l1.end()) == 0);
    assert(is_contiguous_container_asan_correct(l1));
    }
#if TEST_STD_VER >= 11
    {
    int a1[] = {1, 2, 3};
    std::vector<int, min_allocator<int>> l1(a1, a1+3);
    std::vector<int, min_allocator<int>>::const_iterator i = l1.begin();
    assert(is_contiguous_container_asan_correct(l1));
    ++i;
    std::vector<int, min_allocator<int>>::iterator j = l1.erase(i);
    assert(l1.size() == 2);
    assert(distance(l1.begin(), l1.end()) == 2);
    assert(*j == 3);
    assert(*l1.begin() == 1);
    assert(*next(l1.begin()) == 3);
    assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(j);
    assert(j == l1.end());
    assert(l1.size() == 1);
    assert(distance(l1.begin(), l1.end()) == 1);
    assert(*l1.begin() == 1);
    assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(l1.begin());
    assert(j == l1.end());
    assert(l1.size() == 0);
    assert(distance(l1.begin(), l1.end()) == 0);
    assert(is_contiguous_container_asan_correct(l1));
    }
#endif
#ifndef TEST_HAS_NO_EXCEPTIONS
// Test for LWG2853:
// Throws: Nothing unless an exception is thrown by the assignment operator or move assignment operator of T.
    {
    Throws arr[] = {1, 2, 3};
    std::vector<Throws> v(arr, arr+3);
    Throws::sThrows = true;
    v.erase(v.begin());
    v.erase(--v.end());
    v.erase(v.begin());
    assert(v.size() == 0);
    }
#endif
}
