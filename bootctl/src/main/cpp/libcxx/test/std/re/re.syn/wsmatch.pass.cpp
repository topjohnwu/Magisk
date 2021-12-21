//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// typedef match_results<wstring::const_iterator>   wsmatch;

#include <regex>
#include <type_traits>
#include "test_macros.h"

int main()
{
    static_assert((std::is_same<std::match_results<std::wstring::const_iterator>, std::wsmatch>::value), "");
}
