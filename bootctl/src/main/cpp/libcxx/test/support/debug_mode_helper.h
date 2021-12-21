//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef TEST_SUPPORT_DEBUG_MODE_HELPER_H
#define TEST_SUPPORT_DEBUG_MODE_HELPER_H

#ifndef _LIBCPP_DEBUG
#error _LIBCPP_DEBUG must be defined before including this header
#endif
#ifndef _LIBCPP_DEBUG_USE_EXCEPTIONS
#error _LIBCPP_DEBUG_USE_EXCEPTIONS must be defined before including this header
#endif

#include <ciso646>
#ifndef _LIBCPP_VERSION
#error This header may only be used for libc++ tests"
#endif

#include <__debug>
#include <utility>
#include <cstddef>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"
#include "assert_checkpoint.h"
#include "test_allocator.h"

// These test make use of 'if constexpr'.
#if TEST_STD_VER <= 14
#error This header may only be used in C++17 and greater
#endif
#ifdef TEST_HAS_NO_EXCEPTIONS
#error These tests require exceptions
#endif

#ifndef __cpp_if_constexpr
#error These tests require if constexpr
#endif

/// Assert that the specified expression throws a libc++ debug exception.
#define CHECK_DEBUG_THROWS(...) assert((CheckDebugThrows( [&]() { __VA_ARGS__; } )))

template <class Func>
inline bool CheckDebugThrows(Func&& func) {
  try {
    func();
  } catch (std::__libcpp_debug_exception const&) {
    return true;
  }
  return false;
}

namespace IteratorDebugChecks {

enum ContainerType {
  CT_None,
  CT_String,
  CT_Vector,
  CT_VectorBool,
  CT_List,
  CT_Deque,
  CT_ForwardList,
  CT_Map,
  CT_Set,
  CT_MultiMap,
  CT_MultiSet,
  CT_UnorderedMap,
  CT_UnorderedSet,
  CT_UnorderedMultiMap,
  CT_UnorderedMultiSet
};

constexpr bool isSequential(ContainerType CT) {
  return CT_Vector >= CT && CT_ForwardList <= CT;
}

constexpr bool isAssociative(ContainerType CT) {
  return CT_Map >= CT && CT_MultiSet <= CT;
}

constexpr bool isUnordered(ContainerType CT) {
  return CT_UnorderedMap >= CT && CT_UnorderedMultiSet <= CT;
}

constexpr bool isSet(ContainerType CT) {
  return CT == CT_Set
      || CT == CT_MultiSet
      || CT == CT_UnorderedSet
      || CT == CT_UnorderedMultiSet;
}

constexpr bool isMap(ContainerType CT) {
  return CT == CT_Map
      || CT == CT_MultiMap
      || CT == CT_UnorderedMap
      || CT == CT_UnorderedMultiMap;
}

constexpr bool isMulti(ContainerType CT) {
  return CT == CT_MultiMap
      || CT == CT_MultiSet
      || CT == CT_UnorderedMultiMap
      || CT == CT_UnorderedMultiSet;
}

template <class Container, class ValueType = typename Container::value_type>
struct ContainerDebugHelper {
  static_assert(std::is_constructible<ValueType, int>::value,
                "must be constructible from int");

  static ValueType makeValueType(int val = 0, int = 0) {
    return ValueType(val);
  }
};

template <class Container>
struct ContainerDebugHelper<Container, char> {
  static char makeValueType(int = 0, int = 0) {
    return 'A';
  }
};

template <class Container, class Key, class Value>
struct ContainerDebugHelper<Container, std::pair<const Key, Value> > {
  using ValueType = std::pair<const Key, Value>;
  static_assert(std::is_constructible<Key, int>::value,
                "must be constructible from int");
  static_assert(std::is_constructible<Value, int>::value,
                "must be constructible from int");

  static ValueType makeValueType(int key = 0, int val = 0) {
    return ValueType(key, val);
  }
};

template <class Container, ContainerType CT,
    class Helper = ContainerDebugHelper<Container> >
struct BasicContainerChecks {
  using value_type = typename Container::value_type;
  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;
  using allocator_type = typename Container::allocator_type;
  using traits = std::iterator_traits<iterator>;
  using category = typename traits::iterator_category;

