//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: verify-support

// <experimental/optional>

#include <experimental/optional>

// expected-error@experimental/optional:* {{"<experimental/optional> has been removed. Use <optional> instead."}}

int main() {}
