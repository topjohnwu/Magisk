//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// insert(...);
// emplace(...);
// emplace_hint(...);

// UNSUPPORTED: c++98, c++03

#include <map>

#include "container_test_types.h"
#include "../../../map_allocator_requirement_test_templates.h"

int main()
{
  testMapInsert<TCT::map<> >();
  testMapInsertHint<TCT::map<> >();
  testMapEmplace<TCT::map<> >();
  testMapEmplaceHint<TCT::map<> >();
}
