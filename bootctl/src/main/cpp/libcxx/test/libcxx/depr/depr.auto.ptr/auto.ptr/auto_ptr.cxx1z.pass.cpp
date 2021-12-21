//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class X>
// class auto_ptr;
//
//  In C++17, auto_ptr has been removed.
//  However, for backwards compatibility, if _LIBCPP_NO_REMOVE_AUTOPTR
//  is defined before including <memory>, then auto_ptr will be restored.

// MODULES_DEFINES: _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

#include <memory>
#include <type_traits>

int main()
{
    std::auto_ptr<int> p;
}
