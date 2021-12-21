//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// class unordered_multimap

// size_type max_size() const;

#include <cassert>
#include <limits>
#include <type_traits>
#include <unordered_map>

#include "test_allocator.h"
#include "test_macros.h"

int main()
{
  typedef std::pair<const int, int> KV;
  {
    typedef limited_allocator<KV, 10> A;
    typedef std::unordered_multimap<int, int, std::hash<int>,
                                    std::equal_to<int>, A>
        C;
    C c;
    assert(c.max_size() <= 10);
    LIBCPP_ASSERT(c.max_size() == 10);
  }
  {
    typedef limited_allocator<KV, (size_t)-1> A;
    typedef std::unordered_multimap<int, int, std::hash<int>,
                                    std::equal_to<int>, A>
        C;
    const C::size_type max_dist =
        static_cast<C::size_type>(std::numeric_limits<C::difference_type>::max());
    C c;
    assert(c.max_size() <= max_dist);
    LIBCPP_ASSERT(c.max_size() == max_dist);
    }
    {
      typedef std::unordered_multimap<char, int> C;
    const C::size_type max_dist =
        static_cast<C::size_type>(std::numeric_limits<C::difference_type>::max());
      C c;
      assert(c.max_size() <= max_dist);
      assert(c.max_size() <= alloc_max_size(c.get_allocator()));
    }
}
