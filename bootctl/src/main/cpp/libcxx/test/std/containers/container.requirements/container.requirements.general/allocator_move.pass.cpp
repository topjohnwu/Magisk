//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// C++2a[container.requirements.general]p8
//   Move constructors obtain an allocator by move construction from the allocator
//   belonging to the container being moved. Such move construction of the
//   allocator shall not exit via an exception.

#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "test_macros.h"
#include "test_allocator.h"

template <class C>
void test(int expected_num_allocs = 1) {
  {
    test_alloc_base::clear();
    using AllocT = typename C::allocator_type;
    C v(AllocT(42, 101));

    assert(test_alloc_base::count == expected_num_allocs);

    const int num_stored_allocs = test_alloc_base::count;
    {
      const AllocT& a = v.get_allocator();
      assert(test_alloc_base::count == 1 + num_stored_allocs);
      assert(a.get_data() == 42);
      assert(a.get_id() == 101);
    }
    assert(test_alloc_base::count == num_stored_allocs);
    test_alloc_base::clear_ctor_counters();

    C v2 = std::move(v);
    assert(test_alloc_base::count == num_stored_allocs * 2);
    assert(test_alloc_base::copied == 0);
    assert(test_alloc_base::moved == num_stored_allocs);
    {
      const AllocT& a = v.get_allocator();
      assert(a.get_id() == test_alloc_base::moved_value);
      assert(a.get_data() == test_alloc_base::moved_value);
    }
    {
      const AllocT& a = v2.get_allocator();
      assert(a.get_id() == 101);
      assert(a.get_data() == 42);
    }
  }
}

int main() {
  { // test sequence containers
    test<std::vector<int, test_allocator<int> > >();
    test<std::vector<bool, test_allocator<bool> > >();
    test<std::list<int, test_allocator<int> > >();
    test<std::forward_list<int, test_allocator<int> > >();

    // libc++ stores two allocators in deque
#ifdef _LIBCPP_VERSION
    int stored_allocators = 2;
#else
    int stored_allocators = 1;
#endif
    test<std::deque<int, test_allocator<int> > >(stored_allocators);
  }
  { // test associative containers
    test<std::set<int, std::less<int>, test_allocator<int> > >();
    test<std::multiset<int, std::less<int>, test_allocator<int> > >();

    using KV = std::pair<const int, int>;
    test<std::map<int, int, std::less<int>, test_allocator<KV> > >();
    test<std::multimap<int, int, std::less<int>, test_allocator<KV> > >();
  }
  { // test unordered containers
    // libc++ stores two allocators in the unordered containers.
#ifdef _LIBCPP_VERSION
    int stored_allocators = 2;
#else
    int stored_allocators = 1;
#endif
    test<std::unordered_set<int, std::hash<int>, std::equal_to<int>,
                            test_allocator<int> > >(stored_allocators);
    test<std::unordered_multiset<int, std::hash<int>, std::equal_to<int>,
                                 test_allocator<int> > >(stored_allocators);

    using KV = std::pair<const int, int>;
    test<std::unordered_map<int, int, std::hash<int>, std::equal_to<int>,
                            test_allocator<KV> > >(stored_allocators);
    test<std::unordered_multimap<int, int, std::hash<int>, std::equal_to<int>,
                                 test_allocator<KV> > >(stored_allocators);
  }
}
