//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <__hash_table>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include "test_macros.h"
#include "min_allocator.h"

void testKeyValueTrait() {
  {
    typedef int Tp;
    typedef std::__hash_key_value_types<Tp> Traits;
    static_assert((std::is_same<Traits::key_type, int>::value), "");
    static_assert((std::is_same<Traits::__node_value_type, Tp>::value), "");
    static_assert((std::is_same<Traits::__container_value_type, Tp>::value), "");
    static_assert(Traits::__is_map == false, "");
  }
  {
    typedef std::pair<int, int> Tp;
    typedef std::__hash_key_value_types<Tp> Traits;
    static_assert((std::is_same<Traits::key_type, Tp>::value), "");
    static_assert((std::is_same<Traits::__node_value_type, Tp>::value), "");
    static_assert((std::is_same<Traits::__container_value_type, Tp>::value), "");
    static_assert(Traits::__is_map == false, "");
  }
  {
    typedef std::pair<const int, int> Tp;
    typedef std::__hash_key_value_types<Tp> Traits;
    static_assert((std::is_same<Traits::key_type, Tp>::value), "");
    static_assert((std::is_same<Traits::__node_value_type, Tp>::value), "");
    static_assert((std::is_same<Traits::__container_value_type, Tp>::value), "");
    static_assert(Traits::__is_map == false, "");
  }
  {
    typedef std::__hash_value_type<int, int> Tp;
    typedef std::__hash_key_value_types<Tp> Traits;
    static_assert((std::is_same<Traits::key_type, int>::value), "");
    static_assert((std::is_same<Traits::mapped_type, int>::value), "");
    static_assert((std::is_same<Traits::__node_value_type, Tp>::value), "");
    static_assert((std::is_same<Traits::__container_value_type,
                               std::pair<const int, int> >::value), "");
    static_assert((std::is_same<Traits::__map_value_type,
                               std::pair<const int, int> >::value), "");
    static_assert(Traits::__is_map == true, "");
  }
}

int main() {
  testKeyValueTrait();
}
