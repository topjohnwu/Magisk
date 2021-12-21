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

// unique_ptr(nullptr_t);

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"


#if defined(_LIBCPP_VERSION) && TEST_STD_VER >= 11
_LIBCPP_SAFE_STATIC std::unique_ptr<int> global_static_unique_ptr_single(nullptr);
_LIBCPP_SAFE_STATIC std::unique_ptr<int[]> global_static_unique_ptr_runtime(nullptr);
#endif


#if TEST_STD_VER >= 11
struct NonDefaultDeleter {
  NonDefaultDeleter() = delete;
  void operator()(void*) const {}
};
#endif

template <class VT>
void test_basic() {
#if TEST_STD_VER >= 11
  {
    using U1 = std::unique_ptr<VT>;
    using U2 = std::unique_ptr<VT, Deleter<VT> >;
    static_assert(std::is_nothrow_constructible<U1, decltype(nullptr)>::value,
                  "");
    static_assert(std::is_nothrow_constructible<U2, decltype(nullptr)>::value,
                  "");
  }
#endif
  {
    std::unique_ptr<VT> p(nullptr);
    assert(p.get() == 0);
  }
  {
    std::unique_ptr<VT, NCDeleter<VT> > p(nullptr);
    assert(p.get() == 0);
    assert(p.get_deleter().state() == 0);
  }
}

template <class VT>
void test_sfinae() {
#if TEST_STD_VER >= 11
  { // the constructor does not participate in overload resultion when
    // the deleter is a pointer type
    using U = std::unique_ptr<VT, void (*)(void*)>;
    static_assert(!std::is_constructible<U, decltype(nullptr)>::value, "");
  }
  { // the constructor does not participate in overload resolution when
    // the deleter is not default constructible
    using Del = CDeleter<VT>;
    using U1 = std::unique_ptr<VT, NonDefaultDeleter>;
    using U2 = std::unique_ptr<VT, Del&>;
    using U3 = std::unique_ptr<VT, Del const&>;
    static_assert(!std::is_constructible<U1, decltype(nullptr)>::value, "");
    static_assert(!std::is_constructible<U2, decltype(nullptr)>::value, "");
    static_assert(!std::is_constructible<U3, decltype(nullptr)>::value, "");
  }
#endif
}

DEFINE_AND_RUN_IS_INCOMPLETE_TEST({
  { doIncompleteTypeTest(0, nullptr); }
  checkNumIncompleteTypeAlive(0);
  {
    doIncompleteTypeTest<IncompleteType, NCDeleter<IncompleteType> >(0,
                                                                     nullptr);
  }
  checkNumIncompleteTypeAlive(0);
  { doIncompleteTypeTest<IncompleteType[]>(0, nullptr); }
  checkNumIncompleteTypeAlive(0);
  {
    doIncompleteTypeTest<IncompleteType[], NCDeleter<IncompleteType[]> >(
        0, nullptr);
  }
  checkNumIncompleteTypeAlive(0);
})

int main() {
  {
    test_basic<int>();
    test_sfinae<int>();
  }
  {
    test_basic<int[]>();
    test_sfinae<int[]>();
  }
}