  static_assert(std::is_same<test_allocator<value_type>, allocator_type>::value,
                "the container must use a test allocator");

  static constexpr bool IsBiDir =
      std::is_convertible<category, std::bidirectional_iterator_tag>::value;

public:
  static void run() {
    run_iterator_tests();
    run_container_tests();
    run_allocator_aware_tests();
  }

  static void run_iterator_tests() {
    try {
      TestNullIterators<iterator>();
      TestNullIterators<const_iterator>();
      if constexpr (IsBiDir) { DecrementBegin(); }
      IncrementEnd();
      DerefEndIterator();
    } catch (...) {
      assert(false && "uncaught debug exception");
    }
  }

  static void run_container_tests() {
    try {
      CopyInvalidatesIterators();
      MoveInvalidatesIterators();
      if constexpr (CT != CT_ForwardList) {
          EraseIter();
          EraseIterIter();
      }
    } catch (...) {
      assert(false && "uncaught debug exception");
    }
  }

  static void run_allocator_aware_tests() {
    try {
      SwapNonEqualAllocators();
      if constexpr (CT != CT_ForwardList ) {
          // FIXME: This should work for both forward_list and string
          SwapInvalidatesIterators();
      }
    } catch (...) {
      assert(false && "uncaught debug exception");
    }
  }

  static Container makeContainer(int size, allocator_type A = allocator_type()) {
    Container C(A);
    if constexpr (CT == CT_ForwardList) {
      for (int i = 0; i < size; ++i)
        C.insert_after(C.before_begin(), Helper::makeValueType(i));
    } else {
      for (int i = 0; i < size; ++i)
        C.insert(C.end(), Helper::makeValueType(i));
      assert(C.size() == static_cast<std::size_t>(size));
    }
    return C;
  }

  static value_type makeValueType(int value) {
    return Helper::makeValueType(value);
  }

private:
  // Iterator tests
  template <class Iter>
  static void TestNullIterators() {
    CHECKPOINT("testing null iterator");
    Iter it;
    CHECK_DEBUG_THROWS( ++it );
    CHECK_DEBUG_THROWS( it++ );
    CHECK_DEBUG_THROWS( *it );
    if constexpr (CT != CT_VectorBool) {
      CHECK_DEBUG_THROWS( it.operator->() );
    }
    if constexpr (IsBiDir) {
      CHECK_DEBUG_THROWS( --it );
      CHECK_DEBUG_THROWS( it-- );
    }
  }

  static void DecrementBegin() {
    CHECKPOINT("testing decrement on begin");
    Container C = makeContainer(1);
    iterator i = C.end();
    const_iterator ci = C.cend();
    --i;
    --ci;
    assert(i == C.begin());
    CHECK_DEBUG_THROWS( --i );
    CHECK_DEBUG_THROWS( i-- );
    CHECK_DEBUG_THROWS( --ci );
    CHECK_DEBUG_THROWS( ci-- );
  }

  static void IncrementEnd() {
    CHECKPOINT("testing increment on end");
    Container C = makeContainer(1);
    iterator i = C.begin();
    const_iterator ci = C.begin();
    ++i;
    ++ci;
    assert(i == C.end());
    CHECK_DEBUG_THROWS( ++i );
    CHECK_DEBUG_THROWS( i++ );
    CHECK_DEBUG_THROWS( ++ci );
    CHECK_DEBUG_THROWS( ci++ );
  }

  static void DerefEndIterator() {
    CHECKPOINT("testing deref end iterator");
    Container C = makeContainer(1);
    iterator i = C.begin();
    const_iterator ci = C.cbegin();
    (void)*i; (void)*ci;
    if constexpr (CT != CT_VectorBool) {
      i.operator->();
      ci.operator->();
    }
    ++i; ++ci;
    assert(i == C.end());
    CHECK_DEBUG_THROWS( *i );
    CHECK_DEBUG_THROWS( *ci );
    if constexpr (CT != CT_VectorBool) {
      CHECK_DEBUG_THROWS( i.operator->() );
      CHECK_DEBUG_THROWS( ci.operator->() );
    }
  }

