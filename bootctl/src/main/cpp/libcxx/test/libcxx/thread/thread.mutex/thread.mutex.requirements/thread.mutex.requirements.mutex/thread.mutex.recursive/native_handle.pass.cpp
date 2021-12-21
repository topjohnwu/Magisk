//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads, libcpp-has-thread-api-external

// XFAIL: windows

// <mutex>

// class recursive_mutex;

// typedef pthread_mutex_t* native_handle_type;
// native_handle_type native_handle();

#include <mutex>
#include <cassert>

int main()
{
    std::recursive_mutex m;
    pthread_mutex_t* h = m.native_handle();
    assert(h);
}
