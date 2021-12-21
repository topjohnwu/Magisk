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

// close();

//	Inspired by PR#38052 - std::fstream still good after closing and updating content

#include <fstream>
#include <cassert>
#include "platform_support.h"

int main()
{
    std::string temp = get_temp_file_name();

    std::fstream ofs(temp, std::ios::out | std::ios::trunc);
    ofs << "Hello, World!\n";
    assert( ofs.good());
    ofs.close();
    assert( ofs.good());
    ofs << "Hello, World!\n";
    assert(!ofs.good());

    std::remove(temp.c_str());
}
