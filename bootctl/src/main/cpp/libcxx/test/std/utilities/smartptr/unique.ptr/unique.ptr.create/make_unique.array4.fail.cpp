//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <memory>
#include <string>
#include <cassert>

int main()
{
    auto up4 = std::make_unique<int[5]>(11, 22, 33, 44, 55); // deleted
}
