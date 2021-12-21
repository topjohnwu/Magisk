//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: verify-support

// <experimental/system_error>

#include <experimental/system_error>

// expected-error@experimental/system_error:* {{"<experimental/system_error> has been removed. Use <system_error> instead."}}

int main() {}
