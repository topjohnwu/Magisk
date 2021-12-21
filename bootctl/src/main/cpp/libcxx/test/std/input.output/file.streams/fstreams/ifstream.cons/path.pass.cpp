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

// explicit basic_ifstream(const filesystem::path& s,
//     ios_base::openmode mode = ios_base::in);

#include <fstream>
#include <filesystem>
#include <cassert>

namespace fs = std::filesystem;

int main() {
  {
    fs::path p;
    static_assert(!std::is_convertible<fs::path, std::ifstream>::value,
                  "ctor should be explicit");
    static_assert(std::is_constructible<std::ifstream, fs::path const&,
                                        std::ios_base::openmode>::value,
                  "");
  }
  {
    std::ifstream fs(fs::path("test.dat"));
    double x = 0;
    fs >> x;
    assert(x == 3.25);
  }
  // std::ifstream(const fs::path&, std::ios_base::openmode) is tested in
  // test/std/input.output/file.streams/fstreams/ofstream.cons/string.pass.cpp
  // which creates writable files.
  {
    std::wifstream fs(fs::path("test.dat"));
    double x = 0;
    fs >> x;
    assert(x == 3.25);
  }
  // std::wifstream(const fs::path&, std::ios_base::openmode) is tested in
  // test/std/input.output/file.streams/fstreams/ofstream.cons/string.pass.cpp
  // which creates writable files.
}
