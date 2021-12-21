//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <functional>

// template<CopyConstructible Fn, CopyConstructible... Types>
//   unspecified bind(Fn, Types...);
// template<Returnable R, CopyConstructible Fn, CopyConstructible... Types>
//   unspecified bind(Fn, Types...);

#include <stdio.h>

#include <functional>
#include <cassert>

int count = 0;

// 1 arg, return void

void f_void_1(int i)
{
    count += i;
}

struct A_void_1
{
    void operator()(int i)
    {
        count += i;
    }

    void mem1() {++count;}
    void mem2() const {count += 2;}
};

void
test_void_1()
{
    using namespace std::placeholders;
    int save_count = count;
    // function
    {
    std::bind(f_void_1, _1)(2);
    assert(count == save_count + 2);
    save_count = count;
    }
    {
    std::bind(f_void_1, 2)();
    assert(count == save_count + 2);
    save_count = count;
    }
    // function pointer
    {
    void (*fp)(int) = f_void_1;
    std::bind(fp, _1)(3);
    assert(count == save_count+3);
    save_count = count;
    }
    {
    void (*fp)(int) = f_void_1;
    std::bind(fp, 3)();
    assert(count == save_count+3);
    save_count = count;
    }
    // functor
    {
    A_void_1 a0;
    std::bind(a0, _1)(4);
    assert(count == save_count+4);
    save_count = count;
    }
    {
    A_void_1 a0;
    std::bind(a0, 4)();
    assert(count == save_count+4);
    save_count = count;
    }
    // member function pointer
    {
    void (A_void_1::*fp)() = &A_void_1::mem1;
    std::bind(fp, _1)(A_void_1());
    assert(count == save_count+1);
    save_count = count;
    A_void_1 a;
    std::bind(fp, _1)(&a);
    assert(count == save_count+1);
    save_count = count;
    }
    {
    void (A_void_1::*fp)() = &A_void_1::mem1;
    std::bind(fp, A_void_1())();
    assert(count == save_count+1);
    save_count = count;
    A_void_1 a;
    std::bind(fp, &a)();
    assert(count == save_count+1);
    save_count = count;
    }
    // const member function pointer
    {
    void (A_void_1::*fp)() const = &A_void_1::mem2;
    std::bind(fp, _1)(A_void_1());
    assert(count == save_count+2);
    save_count = count;
    A_void_1 a;
    std::bind(fp, _1)(&a);
    assert(count == save_count+2);
    save_count = count;
    }
    {
    void (A_void_1::*fp)() const = &A_void_1::mem2;
    std::bind(fp, A_void_1())();
    assert(count == save_count+2);
    save_count = count;
    A_void_1 a;
    std::bind(fp, &a)();
    assert(count == save_count+2);
    save_count = count;
    }
}

// 1 arg, return int

int f_int_1(int i)
{
    return i + 1;
}

struct A_int_1
{
    A_int_1() : data_(5) {}
    int operator()(int i)
    {
        return i - 1;
    }

    int mem1() {return 3;}
    int mem2() const {return 4;}
    int data_;
};

void
test_int_1()
{
    using namespace std::placeholders;
    // function
    {
    assert(std::bind(f_int_1, _1)(2) == 3);
    assert(std::bind(f_int_1, 2)() == 3);
    }
    // function pointer
    {
    int (*fp)(int) = f_int_1;
    assert(std::bind(fp, _1)(3) == 4);
    assert(std::bind(fp, 3)() == 4);
    }
    // functor
    {
    assert(std::bind(A_int_1(), _1)(4) == 3);
    assert(std::bind(A_int_1(), 4)() == 3);
    }
    // member function pointer
    {
    assert(std::bind(&A_int_1::mem1, _1)(A_int_1()) == 3);
    assert(std::bind(&A_int_1::mem1, A_int_1())() == 3);
    A_int_1 a;
    assert(std::bind(&A_int_1::mem1, _1)(&a) == 3);
    assert(std::bind(&A_int_1::mem1, &a)() == 3);
    }
    // const member function pointer
    {
    assert(std::bind(&A_int_1::mem2, _1)(A_int_1()) == 4);
    assert(std::bind(&A_int_1::mem2, A_int_1())() == 4);
    A_int_1 a;
    assert(std::bind(&A_int_1::mem2, _1)(&a) == 4);
    assert(std::bind(&A_int_1::mem2, &a)() == 4);
    }
    // member data pointer
    {
    assert(std::bind(&A_int_1::data_, _1)(A_int_1()) == 5);
    assert(std::bind(&A_int_1::data_, A_int_1())() == 5);
    A_int_1 a;
    assert(std::bind(&A_int_1::data_, _1)(a) == 5);
    std::bind(&A_int_1::data_, _1)(a) = 6;
    assert(std::bind(&A_int_1::data_, _1)(a) == 6);
    assert(std::bind(&A_int_1::data_, _1)(&a) == 6);
    std::bind(&A_int_1::data_, _1)(&a) = 7;
    assert(std::bind(&A_int_1::data_, _1)(&a) == 7);
    }
}

// 2 arg, return void

void f_void_2(int i, int j)
{
    count += i+j;
}

struct A_void_2
{
    void operator()(int i, int j)
    {
        count += i+j;
    }

    void mem1(int i) {count += i;}
    void mem2(int i) const {count += i;}
};

void
test_void_2()
{
    using namespace std::placeholders;
    int save_count = count;
    // function
    {
    std::bind(f_void_2, _1, _2)(2, 3);
    assert(count == save_count+5);
    save_count = count;
    std::bind(f_void_2, 2, _1)(3);
    assert(count == save_count+5);
    save_count = count;
    std::bind(f_void_2, 2, 3)();
    assert(count == save_count+5);
    save_count = count;
    }
    // member function pointer
    {
    std::bind(&A_void_2::mem1, _1, _2)(A_void_2(), 3);
    assert(count == save_count+3);
    save_count = count;
    std::bind(&A_void_2::mem1, _2, _1)(3, A_void_2());
    assert(count == save_count+3);
    save_count = count;
    }
}

int f_nested(int i)
{
    return i+1;
}

int g_nested(int i)
{
    return i*10;
}

void test_nested()
{
    using namespace std::placeholders;
    assert(std::bind(f_nested, std::bind(g_nested, _1))(3) == 31);
}

int main()
{
    test_void_1();
    test_int_1();
    test_void_2();
    test_nested();
}
