//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class multimap

// insert(...)

// UNSUPPORTED: c++98, c++03

#include <map>

#include "container_test_types.h"
#include "../../../map_allocator_requirement_test_templates.h"


int main()
{
  testMultimapInsert<TCT::multimap<> >();
  testMultimapInsertHint<TCT::multimap<> >();
}
