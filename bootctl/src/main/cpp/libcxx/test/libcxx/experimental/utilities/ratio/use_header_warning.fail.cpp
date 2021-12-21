//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: verify-support

// <experimental/ratio>

#include <experimental/ratio>

// expected-error@experimental/ratio:* {{"<experimental/ratio> has been removed. Use <ratio> instead."}}

int main() {}
