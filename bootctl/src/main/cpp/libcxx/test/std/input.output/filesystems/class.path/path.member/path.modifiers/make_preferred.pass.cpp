//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// class path

// path& make_preferred()

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"


struct MakePreferredTestcase {
  const char* value;
};

const MakePreferredTestcase TestCases[] =
  {
      {""}
    , {"hello_world"}
    , {"/"}
    , {"/foo/bar/baz/"}
    , {"\\"}
    , {"\\foo\\bar\\baz\\"}
    , {"\\foo\\/bar\\/baz\\"}
  };

int main()
{
  // This operation is an identity operation on linux.
  using namespace fs;
  for (auto const & TC : TestCases) {
    path p(TC.value);
    assert(p == TC.value);
    path& Ref = (p.make_preferred());
    assert(p.native() == TC.value);
    assert(&Ref == &p);
  }
}
