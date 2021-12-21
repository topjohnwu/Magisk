//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <fstream>

// template<class charT, class traits = char_traits<charT>>
//   class basic_filebuf;
//
// The char type of the stream and the char_type of the traits have to match

#include <fstream>

int main()
{
	std::basic_filebuf<char, std::char_traits<wchar_t> > f;
//  expected-error-re@streambuf:* {{static_assert failed{{.*}} "traits_type::char_type must be the same type as CharT"}}
}

