//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

// UNSUPPORTED: c++98, c++03

//  Tuples of smart pointers; based on bug #18350
//  auto_ptr doesn't have a copy constructor that takes a const &, but tuple does.

#include <tuple>
#include <memory>

int main () {
    {
    std::tuple<std::unique_ptr<char>> up;
    std::tuple<std::shared_ptr<char>> sp;
    std::tuple<std::weak_ptr  <char>> wp;
    }
    {
    std::tuple<std::unique_ptr<char[]>> up;
    std::tuple<std::shared_ptr<char[]>> sp;
    std::tuple<std::weak_ptr  <char[]>> wp;
    }
    // Smart pointers of type 'T[N]' are not tested here since they are not
    // supported by the standard nor by libc++'s implementation.
    // See https://reviews.llvm.org/D21320 for more information.
}
