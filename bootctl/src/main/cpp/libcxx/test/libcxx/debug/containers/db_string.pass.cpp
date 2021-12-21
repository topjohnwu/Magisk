//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-exceptions, libcpp-no-if-constexpr
// MODULES_DEFINES: _LIBCPP_DEBUG=1
// MODULES_DEFINES: _LIBCPP_DEBUG_USE_EXCEPTIONS

// Can't test the system lib because this test enables debug mode
// UNSUPPORTED: with_system_cxx_lib

// test container debugging

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_USE_EXCEPTIONS
#include <string>
#include <vector>

#include "test_macros.h"
#include "debug_mode_helper.h"

using namespace IteratorDebugChecks;

typedef std::basic_string<char, std::char_traits<char>, test_allocator<char>>  StringType;

template <class Container = StringType, ContainerType CT = CT_String>
struct StringContainerChecks : BasicContainerChecks<Container, CT> {
  using Base = BasicContainerChecks<Container, CT_String>;
  using value_type = typename Container::value_type;
  using allocator_type = typename Container::allocator_type;
  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;

  using Base::makeContainer;
  using Base::makeValueType;

public:
  static void run() {
    Base::run_iterator_tests();
    Base::run_allocator_aware_tests();
    try {
      for (int N : {3, 128}) {
        FrontOnEmptyContainer(N);
        BackOnEmptyContainer(N);
        PopBack(N);
      }
    } catch (...) {
      assert(false && "uncaught debug exception");
    }
  }

private:
  static void BackOnEmptyContainer(int N) {
    CHECKPOINT("testing back on empty");
    Container C = makeContainer(N);
    Container const& CC = C;
    iterator it = --C.end();
    (void)C.back();
    (void)CC.back();
    C.pop_back();
    CHECK_DEBUG_THROWS( C.erase(it) );
    C.clear();
    CHECK_DEBUG_THROWS( C.back() );
    CHECK_DEBUG_THROWS( CC.back() );
  }

  static void FrontOnEmptyContainer(int N) {
    CHECKPOINT("testing front on empty");
    Container C = makeContainer(N);
    Container const& CC = C;
    (void)C.front();
    (void)CC.front();
    C.clear();
    CHECK_DEBUG_THROWS( C.front() );
    CHECK_DEBUG_THROWS( CC.front() );
  }

  static void PopBack(int N) {
    CHECKPOINT("testing pop_back() invalidation");
    Container C1 = makeContainer(N);
    iterator it1 = C1.end();
    --it1;
    C1.pop_back();
    CHECK_DEBUG_THROWS( C1.erase(it1) );
    C1.erase(C1.begin(), C1.end());
    assert(C1.size() == 0);
    CHECK_DEBUG_THROWS( C1.pop_back() );
  }
};

int main()
{
  StringContainerChecks<>::run();
}
