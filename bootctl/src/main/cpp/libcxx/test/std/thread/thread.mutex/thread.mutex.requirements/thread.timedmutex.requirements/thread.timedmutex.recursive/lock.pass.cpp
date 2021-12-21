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

// class recursive_timed_mutex;

// void lock();

#include <mutex>
#include <thread>
#include <cstdlib>
#include <cassert>

#include <iostream>

std::recursive_timed_mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

void f()
{
    time_point t0 = Clock::now();
    m.lock();
    time_point t1 = Clock::now();
    m.lock();
    m.unlock();
    m.unlock();
    ns d = t1 - t0 - ms(250);
    assert(d < ms(50));  // within 50ms
}

int main()
{
    m.lock();
    std::thread t(f);
    std::this_thread::sleep_for(ms(250));
    m.unlock();
    t.join();
}
