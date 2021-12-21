// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "demangle.h"
#include <typeinfo>
#include <cassert>

struct MyType {};

template <class T, class U> struct ArgumentListID {};

int main() {
  struct {
    const char* raw;
    const char* expect;
  } TestCases[] = {
      {typeid(int).name(), "int"},
      {typeid(MyType).name(), "MyType"},
      {typeid(ArgumentListID<int, MyType>).name(), "ArgumentListID<int, MyType>"}
  };
  const size_t size = sizeof(TestCases) / sizeof(TestCases[0]);
  for (size_t i=0; i < size; ++i) {
    const char* raw = TestCases[i].raw;
    const char* expect = TestCases[i].expect;
#ifdef TEST_HAS_NO_DEMANGLE
    assert(demangle(raw) == raw);
    ((void)expect);
#else
    assert(demangle(raw) == expect);
#endif
  }
}
