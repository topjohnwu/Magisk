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

// FLAKY_TEST

// <mutex>

// template <class Mutex> class lock_guard;

// lock_guard(mutex_type& m, adopt_lock_t);

#include <mutex>
#include <thread>
#include <cstdlib>
#include <cassert>

std::mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

void f()
{
    time_point t0 = Clock::now();
    time_point t1;
    {
    m.lock();
    std::lock_guard<std::mutex> lg(m, std::adopt_lock);
    t1 = Clock::now();
    }
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
