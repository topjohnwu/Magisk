//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <experimental/filesystem>

// namespace std::experimental::filesystem::v1

#include <experimental/filesystem>
#include <type_traits>

int main() {
  static_assert(std::is_same<
          std::experimental::filesystem::path,
          std::experimental::filesystem::v1::path
      >::value, "");
}
