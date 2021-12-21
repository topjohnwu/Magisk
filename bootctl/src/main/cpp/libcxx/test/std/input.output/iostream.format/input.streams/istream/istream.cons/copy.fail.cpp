//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <istream>

// template <class charT, class traits = char_traits<charT> >
// class basic_istream;

// basic_istream(basic_istream const& rhs) = delete;
// basic_istream& operator=(basic_istream const&) = delete;

#include <istream>
#include <type_traits>
#include <cassert>

struct test_istream
    : public std::basic_istream<char>
{
    typedef std::basic_istream<char> base;

    test_istream(test_istream&& s)
        : base(std::move(s)) // OK
    {
    }

    test_istream& operator=(test_istream&& s) {
      base::operator=(std::move(s)); // OK
      return *this;
    }

    test_istream(test_istream const& s)
        : base(s) // expected-error {{call to deleted constructor of 'std::basic_istream<char>'}}
    {
    }

    test_istream& operator=(test_istream const& s) {
      base::operator=(s); // expected-error {{call to deleted member function 'operator='}}
      return *this;
    }

};


int main()
{

}
