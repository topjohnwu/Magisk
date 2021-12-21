// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <variant>

// template <class ...Types> class variant;

#include <limits>
#include <type_traits>
#include <utility>
#include <variant>

template <class Sequence>
struct make_variant_imp;

template <size_t ...Indices>
struct make_variant_imp<std::integer_sequence<size_t, Indices...>> {
  template <size_t> using AlwaysChar = char;
  using type = std::variant<AlwaysChar<Indices>...>;
};

template <size_t N>
using make_variant_t = typename make_variant_imp<std::make_index_sequence<N>>::type;

constexpr bool ExpectEqual =
#ifdef _LIBCPP_ABI_VARIANT_INDEX_TYPE_OPTIMIZATION
  false;
#else
  true;
#endif

template <class IndexType>
void test_index_type() {
  using Lim = std::numeric_limits<IndexType>;
  using T1 = make_variant_t<Lim::max() - 1>;
  using T2 = make_variant_t<Lim::max()>;
  static_assert((sizeof(T1) == sizeof(T2)) == ExpectEqual, "");
}

template <class IndexType>
void test_index_internals() {
  using Lim = std::numeric_limits<IndexType>;
  static_assert(std::__choose_index_type(Lim::max() -1) !=
                std::__choose_index_type(Lim::max()), "");
  static_assert(std::is_same_v<
      std::__variant_index_t<Lim::max()-1>,
      std::__variant_index_t<Lim::max()>
    > == ExpectEqual, "");
  using IndexT = std::__variant_index_t<Lim::max()-1>;
  using IndexLim = std::numeric_limits<IndexT>;
  static_assert(std::__variant_npos<IndexT> == IndexLim::max(), "");
}

int main() {
  test_index_type<unsigned char>();
  // This won't compile due to template depth issues.
  //test_index_type<unsigned short>();
  test_index_internals<unsigned char>();
  test_index_internals<unsigned short>();
}
