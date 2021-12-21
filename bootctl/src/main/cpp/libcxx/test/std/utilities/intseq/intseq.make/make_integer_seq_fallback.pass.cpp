//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template<class T, T N>
//   using make_integer_sequence = integer_sequence<T, 0, 1, ..., N-1>;

// UNSUPPORTED: c++98, c++03, c++11

#define _LIBCPP_TESTING_FALLBACK_MAKE_INTEGER_SEQUENCE
#include "make_integer_seq.pass.cpp"
