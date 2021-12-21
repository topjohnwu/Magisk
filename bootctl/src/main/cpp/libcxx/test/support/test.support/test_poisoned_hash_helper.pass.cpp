//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// Test that the header `poisoned_hash_helper.hpp` doesn't include any
// headers that provide hash<T> specializations. This is required so that the
// 'test_library_hash_specializations_available()' function returns false
// by default, unless a STL header providing hash has already been included.

#include "poisoned_hash_helper.hpp"

template <class T, size_t = sizeof(T)>
constexpr bool is_complete_imp(int) { return true; }
template <class> constexpr bool is_complete_imp(long) { return false; }
template <class T> constexpr bool is_complete() { return is_complete_imp<T>(0); }

template <class T> struct has_complete_hash {
  enum { value = is_complete<std::hash<T> >() };
};

int main() {
  static_assert(LibraryHashTypes::assertTrait<has_complete_hash, false>(), "");
}
