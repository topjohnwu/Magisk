//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <condition_variable>

// class condition_variable;

// condition_variable();

#include <condition_variable>
#include <cassert>

int main()
{
    std::condition_variable cv;
}
