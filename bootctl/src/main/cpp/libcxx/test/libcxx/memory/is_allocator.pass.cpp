//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

// <memory>
// UNSUPPORTED: c++98, c++03, c++11, c++14

// template<typename _Alloc>
// struct __is_allocator;

// Is either true_type or false_type depending on if A is an allocator.

#include <memory>
#include <string>

#include "test_macros.h"
#include "min_allocator.h"
#include "test_allocator.h"

template <typename T>
void test_allocators()
{
	static_assert(!std::__is_allocator<T>::value, "" );
	static_assert( std::__is_allocator<std::allocator<T>>::value, "" );
	static_assert( std::__is_allocator<test_allocator<T>>::value, "" );
	static_assert( std::__is_allocator<min_allocator<T>>::value, "" );
}


int main()
{
//	test_allocators<void>();
	test_allocators<char>();
	test_allocators<int>();
	test_allocators<std::string>();
}
