//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: verify-support

// <experimental/chrono>

#include <experimental/chrono>

// expected-error@experimental/chrono:* {{"<experimental/chrono> has been removed. Use <chrono> instead."}}

int main() {}
