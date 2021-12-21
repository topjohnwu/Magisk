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
    auto up1 = std::make_unique<std::string[]>("error"); // doesn't compile - no bound
}
