//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// reference_wrapper

// template <class... ArgTypes>
//   requires Callable<T, ArgTypes&&...>
//   Callable<T, ArgTypes&&...>::result_type
//   operator()(ArgTypes&&... args) const;

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
    void mem2() const {++count;}
};

void
test_void_1()
{
    int save_count = count;
    // function
    {
    std::reference_wrapper<void (int)> r1(f_void_1);
    int i = 2;
    r1(i);
    assert(count == save_count+2);
    save_count = count;
    }
    // function pointer
    {
    void (*fp)(int) = f_void_1;
    std::reference_wrapper<void (*)(int)> r1(fp);
    int i = 3;
    r1(i);
    assert(count == save_count+3);
    save_count = count;
    }
    // functor
    {
    A_void_1 a0;
    std::reference_wrapper<A_void_1> r1(a0);
    int i = 4;
    r1(i);
    assert(count == save_count+4);
    save_count = count;
    }
    // member function pointer
    {
    void (A_void_1::*fp)() = &A_void_1::mem1;
    std::reference_wrapper<void (A_void_1::*)()> r1(fp);
    A_void_1 a;
    r1(a);
    assert(count == save_count+1);
    save_count = count;
    A_void_1* ap = &a;
    r1(ap);
    assert(count == save_count+1);
    save_count = count;
    }
    // const member function pointer
    {
    void (A_void_1::*fp)() const = &A_void_1::mem2;
    std::reference_wrapper<void (A_void_1::*)() const> r1(fp);
    A_void_1 a;
    r1(a);
    assert(count == save_count+1);
    save_count = count;
    A_void_1* ap = &a;
    r1(ap);
    assert(count == save_count+1);
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
    // function
    {
    std::reference_wrapper<int (int)> r1(f_int_1);
    int i = 2;
    assert(r1(i) == 3);
    }
    // function pointer
    {
    int (*fp)(int) = f_int_1;
    std::reference_wrapper<int (*)(int)> r1(fp);
    int i = 3;
    assert(r1(i) == 4);
    }
    // functor
    {
    A_int_1 a0;
    std::reference_wrapper<A_int_1> r1(a0);
    int i = 4;
    assert(r1(i) == 3);
    }
    // member function pointer
    {
    int (A_int_1::*fp)() = &A_int_1::mem1;
    std::reference_wrapper<int (A_int_1::*)()> r1(fp);
    A_int_1 a;
    assert(r1(a) == 3);
    A_int_1* ap = &a;
    assert(r1(ap) == 3);
    }
    // const member function pointer
    {
    int (A_int_1::*fp)() const = &A_int_1::mem2;
    std::reference_wrapper<int (A_int_1::*)() const> r1(fp);
    A_int_1 a;
    assert(r1(a) == 4);
    A_int_1* ap = &a;
    assert(r1(ap) == 4);
    }
    // member data pointer
    {
    int A_int_1::*fp = &A_int_1::data_;
    std::reference_wrapper<int A_int_1::*> r1(fp);
    A_int_1 a;
    assert(r1(a) == 5);
    r1(a) = 6;
    assert(r1(a) == 6);
    A_int_1* ap = &a;
    assert(r1(ap) == 6);
    r1(ap) = 7;
    assert(r1(ap) == 7);
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
    int save_count = count;
    // function
    {
    std::reference_wrapper<void (int, int)> r1(f_void_2);
    int i = 2;
    int j = 3;
    r1(i, j);
    assert(count == save_count+5);
    save_count = count;
    }
    // function pointer
    {
    void (*fp)(int, int) = f_void_2;
    std::reference_wrapper<void (*)(int, int)> r1(fp);
    int i = 3;
    int j = 4;
    r1(i, j);
    assert(count == save_count+7);
    save_count = count;
    }
    // functor
    {
    A_void_2 a0;
    std::reference_wrapper<A_void_2> r1(a0);
    int i = 4;
    int j = 5;
    r1(i, j);
    assert(count == save_count+9);
    save_count = count;
    }
    // member function pointer
    {
    void (A_void_2::*fp)(int) = &A_void_2::mem1;
    std::reference_wrapper<void (A_void_2::*)(int)> r1(fp);
    A_void_2 a;
    int i = 3;
    r1(a, i);
    assert(count == save_count+3);
    save_count = count;
    A_void_2* ap = &a;
    r1(ap, i);
    assert(count == save_count+3);
    save_count = count;
    }
    // const member function pointer
    {
    void (A_void_2::*fp)(int) const = &A_void_2::mem2;
    std::reference_wrapper<void (A_void_2::*)(int) const> r1(fp);
    A_void_2 a;
    int i = 4;
    r1(a, i);
    assert(count == save_count+4);
    save_count = count;
    A_void_2* ap = &a;
    r1(ap, i);
    assert(count == save_count+4);
    save_count = count;
    }
}

// 2 arg, return int

int f_int_2(int i, int j)
{
    return i+j;
}

struct A_int_2
{
    int operator()(int i, int j)
    {
        return i+j;
    }

    int mem1(int i) {return i+1;}
    int mem2(int i) const {return i+2;}
};

void
testint_2()
{
    // function
    {
    std::reference_wrapper<int (int, int)> r1(f_int_2);
    int i = 2;
    int j = 3;
    assert(r1(i, j) == i+j);
    }
    // function pointer
    {
    int (*fp)(int, int) = f_int_2;
    std::reference_wrapper<int (*)(int, int)> r1(fp);
    int i = 3;
    int j = 4;
    assert(r1(i, j) == i+j);
    }
    // functor
    {
    A_int_2 a0;
    std::reference_wrapper<A_int_2> r1(a0);
    int i = 4;
    int j = 5;
    assert(r1(i, j) == i+j);
    }
    // member function pointer
    {
    int(A_int_2::*fp)(int) = &A_int_2::mem1;
    std::reference_wrapper<int (A_int_2::*)(int)> r1(fp);
    A_int_2 a;
    int i = 3;
    assert(r1(a, i) == i+1);
    A_int_2* ap = &a;
    assert(r1(ap, i) == i+1);
    }
    // const member function pointer
    {
    int (A_int_2::*fp)(int) const = &A_int_2::mem2;
    std::reference_wrapper<int (A_int_2::*)(int) const> r1(fp);
    A_int_2 a;
    int i = 4;
    assert(r1(a, i) == i+2);
    A_int_2* ap = &a;
    assert(r1(ap, i) == i+2);
    }
}

int main()
{
    test_void_1();
    test_int_1();
    test_void_2();
    testint_2();
}
