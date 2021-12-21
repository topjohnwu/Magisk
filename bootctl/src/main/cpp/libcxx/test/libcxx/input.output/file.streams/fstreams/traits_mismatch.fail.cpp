//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <fstream>

// template <class charT, class traits = char_traits<charT> >
// class basic_fstream

// The char type of the stream and the char_type of the traits have to match

#include <fstream>

int main()
{
	std::basic_fstream<char, std::char_traits<wchar_t> > f;
//  expected-error-re@ios:* {{static_assert failed{{.*}} "traits_type::char_type must be the same type as CharT"}}
//  expected-error-re@streambuf:* {{static_assert failed{{.*}} "traits_type::char_type must be the same type as CharT"}}

// FIXME: As of commit r324062 Clang incorrectly generates a diagnostic about mismatching
// exception specifications for types which are already invalid for one reason or another.
// For now we tolerate this diagnostic.
// expected-error@ostream:* 0-1 {{exception specification of overriding function is more lax than base version}}
}

