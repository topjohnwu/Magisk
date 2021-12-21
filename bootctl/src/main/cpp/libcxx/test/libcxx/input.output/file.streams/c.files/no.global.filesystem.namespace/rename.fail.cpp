//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: libcpp-has-no-global-filesystem-namespace

#include <cstdio>

int main() {
    // rename is not available on systems without a global filesystem namespace.
    std::rename("", "");
}
