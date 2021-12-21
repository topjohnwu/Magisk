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

#include <map>
#include <set>
#include <utility>
#include <cassert>
#include "debug_mode_helper.h"

using namespace IteratorDebugChecks;

template <class Container, ContainerType CT>
struct AssociativeContainerChecks : BasicContainerChecks<Container, CT> {
  using Base = BasicContainerChecks<Container, CT>;
  using value_type = typename Container::value_type;
  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;
  using traits = std::iterator_traits<iterator>;
  using category = typename traits::iterator_category;

  using Base::makeContainer;
public:
  static void run() {
    Base::run();
    try {
     // FIXME Add tests
    } catch (...) {
      assert(false && "uncaught debug exception");
    }
  }

private:
  // FIXME Add tests here
};

int main()
{
  using SetAlloc = test_allocator<int>;
  using MapAlloc = test_allocator<std::pair<const int, int>>;
  // FIXME: Add debug mode to these containers
  if ((false)) {
    AssociativeContainerChecks<
        std::set<int, std::less<int>, SetAlloc>, CT_Set>::run();
    AssociativeContainerChecks<
        std::multiset<int, std::less<int>, SetAlloc>, CT_MultiSet>::run();
    AssociativeContainerChecks<
        std::map<int, int, std::less<int>, MapAlloc>, CT_Map>::run();
    AssociativeContainerChecks<
        std::multimap<int, int, std::less<int>, MapAlloc>, CT_MultiMap>::run();
  }
}