  // Container tests
  static void CopyInvalidatesIterators() {
    CHECKPOINT("copy invalidates iterators");
    Container C1 = makeContainer(3);
    iterator i = C1.begin();
    Container C2 = C1;
    if constexpr (CT == CT_ForwardList) {
      iterator i_next = i;
      ++i_next;
      (void)*i_next;
      CHECK_DEBUG_THROWS( C2.erase_after(i) );
      C1.erase_after(i);
      CHECK_DEBUG_THROWS( *i_next );
    } else {
      CHECK_DEBUG_THROWS( C2.erase(i) );
      (void)*i;
      C1.erase(i);
      CHECK_DEBUG_THROWS( *i );
    }
  }

  static void MoveInvalidatesIterators() {
    CHECKPOINT("copy move invalidates iterators");
    Container C1 = makeContainer(3);
    iterator i = C1.begin();
    Container C2 = std::move(C1);
    (void) *i;
    if constexpr (CT == CT_ForwardList) {
      CHECK_DEBUG_THROWS( C1.erase_after(i) );
      C2.erase_after(i);
    } else {
      CHECK_DEBUG_THROWS( C1.erase(i) );
      C2.erase(i);
      CHECK_DEBUG_THROWS(*i);
    }
  }

  static void EraseIter() {
    CHECKPOINT("testing erase invalidation");
    Container C1 = makeContainer(2);
    iterator it1 = C1.begin();
    iterator it1_next = it1;
    ++it1_next;
    Container C2 = C1;
    CHECK_DEBUG_THROWS( C2.erase(it1) ); // wrong container
    CHECK_DEBUG_THROWS( C2.erase(C2.end()) ); // erase with end
    C1.erase(it1_next);
    CHECK_DEBUG_THROWS( C1.erase(it1_next) ); // invalidated iterator
    C1.erase(it1);
    CHECK_DEBUG_THROWS( C1.erase(it1) ); // invalidated iterator
  }

  static void EraseIterIter() {
    CHECKPOINT("testing erase iter iter invalidation");
    Container C1 = makeContainer(2);
    iterator it1 = C1.begin();
    iterator it1_next = it1;
    ++it1_next;
    Container C2 = C1;
    iterator it2 = C2.begin();
    iterator it2_next = it2;
    ++it2_next;
    CHECK_DEBUG_THROWS( C2.erase(it1, it1_next) ); // begin from wrong container
    CHECK_DEBUG_THROWS( C2.erase(it1, it2_next) ); // end   from wrong container
    CHECK_DEBUG_THROWS( C2.erase(it2, it1_next) ); // both  from wrong container
    C2.erase(it2, it2_next);
  }

  // Allocator aware tests
  static void SwapInvalidatesIterators() {
    CHECKPOINT("testing swap invalidates iterators");
    Container C1 = makeContainer(3);
    Container C2 = makeContainer(3);
    iterator it1 = C1.begin();
    iterator it2 = C2.begin();
    swap(C1, C2);
    CHECK_DEBUG_THROWS( C1.erase(it1) );
    if (CT == CT_String) {
      CHECK_DEBUG_THROWS(C1.erase(it2));
    } else
      C1.erase(it2);
    //C2.erase(it1);
    CHECK_DEBUG_THROWS( C1.erase(it1) );
  }

  static void SwapNonEqualAllocators() {
    CHECKPOINT("testing swap with non-equal allocators");
    Container C1 = makeContainer(3, allocator_type(1));
    Container C2 = makeContainer(1, allocator_type(2));
    Container C3 = makeContainer(2, allocator_type(2));
    swap(C2, C3);
    CHECK_DEBUG_THROWS( swap(C1, C2) );
  }

private:
  BasicContainerChecks() = delete;
};

} // namespace IteratorDebugChecks

#endif // TEST_SUPPORT_DEBUG_MODE_HELPER_H
