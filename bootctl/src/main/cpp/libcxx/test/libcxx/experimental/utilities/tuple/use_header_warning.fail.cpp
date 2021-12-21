//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: verify-support

// <experimental/tuple>

#include <experimental/tuple>

// expected-error@experimental/tuple:* {{"<experimental/tuple> has been removed. Use <tuple> instead."}}

int main() {}
