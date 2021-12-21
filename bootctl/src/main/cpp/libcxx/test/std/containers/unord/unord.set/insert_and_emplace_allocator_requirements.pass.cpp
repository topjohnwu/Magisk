//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// class unordered_set

// insert(...)
// emplace(...)

// UNSUPPORTED: c++98, c++03

#include <unordered_set>

#include "container_test_types.h"
#include "../../set_allocator_requirement_test_templates.h"


int main()
{
  testSetInsert<TCT::unordered_set<> >();
  testSetEmplace<TCT::unordered_set<> >();
}
