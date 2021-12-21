//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: verify-support

// <experimental/numeric>

#include <experimental/numeric>

// expected-error@experimental/numeric:* {{"<experimental/numeric> has been removed. Use <numeric> instead."}}

int main() {}
