//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

#include <queue>
#include <cassert>
#include <type_traits>

int main()
{
//  LWG#2566 says that the first template param must match the second one's value type
    std::queue<double, std::deque<int>> t;
}
