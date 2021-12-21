//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-W#warnings"
#endif

#define min THIS IS A NASTY MACRO!
#define max THIS IS A NASTY MACRO!

#include <map>

int main() {
  std::map<int, int> m;
  ((void)m);
}
