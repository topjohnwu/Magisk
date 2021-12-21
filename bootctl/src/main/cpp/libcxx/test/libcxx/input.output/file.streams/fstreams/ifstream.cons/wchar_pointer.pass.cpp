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
// class basic_ifstream

// explicit basic_ifstream(const wchar_t* s, ios_base::openmode mode = ios_base::in);

#include <fstream>
#include <cassert>

int main()
{
#ifdef _LIBCPP_HAS_OPEN_WITH_WCHAR
    {
        std::ifstream fs(L"test.dat");
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    // std::ifstream(const wchar_t*, std::ios_base::openmode) is tested in
    // test/libcxx/input.output/file.streams/fstreams/ofstream.cons/wchar_pointer.pass.cpp
    // which creates writable files.
    {
        std::wifstream fs(L"test.dat");
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    // std::wifstream(const wchar_t*, std::ios_base::openmode) is tested in
    // test/libcxx/input.output/file.streams/fstreams/ofstream.cons/wchar_pointer.pass.cpp
    // which creates writable files.
#endif
}
