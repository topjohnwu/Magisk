//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


//  Assumption: minValue < maxValue
//  Assumption: minValue <= rhs <= maxValue
//  Assumption: minValue <= lhs <= maxValue
//  Assumption: minValue >= 0
template <typename T, T minValue, T maxValue>
T euclidian_addition(T rhs, T lhs)
{
    const T modulus = maxValue - minValue + 1;
    T ret = rhs + lhs;
    if (ret > maxValue)
        ret -= modulus;
    return ret;
}

//  Assumption: minValue < maxValue
//  Assumption: minValue <= rhs <= maxValue
//  Assumption: minValue <= lhs <= maxValue
//  Assumption: minValue >= 0
template <typename T, T minValue, T maxValue>
T euclidian_subtraction(T lhs, T rhs)
{
    const T modulus = maxValue - minValue + 1;
    T ret = lhs - rhs;
    if (ret < minValue)
        ret += modulus;
    if (ret > maxValue)     // this can happen if T is unsigned
        ret += modulus;
    return ret;
}
