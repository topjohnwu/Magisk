//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// string to_string(int val);
// string to_string(unsigned val);
// string to_string(long val);
// string to_string(unsigned long val);
// string to_string(long long val);
// string to_string(unsigned long long val);
// string to_string(float val);
// string to_string(double val);
// string to_string(long double val);

#include <limits>
#include <string>
#include <cassert>
#include <sstream>

template <class T>
void
test_signed()
{
    {
        std::string s = std::to_string(T(0));
        assert(s.size() == 1);
        assert(s[s.size()] == 0);
        assert(s == "0");
    }
    {
        std::string s = std::to_string(T(12345));
        assert(s.size() == 5);
        assert(s[s.size()] == 0);
        assert(s == "12345");
    }
    {
        std::string s = std::to_string(T(-12345));
        assert(s.size() == 6);
        assert(s[s.size()] == 0);
        assert(s == "-12345");
    }
    {
        std::string s = std::to_string(std::numeric_limits<T>::max());
        assert(s.size() == std::numeric_limits<T>::digits10 + 1);
        std::istringstream is(s);
        T t(0);
        is >> t;
        assert(t == std::numeric_limits<T>::max());
    }
    {
        std::string s = std::to_string(std::numeric_limits<T>::min());
        std::istringstream is(s);
        T t(0);
        is >> t;
        assert(t == std::numeric_limits<T>::min());
    }
}

template <class T>
void
test_unsigned()
{
    {
        std::string s = std::to_string(T(0));
        assert(s.size() == 1);
        assert(s[s.size()] == 0);
        assert(s == "0");
    }
    {
        std::string s = std::to_string(T(12345));
        assert(s.size() == 5);
        assert(s[s.size()] == 0);
        assert(s == "12345");
    }
    {
        std::string s = std::to_string(std::numeric_limits<T>::max());
        assert(s.size() == std::numeric_limits<T>::digits10 + 1);
        std::istringstream is(s);
        T t(0);
        is >> t;
        assert(t == std::numeric_limits<T>::max());
    }
}

template <class T>
void
test_float()
{
    {
        std::string s = std::to_string(T(0));
        assert(s.size() == 8);
        assert(s[s.size()] == 0);
        assert(s == "0.000000");
    }
    {
        std::string s = std::to_string(T(12345));
        assert(s.size() == 12);
        assert(s[s.size()] == 0);
        assert(s == "12345.000000");
    }
    {
        std::string s = std::to_string(T(-12345));
        assert(s.size() == 13);
        assert(s[s.size()] == 0);
        assert(s == "-12345.000000");
    }
}

int main()
{
    test_signed<int>();
    test_signed<long>();
    test_signed<long long>();
    test_unsigned<unsigned>();
    test_unsigned<unsigned long>();
    test_unsigned<unsigned long long>();
    test_float<float>();
    test_float<double>();
    test_float<long double>();
}
