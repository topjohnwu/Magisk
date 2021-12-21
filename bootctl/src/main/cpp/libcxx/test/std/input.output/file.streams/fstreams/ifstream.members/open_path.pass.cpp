//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <fstream>

// template <class charT, class traits = char_traits<charT> >
// class basic_ifstream

// void open(const filesystem::path& s, ios_base::openmode mode = ios_base::in);

#include <fstream>
#include <filesystem>
#include <cassert>

int main() {
  {
    std::ifstream fs;
    assert(!fs.is_open());
    char c = 'a';
    fs >> c;
    assert(fs.fail());
    assert(c == 'a');
    fs.open(std::filesystem::path("test.dat"));
    assert(fs.is_open());
    fs >> c;
    assert(c == 'r');
  }
  {
    std::wifstream fs;
    assert(!fs.is_open());
    wchar_t c = L'a';
    fs >> c;
    assert(fs.fail());
    assert(c == L'a');
    fs.open(std::filesystem::path("test.dat"));
    assert(fs.is_open());
    fs >> c;
    assert(c == L'r');
  }
}
