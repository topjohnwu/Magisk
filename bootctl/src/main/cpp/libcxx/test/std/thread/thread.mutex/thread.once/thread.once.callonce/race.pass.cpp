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

// <mutex>

// struct once_flag;

// template<class Callable, class ...Args>
//   void call_once(once_flag& flag, Callable&& func, Args&&... args);

// This test is supposed to be run with ThreadSanitizer and verifies that
// call_once properly synchronizes user state, a data race that was fixed
// in r280621.

#include <mutex>
#include <thread>
#include <cassert>

std::once_flag flg0;
long global = 0;

void init0()
{
    ++global;
}

void f0()
{
    std::call_once(flg0, init0);
    assert(global == 1);
}

int main()
{
    std::thread t0(f0);
    std::thread t1(f0);
    t0.join();
    t1.join();
    assert(global == 1);
}
