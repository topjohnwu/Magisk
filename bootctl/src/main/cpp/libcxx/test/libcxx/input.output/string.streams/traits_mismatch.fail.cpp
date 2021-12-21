//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <sstream>

//  template<class charT, class traits = char_traits<charT>,
//           class Allocator = allocator<charT>>
//    class basic_stringbuf;
//
// The char type of the stream and the char_type of the traits have to match

#include <sstream>

int main()
{
	std::basic_stringbuf<char, std::char_traits<wchar_t> > sb;
//  expected-error-re@streambuf:* {{static_assert failed{{.*}} "traits_type::char_type must be the same type as CharT"}}
//  expected-error-re@string:* {{static_assert failed{{.*}} "traits_type::char_type must be the same type as CharT"}}
}

