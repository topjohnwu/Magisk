//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// class unordered_map

// insert(...);
// emplace(...);

// UNSUPPORTED: c++98, c++03


#include <unordered_map>

#include "container_test_types.h"
#include "../../../map_allocator_requirement_test_templates.h"

int main()
{
  testMapInsert<TCT::unordered_map<> >();
  testMapInsertHint<TCT::unordered_map<> >();
  testMapEmplace<TCT::unordered_map<> >();
  testMapEmplaceHint<TCT::unordered_map<> >();
}
