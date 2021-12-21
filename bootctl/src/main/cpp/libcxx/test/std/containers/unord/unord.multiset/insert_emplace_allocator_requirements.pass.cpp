//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// class unordered_multiset

// insert(...)

// UNSUPPORTED: c++98, c++03

#include <unordered_set>
#include "container_test_types.h"
#include "../../set_allocator_requirement_test_templates.h"

int main()
{
  testMultisetInsert<TCT::unordered_multiset<> >();
  testMultisetEmplace<TCT::unordered_multiset<> >();
}
