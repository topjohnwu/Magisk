//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <thread>

// class thread
//     template <class _Fp, class ..._Args,
//         explicit thread(_Fp&& __f, _Args&&... __args);
//  This constructor shall not participate in overload resolution
//       if decay<F>::type is the same type as std::thread.


#include <thread>
#include <cassert>

int main()
{
    volatile std::thread t1;
    std::thread t2 ( t1, 1, 2.0 );
}
