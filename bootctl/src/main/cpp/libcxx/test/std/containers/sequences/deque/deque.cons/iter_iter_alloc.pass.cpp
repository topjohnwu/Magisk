//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// template <class InputIterator>
//   deque(InputIterator f, InputIterator l, const allocator_type& a);

#include <deque>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_iterators.h"
#include "test_allocator.h"
#include "min_allocator.h"
#if TEST_STD_VER >= 11
#include "emplace_constructible.h"
#endif

template <class InputIterator, class Allocator>
void
test(InputIterator f, InputIterator l, const Allocator& a)
{
    typedef typename std::iterator_traits<InputIterator>::value_type T;
    typedef std::deque<T, Allocator> C;
    typedef typename C::const_iterator const_iterator;
    C d(f, l, a);
    assert(d.get_allocator() == a);
    assert(d.size() == static_cast<std::size_t>(std::distance(f, l)));
    assert(static_cast<std::size_t>(distance(d.begin(), d.end())) == d.size());
    for (const_iterator i = d.begin(), e = d.end(); i != e; ++i, ++f)
        assert(*i == *f);
}

void basic_test()
{
    int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
    int* an = ab + sizeof(ab)/sizeof(ab[0]);
    test(input_iterator<const int*>(ab), input_iterator<const int*>(an), test_allocator<int>(3));
    test(forward_iterator<const int*>(ab), forward_iterator<const int*>(an), test_allocator<int>(4));
    test(bidirectional_iterator<const int*>(ab), bidirectional_iterator<const int*>(an), test_allocator<int>(5));
    test(random_access_iterator<const int*>(ab), random_access_iterator<const int*>(an), test_allocator<int>(6));
#if TEST_STD_VER >= 11
    test(input_iterator<const int*>(ab), input_iterator<const int*>(an), min_allocator<int>());
    test(forward_iterator<const int*>(ab), forward_iterator<const int*>(an), min_allocator<int>());
    test(bidirectional_iterator<const int*>(ab), bidirectional_iterator<const int*>(an), min_allocator<int>());
    test(random_access_iterator<const int*>(ab), random_access_iterator<const int*>(an), min_allocator<int>());
#endif
}


void test_emplacable_concept() {
#if TEST_STD_VER >= 11
  int arr1[] = {42};
  int arr2[] = {1, 101, 42};
  {
    using T = EmplaceConstructibleAndMoveable<int>;
    using It = random_access_iterator<int*>;
    std::allocator<T> a;
    {
      std::deque<T> v(It(arr1), It(std::end(arr1)), a);
      assert(v[0].value == 42);
    }
    {
      std::deque<T> v(It(arr2), It(std::end(arr2)), a);
      assert(v[0].value == 1);
      assert(v[1].value == 101);
      assert(v[2].value == 42);
    }
  }
  {
    using T = EmplaceConstructibleAndMoveable<int>;
    using It = input_iterator<int*>;
    std::allocator<T> a;
    {
      std::deque<T> v(It(arr1), It(std::end(arr1)), a);
      assert(v[0].copied == 0);
      assert(v[0].value == 42);
    }
    {
      std::deque<T> v(It(arr2), It(std::end(arr2)), a);
      //assert(v[0].copied == 0);
      assert(v[0].value == 1);
      //assert(v[1].copied == 0);
      assert(v[1].value == 101);
      assert(v[2].copied == 0);
      assert(v[2].value == 42);
    }
  }
#endif
}

int main() {
  basic_test();
  test_emplacable_concept();
}
