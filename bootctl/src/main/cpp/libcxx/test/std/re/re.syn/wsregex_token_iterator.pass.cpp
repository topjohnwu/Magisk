//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// typedef regex_token_iterator<wstring::const_iterator>   wsregex_token_iterator;

#include <regex>
#include <type_traits>
#include "test_macros.h"

int main()
{
    static_assert((std::is_same<std::regex_token_iterator<std::wstring::const_iterator>, std::wsregex_token_iterator>::value), "");
}
