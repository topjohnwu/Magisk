//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// RUN: %build -O2
// RUN: %run

// <map>

// Previously this code caused a segfault when compiled at -O2 due to undefined
// behavior in __tree. See https://bugs.llvm.org/show_bug.cgi?id=28469

#include <functional>
#include <map>

void dummy() {}

struct F {
    std::map<int, std::function<void()> > m;
    F() { m[42] = &dummy; }
};

int main() {
    F f;
    f = F();
}
