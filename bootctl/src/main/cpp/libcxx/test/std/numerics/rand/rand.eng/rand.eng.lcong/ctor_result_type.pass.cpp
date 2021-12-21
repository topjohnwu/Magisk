//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template <class UIntType, UIntType a, UIntType c, UIntType m>
//   class linear_congruential_engine;

// explicit linear_congruential_engine(result_type s = default_seed);

#include <random>
#include <sstream>
#include <cassert>

template <class T>
void
test1()
{
    // c % m != 0 && s % m != 0
    {
        typedef std::linear_congruential_engine<T, 2, 3, 7> E;
        E e(5);
        std::ostringstream os;
        os << e;
        assert(os.str() == "5");
    }
    {
        typedef std::linear_congruential_engine<T, 2, 3, 0> E;
        E e(5);
        std::ostringstream os;
        os << e;
        assert(os.str() == "5");
    }
    {
        typedef std::linear_congruential_engine<T, 2, 3, 4> E;
        E e(5);
        std::ostringstream os;
        os << e;
        assert(os.str() == "1");
    }
}

template <class T>
void
test2()
{
    // c % m != 0 && s % m == 0
    {
        typedef std::linear_congruential_engine<T, 2, 3, 7> E;
        E e(7);
        std::ostringstream os;
        os << e;
        assert(os.str() == "0");
    }
    {
        typedef std::linear_congruential_engine<T, 2, 3, 0> E;
        E e(0);
        std::ostringstream os;
        os << e;
        assert(os.str() == "0");
    }
    {
        typedef std::linear_congruential_engine<T, 2, 3, 4> E;
        E e(4);
        std::ostringstream os;
        os << e;
        assert(os.str() == "0");
    }
}

template <class T>
void
test3()
{
    // c % m == 0 && s % m != 0
    {
        typedef std::linear_congruential_engine<T, 2, 0, 7> E;
        E e(3);
        std::ostringstream os;
        os << e;
        assert(os.str() == "3");
    }
    {
        typedef std::linear_congruential_engine<T, 2, 0, 0> E;
        E e(5);
        std::ostringstream os;
        os << e;
        assert(os.str() == "5");
    }
    {
        typedef std::linear_congruential_engine<T, 2, 0, 4> E;
        E e(7);
        std::ostringstream os;
        os << e;
        assert(os.str() == "3");
    }
}

template <class T>
void
test4()
{
    // c % m == 0 && s % m == 0
    {
        typedef std::linear_congruential_engine<T, 2, 0, 7> E;
        E e(7);
        std::ostringstream os;
        os << e;
        assert(os.str() == "1");
    }
    {
        typedef std::linear_congruential_engine<T, 2, 0, 0> E;
        E e(0);
        std::ostringstream os;
        os << e;
        assert(os.str() == "1");
    }
    {
        typedef std::linear_congruential_engine<T, 2, 0, 4> E;
        E e(8);
        std::ostringstream os;
        os << e;
        assert(os.str() == "1");
    }
}

int main()
{
    test1<unsigned short>();
    test1<unsigned int>();
    test1<unsigned long>();
    test1<unsigned long long>();

    test2<unsigned short>();
    test2<unsigned int>();
    test2<unsigned long>();
    test2<unsigned long long>();

    test3<unsigned short>();
    test3<unsigned int>();
    test3<unsigned long>();
    test3<unsigned long long>();

    test4<unsigned short>();
    test4<unsigned int>();
    test4<unsigned long>();
    test4<unsigned long long>();
}
