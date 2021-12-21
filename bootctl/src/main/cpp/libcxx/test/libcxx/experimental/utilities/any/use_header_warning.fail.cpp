//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: verify-support

// <experimental/any>

#include <experimental/any>

// expected-error@experimental/any:* {{"<experimental/any> has been removed. Use <any> instead."}}

int main() {}
