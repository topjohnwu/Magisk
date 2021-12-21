//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// pop_back() more than the number of elements in a vector

#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <cstdlib>
#include <vector>


int main() {
    std::vector<int> v;
    v.push_back(0);
    v.pop_back();
    v.pop_back();
    std::exit(1);
}
