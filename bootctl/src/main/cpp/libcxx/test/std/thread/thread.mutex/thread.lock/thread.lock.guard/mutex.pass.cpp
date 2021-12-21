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

// explicit lock_guard(mutex_type& m);

// template<class _Mutex> lock_guard(lock_guard<_Mutex>)
//     -> lock_guard<_Mutex>;  // C++17

#include <mutex>
#include <thread>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

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
    std::lock_guard<std::mutex> lg(m);
    t1 = Clock::now();
    }
    ns d = t1 - t0 - ms(250);
    assert(d < ms(200));  // within 200ms
}

int main()
{
    m.lock();
    std::thread t(f);
    std::this_thread::sleep_for(ms(250));
    m.unlock();
    t.join();

#ifdef __cpp_deduction_guides
    std::lock_guard lg(m);
    static_assert((std::is_same<decltype(lg), std::lock_guard<decltype(m)>>::value), "" );
#endif
}
