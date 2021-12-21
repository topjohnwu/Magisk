//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <streambuf>

// template <class charT, class traits = char_traits<charT> >
// class basic_streambuf;

// basic_streambuf(const basic_streambuf& rhs);  // protected

#include <streambuf>
#include <cassert>

std::streambuf &get();

int main()
{
    std::streambuf sb = get(); // expected-error {{calling a protected constructor}}
}
