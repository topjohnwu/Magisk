//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// unique_ptr

// Test unique_ptr::pointer type

#include <memory>
#include <type_traits>

#include "test_macros.h"

struct Deleter {
  struct pointer {};
};

struct D2 {
private:
  typedef void pointer;
};

struct D3 {
  static long pointer;
};

template <bool IsArray>
void test_basic() {
  typedef typename std::conditional<IsArray, int[], int>::type VT;
  {
    typedef std::unique_ptr<VT> P;
    static_assert((std::is_same<typename P::pointer, int*>::value), "");
  }
  {
    typedef std::unique_ptr<VT, Deleter> P;
    static_assert((std::is_same<typename P::pointer, Deleter::pointer>::value),
                  "");
  }
#if TEST_STD_VER >= 11
  {
    typedef std::unique_ptr<VT, D2> P;
    static_assert(std::is_same<typename P::pointer, int*>::value, "");
  }
  {
    typedef std::unique_ptr<VT, D3> P;
    static_assert(std::is_same<typename P::pointer, int*>::value, "");
  }
#endif
}

int main() {
  test_basic</*IsArray*/ false>();
  test_basic<true>();
}
