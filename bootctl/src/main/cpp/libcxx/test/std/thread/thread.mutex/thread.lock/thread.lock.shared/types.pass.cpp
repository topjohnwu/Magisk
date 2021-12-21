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
// UNSUPPORTED: c++98, c++03, c++11


// <shared_mutex>

// template <class Mutex>
// class shared_lock
// {
// public:
//     typedef Mutex mutex_type;
//     ...
// };

#include <shared_mutex>
#include <type_traits>

int main()
{
    static_assert((std::is_same<std::shared_lock<std::mutex>::mutex_type,
                   std::mutex>::value), "");
}
