//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>


#include "filesystem_include.hpp"

using namespace fs;

struct ConvToPath {
  operator fs::path() const {
    return "";
  }
};

int main() {
  ConvToPath LHS, RHS;
  (void)(LHS == RHS); // expected-error {{invalid operands to binary expression}}
  (void)(LHS != RHS); // expected-error {{invalid operands to binary expression}}
  (void)(LHS < RHS); // expected-error {{invalid operands to binary expression}}
  (void)(LHS <= RHS); // expected-error {{invalid operands to binary expression}}
  (void)(LHS > RHS); // expected-error {{invalid operands to binary expression}}
  (void)(LHS >= RHS); // expected-error {{invalid operands to binary expression}}
}