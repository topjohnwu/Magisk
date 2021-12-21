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

// This test uses the POSIX header <sys/time.h> which Windows doesn't provide
// UNSUPPORTED: windows

// This test depends on signal behaviour until r210210, so some system libs
// don't pass.
//
// XFAIL: with_system_cxx_lib=macosx10.11
// XFAIL: with_system_cxx_lib=macosx10.10
// XFAIL: with_system_cxx_lib=macosx10.9
// XFAIL: with_system_cxx_lib=macosx10.8
// XFAIL: with_system_cxx_lib=macosx10.7

// <thread>

// template <class Rep, class Period>
//   void sleep_for(const chrono::duration<Rep, Period>& rel_time);

#include <thread>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <signal.h>
#include <sys/time.h>

void sig_action(int) {}

int main()
{
    int ec;
    struct sigaction action;
    action.sa_handler = &sig_action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    ec = sigaction(SIGALRM, &action, nullptr);
    assert(!ec);

    struct itimerval it;
    std::memset(&it, 0, sizeof(itimerval));
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 250000;
    // This will result in a SIGALRM getting fired resulting in the nanosleep
    // inside sleep_for getting EINTR.
    ec = setitimer(ITIMER_REAL, &it, nullptr);
    assert(!ec);

    typedef std::chrono::system_clock Clock;
    typedef Clock::time_point time_point;
    std::chrono::milliseconds ms(500);
    time_point t0 = Clock::now();
    std::this_thread::sleep_for(ms);
    time_point t1 = Clock::now();
    std::chrono::nanoseconds ns = (t1 - t0) - ms;
    std::chrono::nanoseconds err = 5 * ms / 100;
    // The time slept is within 5% of 500ms
    assert(std::abs(ns.count()) < err.count());
}
