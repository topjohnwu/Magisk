//===-------------------------- test_aux_runtime_op_array_new.cpp ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <iostream>
#include <cxxabi.h>

//  If the expression passed to operator new[] would result in an overflow, the
//  allocation function is not called, and a std::bad_array_new_length exception
//  is thrown instead (5.3.4p7).
bool bad_array_new_length_test() {
    try {
      // We test this directly because Clang does not currently codegen the
      // correct call to __cxa_bad_array_new_length, so this test would result
      // in passing -1 to ::operator new[], which would then throw a
      // std::bad_alloc, causing the test to fail.
      __cxxabiv1::__cxa_throw_bad_array_new_length();
    } catch ( const std::bad_array_new_length &banl ) {
      return true;
    }
    return false;
}

int main() {
    int ret_val = 0;

    if ( !bad_array_new_length_test ()) {
        std::cerr << "Bad array new length test failed!" << std::endl;
        ret_val = 1;
    }

    return ret_val;
}
