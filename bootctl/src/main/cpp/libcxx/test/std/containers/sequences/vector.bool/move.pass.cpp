//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <vector>

// vector(vector&& c);

#include <vector>
#include <cassert>
#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        std::vector<bool, test_allocator<bool> > l(test_allocator<bool>(5));
        std::vector<bool, test_allocator<bool> > lo(test_allocator<bool>(5));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(true);
            lo.push_back(true);
        }
        std::vector<bool, test_allocator<bool> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
    }
    {
        std::vector<bool, other_allocator<bool> > l(other_allocator<bool>(5));
        std::vector<bool, other_allocator<bool> > lo(other_allocator<bool>(5));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(true);
            lo.push_back(true);
        }
        std::vector<bool, other_allocator<bool> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
    }
    {
        std::vector<bool, min_allocator<bool> > l(min_allocator<bool>{});
        std::vector<bool, min_allocator<bool> > lo(min_allocator<bool>{});
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(true);
            lo.push_back(true);
        }
        std::vector<bool, min_allocator<bool> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
    }
    {
      test_alloc_base::clear();
      using Vect = std::vector<bool, test_allocator<bool> >;
      using AllocT = Vect::allocator_type;
      Vect v(test_allocator<bool>(42, 101));
      assert(test_alloc_base::count == 1);
      {
        const AllocT& a = v.get_allocator();
        assert(test_alloc_base::count == 2);
        assert(a.get_data() == 42);
        assert(a.get_id() == 101);
      }
      assert(test_alloc_base::count == 1);
      test_alloc_base::clear_ctor_counters();

      Vect v2 = std::move(v);
      assert(test_alloc_base::count == 2);
      assert(test_alloc_base::copied == 0);
      assert(test_alloc_base::moved == 1);
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
