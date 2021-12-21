//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: c++98 || c++03 || c++11 || c++14

// <filesystem>

// namespace std::filesystem

#include <filesystem>
#include "test_macros.h"

using namespace std::filesystem;

#if TEST_STD_VER >= 11
// expected-error@-3 {{no namespace named 'filesystem' in namespace 'std';}}
#else
// expected-error@-5 {{expected namespace name}}
#endif

int main() {

}
