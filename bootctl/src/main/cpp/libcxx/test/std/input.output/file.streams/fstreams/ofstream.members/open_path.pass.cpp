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

// plate <class charT, class traits = char_traits<charT> >
// class basic_ofstream

// void open(const filesystem::path& s, ios_base::openmode mode = ios_base::out);

#include <fstream>
#include <filesystem>
#include <cassert>
#include "platform_support.h"

namespace fs = std::filesystem;

int main() {
  fs::path p = get_temp_file_name();
  {
    std::ofstream fs;
    assert(!fs.is_open());
    char c = 'a';
    fs << c;
    assert(fs.fail());
    fs.open(p);
    assert(fs.is_open());
    fs << c;
  }
  {
    std::ifstream fs(p.c_str());
    char c = 0;
    fs >> c;
    assert(c == 'a');
  }
  std::remove(p.c_str());
  {
    std::wofstream fs;
    assert(!fs.is_open());
    wchar_t c = L'a';
    fs << c;
    assert(fs.fail());
    fs.open(p);
    assert(fs.is_open());
    fs << c;
  }
  {
    std::wifstream fs(p.c_str());
    wchar_t c = 0;
    fs >> c;
    assert(c == L'a');
  }
  std::remove(p.c_str());
}
