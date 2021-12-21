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
// class basic_fstream

// void open(const filesystem::path& s, ios_base::openmode mode = ios_base::in|ios_base::out);

#include <fstream>
#include <filesystem>
#include <cassert>
#include "platform_support.h"

int main() {
  std::filesystem::path p = get_temp_file_name();
  {
    std::fstream stream;
    assert(!stream.is_open());
    stream.open(p,
                std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
    assert(stream.is_open());
    double x = 0;
    stream << 3.25;
    stream.seekg(0);
    stream >> x;
    assert(x == 3.25);
  }
  std::remove(p.c_str());
  {
    std::wfstream stream;
    assert(!stream.is_open());
    stream.open(p,
                std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
    assert(stream.is_open());
    double x = 0;
    stream << 3.25;
    stream.seekg(0);
    stream >> x;
    assert(x == 3.25);
  }
  std::remove(p.c_str());
}
