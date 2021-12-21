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

// lock_guard(lock_guard const&) = delete;

#include <mutex>

int main()
{
    std::mutex m;
    std::lock_guard<std::mutex> lg0(m);
    std::lock_guard<std::mutex> lg(lg0);
}
