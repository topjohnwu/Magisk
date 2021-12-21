//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// class unordered_multimap

// insert(...)

// UNSUPPORTED: c++98, c++03

#include <unordered_map>

#include "container_test_types.h"
#include "../../../map_allocator_requirement_test_templates.h"

int main()
{
  testMultimapInsert<TCT::unordered_multimap<> >();
  testMultimapInsertHint<TCT::unordered_multimap<> >();
}
