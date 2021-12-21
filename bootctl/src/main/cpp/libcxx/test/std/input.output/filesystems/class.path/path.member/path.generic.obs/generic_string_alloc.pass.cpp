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

// template <class ECharT, class Traits = char_traits<ECharT>,
//           class Allocator = allocator<ECharT>>
// basic_string<ECharT, Traits, Allocator>
// generic_string(const Allocator& a = Allocator()) const;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "min_allocator.h"
#include "filesystem_test_helper.hpp"

MultiStringType longString = MKSTR("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/123456789/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");


// generic_string<C, T, A> forwards to string<C, T, A>. Tests for
// string<C, T, A>() are in "path.native.op/string_alloc.pass.cpp".
// generic_string is minimally tested here.
int main()
{
  using namespace fs;
  using CharT = wchar_t;
  using Traits = std::char_traits<CharT>;
  using Alloc = malloc_allocator<CharT>;
  using Str = std::basic_string<CharT, Traits, Alloc>;
  const wchar_t* expect = longString;
  const path p((const char*)longString);
  {
    DisableAllocationGuard g;
    Alloc a;
    Alloc::disable_default_constructor = true;
    Str s = p.generic_string<wchar_t, Traits, Alloc>(a);
    assert(s == expect);
    assert(Alloc::alloc_count > 0);
    assert(Alloc::outstanding_alloc() == 1);
  }
}
