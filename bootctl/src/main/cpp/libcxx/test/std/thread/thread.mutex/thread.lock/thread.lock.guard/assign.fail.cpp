//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <mutex>

// template <class Mutex> class lock_guard;

// lock_guard& operator=(lock_guard const&) = delete;

#include <mutex>

int main()
{
    std::mutex m0;
    std::mutex m1;
    std::lock_guard<std::mutex> lg0(m0);
    std::lock_guard<std::mutex> lg(m1);
    lg = lg0;
}
