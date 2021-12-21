//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iosfwd>

#include <iosfwd>
#include <cwchar>  // for mbstate_t

template <class Ptr> void test()
{
    Ptr p = 0;
    ((void)p); // Prevent unused warning
}

int main()
{
    test<std::char_traits<char>*          >();
    test<std::char_traits<wchar_t>*       >();
    test<std::char_traits<unsigned short>*>();

    test<std::basic_ios<char>*          >();
    test<std::basic_ios<wchar_t>*       >();
    test<std::basic_ios<unsigned short>*>();

    test<std::basic_streambuf<char>*          >();
    test<std::basic_streambuf<wchar_t>*       >();
    test<std::basic_streambuf<unsigned short>*>();

    test<std::basic_istream<char>*          >();
    test<std::basic_istream<wchar_t>*       >();
    test<std::basic_istream<unsigned short>*>();

    test<std::basic_ostream<char>*          >();
    test<std::basic_ostream<wchar_t>*       >();
    test<std::basic_ostream<unsigned short>*>();

    test<std::basic_iostream<char>*          >();
    test<std::basic_iostream<wchar_t>*       >();
    test<std::basic_iostream<unsigned short>*>();

    test<std::basic_stringbuf<char>*          >();
    test<std::basic_stringbuf<wchar_t>*       >();
    test<std::basic_stringbuf<unsigned short>*>();

    test<std::basic_istringstream<char>*          >();
    test<std::basic_istringstream<wchar_t>*       >();
    test<std::basic_istringstream<unsigned short>*>();

    test<std::basic_ostringstream<char>*          >();
    test<std::basic_ostringstream<wchar_t>*       >();
    test<std::basic_ostringstream<unsigned short>*>();

    test<std::basic_stringstream<char>*          >();
    test<std::basic_stringstream<wchar_t>*       >();
    test<std::basic_stringstream<unsigned short>*>();

    test<std::basic_filebuf<char>*          >();
    test<std::basic_filebuf<wchar_t>*       >();
    test<std::basic_filebuf<unsigned short>*>();

    test<std::basic_ifstream<char>*          >();
    test<std::basic_ifstream<wchar_t>*       >();
    test<std::basic_ifstream<unsigned short>*>();

    test<std::basic_ofstream<char>*          >();
    test<std::basic_ofstream<wchar_t>*       >();
    test<std::basic_ofstream<unsigned short>*>();

    test<std::basic_fstream<char>*          >();
    test<std::basic_fstream<wchar_t>*       >();
    test<std::basic_fstream<unsigned short>*>();

    test<std::istreambuf_iterator<char>*          >();
    test<std::istreambuf_iterator<wchar_t>*       >();
    test<std::istreambuf_iterator<unsigned short>*>();

    test<std::ostreambuf_iterator<char>*          >();
    test<std::ostreambuf_iterator<wchar_t>*       >();
    test<std::ostreambuf_iterator<unsigned short>*>();

    test<std::ios* >();
    test<std::wios*>();

    test<std::streambuf*>();
    test<std::istream*  >();
    test<std::ostream*  >();
    test<std::iostream* >();

    test<std::stringbuf*    >();
    test<std::istringstream*>();
    test<std::ostringstream*>();
    test<std::stringstream* >();

    test<std::filebuf* >();
    test<std::ifstream*>();
    test<std::ofstream*>();
    test<std::fstream* >();

    test<std::wstreambuf*>();
    test<std::wistream*  >();
    test<std::wostream*  >();
    test<std::wiostream* >();

    test<std::wstringbuf*    >();
    test<std::wistringstream*>();
    test<std::wostringstream*>();
    test<std::wstringstream* >();

    test<std::wfilebuf* >();
    test<std::wifstream*>();
    test<std::wofstream*>();
    test<std::wfstream* >();

    test<std::fpos<std::mbstate_t>*>();
    test<std::streampos*           >();
    test<std::wstreampos*          >();
}
