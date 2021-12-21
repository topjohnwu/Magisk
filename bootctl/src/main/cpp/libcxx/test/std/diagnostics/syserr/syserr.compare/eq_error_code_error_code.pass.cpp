//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>

// bool operator==(const error_code&      lhs, const error_code&      rhs);
// bool operator==(const error_code&      lhs, const error_condition& rhs);
// bool operator==(const error_condition& lhs, const error_code&      rhs);
// bool operator==(const error_condition& lhs, const error_condition& rhs);
// bool operator!=(const error_code&      lhs, const error_code&      rhs);
// bool operator!=(const error_code&      lhs, const error_condition& rhs);
// bool operator!=(const error_condition& lhs, const error_code&      rhs);
// bool operator!=(const error_condition& lhs, const error_condition& rhs);

#include <system_error>
#include <cassert>

int main()
{
    std::error_code e_code1(5, std::generic_category());
    std::error_code e_code2(5, std::system_category());
    std::error_code e_code3(6, std::generic_category());
    std::error_code e_code4(6, std::system_category());
    std::error_condition e_condition1(5, std::generic_category());
    std::error_condition e_condition2(5, std::system_category());
    std::error_condition e_condition3(6, std::generic_category());
    std::error_condition e_condition4(6, std::system_category());

    assert(e_code1 == e_code1);
    assert(e_code1 != e_code2);
    assert(e_code1 != e_code3);
    assert(e_code1 != e_code4);
    assert(e_code1 == e_condition1);
    assert(e_code1 != e_condition2);
    assert(e_code1 != e_condition3);
    assert(e_code1 != e_condition4);

    assert(e_code2 != e_code1);
    assert(e_code2 == e_code2);
    assert(e_code2 != e_code3);
    assert(e_code2 != e_code4);
    assert(e_code2 == e_condition1);  // ?
    assert(e_code2 == e_condition2);
    assert(e_code2 != e_condition3);
    assert(e_code2 != e_condition4);

    assert(e_code3 != e_code1);
    assert(e_code3 != e_code2);
    assert(e_code3 == e_code3);
    assert(e_code3 != e_code4);
    assert(e_code3 != e_condition1);
    assert(e_code3 != e_condition2);
    assert(e_code3 == e_condition3);
    assert(e_code3 != e_condition4);

    assert(e_code4 != e_code1);
    assert(e_code4 != e_code2);
    assert(e_code4 != e_code3);
    assert(e_code4 == e_code4);
    assert(e_code4 != e_condition1);
    assert(e_code4 != e_condition2);
    assert(e_code4 == e_condition3);  // ?
    assert(e_code4 == e_condition4);

    assert(e_condition1 == e_code1);
    assert(e_condition1 == e_code2);  // ?
    assert(e_condition1 != e_code3);
    assert(e_condition1 != e_code4);
    assert(e_condition1 == e_condition1);
    assert(e_condition1 != e_condition2);
    assert(e_condition1 != e_condition3);
    assert(e_condition1 != e_condition4);

    assert(e_condition2 != e_code1);
    assert(e_condition2 == e_code2);
    assert(e_condition2 != e_code3);
    assert(e_condition2 != e_code4);
    assert(e_condition2 != e_condition1);
    assert(e_condition2 == e_condition2);
    assert(e_condition2 != e_condition3);
    assert(e_condition2 != e_condition4);

    assert(e_condition3 != e_code1);
    assert(e_condition3 != e_code2);
    assert(e_condition3 == e_code3);
    assert(e_condition3 == e_code4);  // ?
    assert(e_condition3 != e_condition1);
    assert(e_condition3 != e_condition2);
    assert(e_condition3 == e_condition3);
    assert(e_condition3 != e_condition4);

    assert(e_condition4 != e_code1);
    assert(e_condition4 != e_code2);
    assert(e_condition4 != e_code3);
    assert(e_condition4 == e_code4);
    assert(e_condition4 != e_condition1);
    assert(e_condition4 != e_condition2);
    assert(e_condition4 != e_condition3);
    assert(e_condition4 == e_condition4);
}
