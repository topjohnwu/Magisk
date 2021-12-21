//===----------------------------------------------------------------------===////
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===////

#ifndef TIMER_HPP
#define TIMER_HPP

// Define LIBCXXABI_NO_TIMER to disable testing with a timer.
#ifndef LIBCXXABI_NO_TIMER

#include <chrono>
#include <iostream>

class timer
{
    typedef std::chrono::high_resolution_clock Clock;
    typedef Clock::time_point TimePoint;
    typedef std::chrono::microseconds MicroSeconds;
public:
    timer() : m_start(Clock::now()) {}

    timer(timer const &) = delete;
    timer & operator=(timer const &) = delete;

    ~timer()
    {
        using std::chrono::duration_cast;
        TimePoint end = Clock::now();
        MicroSeconds us = duration_cast<MicroSeconds>(end - m_start);
        std::cout << us.count() << " microseconds\n";
    }

private:
    TimePoint m_start;
};

#else /* LIBCXXABI_NO_TIMER */

class timer
{
public:
    timer() {}
    timer(timer const &) = delete;
    timer & operator=(timer const &) = delete;
    ~timer() {}
};

#endif /* LIBCXXABI_NO_TIMER */

#endif /* TIMER_HPP */
