//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <csignal>

#include <csignal>
#include <type_traits>

#ifndef SIG_DFL
#error SIG_DFL not defined
#endif

#ifndef SIG_ERR
#error SIG_ERR not defined
#endif

#ifndef SIG_IGN
#error SIG_IGN not defined
#endif

#ifndef SIGABRT
#error SIGABRT not defined
#endif

#ifndef SIGFPE
#error SIGFPE not defined
#endif

#ifndef SIGILL
#error SIGILL not defined
#endif

#ifndef SIGINT
#error SIGINT not defined
#endif

#ifndef SIGSEGV
#error SIGSEGV not defined
#endif

#ifndef SIGTERM
#error SIGTERM not defined
#endif

int main()
{
    std::sig_atomic_t sig = 0;
    ((void)sig);
    typedef void (*func)(int);
    static_assert((std::is_same<decltype(std::signal(0, (func)0)), func>::value), "");
    static_assert((std::is_same<decltype(std::raise(0)), int>::value), "");
}
