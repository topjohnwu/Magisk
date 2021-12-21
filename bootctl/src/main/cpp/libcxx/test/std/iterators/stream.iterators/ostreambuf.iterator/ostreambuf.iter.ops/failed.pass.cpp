//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class ostreambuf_iterator

// bool failed() const throw();

#include <iterator>
#include <sstream>
#include <cassert>

template <typename Char, typename Traits = std::char_traits<Char> >
struct my_streambuf : public std::basic_streambuf<Char,Traits> {
    typedef typename std::basic_streambuf<Char,Traits>::int_type  int_type;
    typedef typename std::basic_streambuf<Char,Traits>::char_type char_type;

    my_streambuf() {}
    int_type sputc(char_type) { return Traits::eof(); }
    };

int main()
{
    {
        my_streambuf<char> buf;
        std::ostreambuf_iterator<char> i(&buf);
        i = 'a';
        assert(i.failed());
    }
    {
        my_streambuf<wchar_t> buf;
        std::ostreambuf_iterator<wchar_t> i(&buf);
        i = L'a';
        assert(i.failed());
    }
}
