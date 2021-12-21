//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Class bad_function_call

// class bad_function_call
//     : public exception
// {
// public:
//   // 20.7.16.1.1, constructor:
//   bad_function_call();
// };

#include <functional>
#include <type_traits>

int main()
{
    static_assert((std::is_base_of<std::exception, std::bad_function_call>::value), "");
}
