//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include "test_macros.h"
#include "min_allocator.h"
#include "test_allocator.h"


template <class Map, class ValueTp, class PtrT, class CPtrT>
void testUnorderedMap() {
  typedef typename Map::difference_type Diff;
  {
    typedef typename Map::iterator It;
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp&>::value), "");
    static_assert((std::is_same<typename It::pointer, PtrT>::value), "");
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");
  }
  {
    typedef typename Map::const_iterator It;
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp const&>::value), "");
    static_assert((std::is_same<typename It::pointer, CPtrT>::value), "");
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");
  }
  {
    typedef typename Map::local_iterator It;
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp&>::value), "");
    static_assert((std::is_same<typename It::pointer, PtrT>::value), "");
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");
  }
  {
    typedef typename Map::const_local_iterator It;
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp const&>::value), "");
    static_assert((std::is_same<typename It::pointer, CPtrT>::value), "");
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");
  }
}


template <class Set, class ValueTp, class CPtrT>
void testUnorderedSet() {
  static_assert((std::is_same<typename Set::iterator,
                             typename Set::const_iterator>::value), "");
  static_assert((std::is_same<typename Set::local_iterator,
                             typename Set::const_local_iterator>::value), "");
  typedef typename Set::difference_type Diff;
  {
    typedef typename Set::iterator It;
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp const&>::value), "");
    static_assert((std::is_same<typename It::pointer, CPtrT>::value), "");
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");

  }
  {
    typedef typename Set::local_iterator It;
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp const&>::value), "");
    static_assert((std::is_same<typename It::pointer, CPtrT>::value), "");
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");
  }
}

int main() {
  {
    typedef std::unordered_map<int, int> Map;
    typedef std::pair<const int, int> ValueTp;
    testUnorderedMap<Map, ValueTp, ValueTp*, ValueTp const*>();
  }
  {
    typedef std::pair<const int, int> ValueTp;
    typedef test_allocator<ValueTp> Alloc;
    typedef std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, Alloc> Map;
    testUnorderedMap<Map, ValueTp, ValueTp*, ValueTp const*>();
  }
#if TEST_STD_VER >= 11
  {
    typedef std::pair<const int, int> ValueTp;
    typedef min_allocator<ValueTp> Alloc;
    typedef std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, Alloc> Map;
    testUnorderedMap<Map, ValueTp, min_pointer<ValueTp>, min_pointer<const ValueTp>>();
  }
#endif
  {
    typedef std::unordered_multimap<int, int> Map;
    typedef std::pair<const int, int> ValueTp;
    testUnorderedMap<Map, ValueTp, ValueTp*, ValueTp const*>();
  }
  {
    typedef std::pair<const int, int> ValueTp;
    typedef test_allocator<ValueTp> Alloc;
    typedef std::unordered_multimap<int, int, std::hash<int>, std::equal_to<int>, Alloc> Map;
    testUnorderedMap<Map, ValueTp, ValueTp*, ValueTp const*>();
  }
#if TEST_STD_VER >= 11
  {
    typedef std::pair<const int, int> ValueTp;
    typedef min_allocator<ValueTp> Alloc;
    typedef std::unordered_multimap<int, int, std::hash<int>, std::equal_to<int>, Alloc> Map;
    testUnorderedMap<Map, ValueTp, min_pointer<ValueTp>, min_pointer<const ValueTp>>();
  }
#endif
  {
    typedef int ValueTp;
    typedef std::unordered_set<ValueTp> Set;
    testUnorderedSet<Set, ValueTp, ValueTp const*>();
  }
  {
    typedef int ValueTp;
    typedef test_allocator<ValueTp> Alloc;
    typedef std::unordered_set<ValueTp, std::hash<ValueTp>, std::equal_to<ValueTp>, Alloc> Set;
    testUnorderedSet<Set, ValueTp, ValueTp const*>();
  }
#if TEST_STD_VER >= 11
  {
    typedef int ValueTp;
    typedef min_allocator<ValueTp> Alloc;
    typedef std::unordered_set<ValueTp, std::hash<ValueTp>, std::equal_to<ValueTp>, Alloc> Set;
    testUnorderedSet<Set, ValueTp, min_pointer<const ValueTp>>();
  }
#endif
  {
    typedef int ValueTp;
    typedef std::unordered_multiset<ValueTp> Set;
    testUnorderedSet<Set, ValueTp, ValueTp const*>();
  }
  {
    typedef int ValueTp;
    typedef test_allocator<ValueTp> Alloc;
    typedef std::unordered_multiset<ValueTp, std::hash<ValueTp>, std::equal_to<ValueTp>, Alloc> Set;
    testUnorderedSet<Set, ValueTp, ValueTp const*>();
  }
#if TEST_STD_VER >= 11
  {
    typedef int ValueTp;
    typedef min_allocator<ValueTp> Alloc;
    typedef std::unordered_multiset<ValueTp, std::hash<ValueTp>, std::equal_to<ValueTp>, Alloc> Set;
    testUnorderedSet<Set, ValueTp, min_pointer<const ValueTp>>();
  }
#endif
}
