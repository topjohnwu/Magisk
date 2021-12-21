//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: verify-support

// <experimental/string_view>

#include <experimental/string_view>

// expected-error@experimental/string_view:* {{"<experimental/string_view> has been removed. Use <string_view> instead."}}

int main() {}
