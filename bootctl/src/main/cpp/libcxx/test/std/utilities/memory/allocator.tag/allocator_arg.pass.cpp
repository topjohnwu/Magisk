//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// struct allocator_arg_t { };
// const allocator_arg_t allocator_arg = allocator_arg_t();

#include <memory>

void test(std::allocator_arg_t) {}

int main()
{
    test(std::allocator_arg);
}
