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

// template <class charT, class traits>
//   void swap(basic_fstream<charT, traits>& x, basic_fstream<charT, traits>& y);

#include <fstream>
#include <utility>
#include <cassert>
#include "platform_support.h"


std::pair<std::string, std::string> get_temp_file_names() {
  std::pair<std::string, std::string> names;
  names.first = get_temp_file_name();

  // Create the file so the next call to `get_temp_file_name()` doesn't
  // return the same file.
  std::FILE *fd1 = std::fopen(names.first.c_str(), "w");

  names.second = get_temp_file_name();
  assert(names.first != names.second);

  std::fclose(fd1);
  std::remove(names.first.c_str());

  return names;
}

int main()
{
    std::pair<std::string, std::string> temp_files = get_temp_file_names();
    std::string& temp1 = temp_files.first;
    std::string& temp2 = temp_files.second;
    assert(temp1 != temp2);
    {
        std::fstream fs1(temp1.c_str(), std::ios_base::in | std::ios_base::out
                                                  | std::ios_base::trunc);
        std::fstream fs2(temp2.c_str(), std::ios_base::in | std::ios_base::out
                                                  | std::ios_base::trunc);
        fs1 << 1 << ' ' << 2;
        fs2 << 2 << ' ' << 1;
        fs1.seekg(0);
        swap(fs1, fs2);
        fs1.seekg(0);
        int i;
        fs1 >> i;
        assert(i == 2);
        fs1 >> i;
        assert(i == 1);
        i = 0;
        fs2 >> i;
        assert(i == 1);
        fs2 >> i;
        assert(i == 2);
    }
    std::remove(temp1.c_str());
    std::remove(temp2.c_str());
    {
        std::wfstream fs1(temp1.c_str(), std::ios_base::in | std::ios_base::out
                                                   | std::ios_base::trunc);
        std::wfstream fs2(temp2.c_str(), std::ios_base::in | std::ios_base::out
                                                   | std::ios_base::trunc);
        fs1 << 1 << ' ' << 2;
        fs2 << 2 << ' ' << 1;
        fs1.seekg(0);
        swap(fs1, fs2);
        fs1.seekg(0);
        int i;
        fs1 >> i;
        assert(i == 2);
        fs1 >> i;
        assert(i == 1);
        i = 0;
        fs2 >> i;
        assert(i == 1);
        fs2 >> i;
        assert(i == 2);
    }
    std::remove(temp1.c_str());
    std::remove(temp2.c_str());
}
