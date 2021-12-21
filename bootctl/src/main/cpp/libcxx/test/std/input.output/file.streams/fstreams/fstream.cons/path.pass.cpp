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

// explicit basic_fstream(const filesystem::path& s,
//     ios_base::openmode mode = ios_base::in|ios_base::out);

#include <fstream>
#include <filesystem>
#include <cassert>
#include "platform_support.h"

namespace fs = std::filesystem;

int main() {
  fs::path p = get_temp_file_name();
  {
    std::fstream fs(p, std::ios_base::in | std::ios_base::out |
                           std::ios_base::trunc);
    double x = 0;
    fs << 3.25;
    fs.seekg(0);
    fs >> x;
    assert(x == 3.25);
  }
  std::remove(p.c_str());
  {
    std::wfstream fs(p, std::ios_base::in | std::ios_base::out |
                            std::ios_base::trunc);
    double x = 0;
    fs << 3.25;
    fs.seekg(0);
    fs >> x;
    assert(x == 3.25);
  }
  std::remove(p.c_str());
}
