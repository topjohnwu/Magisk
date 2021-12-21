//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests workaround for  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64816.

#include <string>
#include "test_macros.h"

void f(const std::string &s) { TEST_IGNORE_NODISCARD s.begin(); }

#include <vector>

void AppendTo(const std::vector<char> &v) { TEST_IGNORE_NODISCARD v.begin(); }

int main() {}
