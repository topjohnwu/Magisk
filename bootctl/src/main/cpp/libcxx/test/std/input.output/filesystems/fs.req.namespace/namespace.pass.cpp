//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <filesystem>

// namespace std::filesystem

#include <filesystem>
#include <type_traits>

using namespace std::filesystem;

int main() {
  static_assert(std::is_same<
          path,
          std::filesystem::path
      >::value, "");
}
