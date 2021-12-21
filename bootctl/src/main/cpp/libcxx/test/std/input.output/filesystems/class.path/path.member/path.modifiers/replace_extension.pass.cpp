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

// path& replace_extension(path const& p = path())

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"


struct ReplaceExtensionTestcase {
  const char* value;
  const char* expect;
  const char* extension;
};

const ReplaceExtensionTestcase TestCases[] =
  {
      {"", "", ""}
    , {"foo.cpp", "foo", ""}
    , {"foo.cpp", "foo.", "."}
    , {"foo..cpp", "foo..txt", "txt"}
    , {"", ".txt", "txt"}
    , {"", ".txt", ".txt"}
    , {"/foo", "/foo.txt", ".txt"}
    , {"/foo", "/foo.txt", "txt"}
    , {"/foo.cpp", "/foo.txt", ".txt"}
    , {"/foo.cpp", "/foo.txt", "txt"}
  };
const ReplaceExtensionTestcase NoArgCases[] =
  {
      {"", "", ""}
    , {"foo", "foo", ""}
    , {"foo.cpp", "foo", ""}
    , {"foo..cpp", "foo.", ""}
};

int main()
{
  using namespace fs;
  for (auto const & TC : TestCases) {
    path p(TC.value);
    assert(p == TC.value);
    path& Ref = (p.replace_extension(TC.extension));
    assert(p == TC.expect);
    assert(&Ref == &p);
  }
  for (auto const& TC : NoArgCases) {
    path p(TC.value);
    assert(p == TC.value);
    path& Ref = (p.replace_extension());
    assert(p == TC.expect);
    assert(&Ref == &p);
  }
}
