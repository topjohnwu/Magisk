//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// R operator()(ArgTypes... args) const

#include <functional>
#include <cassert>


int count = 0;


// 0 args, return int

int f_int_0()
{
    return 3;
}

struct A_int_0
{
    int operator()() {return 4;}
};

void test_int_0()
{
    // function
    {
        std::function<int ()> r1(f_int_0);
        assert(r1() == 3);
    }
    // function pointer
    {
        int (*fp)() = f_int_0;
        std::function<int ()> r1(fp);
        assert(r1() == 3);
    }
    // functor
    {
        A_int_0 a0;
        std::function<int ()> r1(a0);
        assert(r1() == 4);
    }
}


// 0 args, return void

void f_void_0()
{
    ++count;
}

struct A_void_0
{
    void operator()() {++count;}
};

void
test_void_0()
{
    int save_count = count;
    // function
    {
        std::function<void ()> r1(f_void_0);
        r1();
        assert(count == save_count+1);
        save_count = count;
    }
    // function pointer
    {
        void (*fp)() = f_void_0;
        std::function<void ()> r1(fp);
        r1();
        assert(count == save_count+1);
        save_count = count;
    }
    // functor
    {
        A_void_0 a0;
        std::function<void ()> r1(a0);
        r1();
        assert(count == save_count+1);
        save_count = count;
    }
}

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
        std::function<void (int)> r1(f_void_1);
        int i = 2;
        r1(i);
        assert(count == save_count+2);
        save_count = count;
    }
    // function pointer
    {
        void (*fp)(int) = f_void_1;
        std::function<void (int)> r1(fp);
        int i = 3;
        r1(i);
        assert(count == save_count+3);
        save_count = count;
    }
    // functor
    {
        A_void_1 a0;
        std::function<void (int)> r1(a0);
        int i = 4;
        r1(i);
        assert(count == save_count+4);
        save_count = count;
    }
    // member function pointer
    {
        void (A_void_1::*fp)() = &A_void_1::mem1;
        std::function<void (A_void_1)> r1(fp);
        A_void_1 a;
        r1(a);
        assert(count == save_count+1);
        save_count = count;
        A_void_1* ap = &a;
        std::function<void (A_void_1*)> r2 = fp;
        r2(ap);
        assert(count == save_count+1);
        save_count = count;
    }
    // const member function pointer
    {
        void (A_void_1::*fp)() const = &A_void_1::mem2;
        std::function<void (A_void_1)> r1(fp);
        A_void_1 a;
        r1(a);
        assert(count == save_count+1);
        save_count = count;
        std::function<void (A_void_1*)> r2(fp);
        A_void_1* ap = &a;
        r2(ap);
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
        std::function<int (int)> r1(f_int_1);
        int i = 2;
        assert(r1(i) == 3);
    }
    // function pointer
    {
        int (*fp)(int) = f_int_1;
        std::function<int (int)> r1(fp);
        int i = 3;
        assert(r1(i) == 4);
    }
    // functor
    {
        A_int_1 a0;
        std::function<int (int)> r1(a0);
        int i = 4;
        assert(r1(i) == 3);
    }
    // member function pointer
    {
        int (A_int_1::*fp)() = &A_int_1::mem1;
        std::function<int (A_int_1)> r1(fp);
        A_int_1 a;
        assert(r1(a) == 3);
        std::function<int (A_int_1*)> r2(fp);
        A_int_1* ap = &a;
        assert(r2(ap) == 3);
    }
    // const member function pointer
    {
        int (A_int_1::*fp)() const = &A_int_1::mem2;
        std::function<int (A_int_1)> r1(fp);
        A_int_1 a;
        assert(r1(a) == 4);
        std::function<int (A_int_1*)> r2(fp);
        A_int_1* ap = &a;
        assert(r2(ap) == 4);
    }
    // member data pointer
    {
        int A_int_1::*fp = &A_int_1::data_;
        std::function<int& (A_int_1&)> r1(fp);
        A_int_1 a;
        assert(r1(a) == 5);
        r1(a) = 6;
        assert(r1(a) == 6);
        std::function<int& (A_int_1*)> r2(fp);
        A_int_1* ap = &a;
        assert(r2(ap) == 6);
        r2(ap) = 7;
        assert(r2(ap) == 7);
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
        std::function<void (int, int)> r1(f_void_2);
        int i = 2;
        int j = 3;
        r1(i, j);
        assert(count == save_count+5);
        save_count = count;
    }
    // function pointer
    {
        void (*fp)(int, int) = f_void_2;
        std::function<void (int, int)> r1(fp);
        int i = 3;
        int j = 4;
        r1(i, j);
        assert(count == save_count+7);
        save_count = count;
    }
    // functor
    {
        A_void_2 a0;
        std::function<void (int, int)> r1(a0);
        int i = 4;
        int j = 5;
        r1(i, j);
        assert(count == save_count+9);
        save_count = count;
    }
    // member function pointer
    {
        void (A_void_2::*fp)(int) = &A_void_2::mem1;
        std::function<void (A_void_2, int)> r1(fp);
        A_void_2 a;
        int i = 3;
        r1(a, i);
        assert(count == save_count+3);
        save_count = count;
        std::function<void (A_void_2*, int)> r2(fp);
        A_void_2* ap = &a;
        r2(ap, i);
        assert(count == save_count+3);
        save_count = count;
    }
    // const member function pointer
    {
        void (A_void_2::*fp)(int) const = &A_void_2::mem2;
        std::function<void (A_void_2, int)> r1(fp);
        A_void_2 a;
        int i = 4;
        r1(a, i);
        assert(count == save_count+4);
        save_count = count;
        std::function<void (A_void_2*, int)> r2(fp);
        A_void_2* ap = &a;
        r2(ap, i);
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

void test_int_2()
{
    // function
    {
        std::function<int (int, int)> r1(f_int_2);
        int i = 2;
        int j = 3;
        assert(r1(i, j) == i+j);
    }
    // function pointer
    {
        int (*fp)(int, int) = f_int_2;
        std::function<int (int, int)> r1(fp);
        int i = 3;
        int j = 4;
        assert(r1(i, j) == i+j);
    }
    // functor
    {
        A_int_2 a0;
        std::function<int (int, int)> r1(a0);
        int i = 4;
        int j = 5;
        assert(r1(i, j) == i+j);
    }
    // member function pointer
    {
        int(A_int_2::*fp)(int) = &A_int_2::mem1;
        std::function<int (A_int_2, int)> r1(fp);
        A_int_2 a;
        int i = 3;
        assert(r1(a, i) == i+1);
        std::function<int (A_int_2*, int)> r2(fp);
        A_int_2* ap = &a;
        assert(r2(ap, i) == i+1);
    }
    // const member function pointer
    {
        int (A_int_2::*fp)(int) const = &A_int_2::mem2;
        std::function<int (A_int_2, int)> r1(fp);
        A_int_2 a;
        int i = 4;
        assert(r1(a, i) == i+2);
        std::function<int (A_int_2*, int)> r2(fp);
        A_int_2* ap = &a;
        assert(r2(ap, i) == i+2);
    }
}

int main()
{
    test_void_0();
    test_int_0();
    test_void_1();
    test_int_1();
    test_void_2();
    test_int_2();
}
