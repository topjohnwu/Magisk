//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef FUNCTION_TYPES_H
#define FUNCTION_TYPES_H


class FunctionObject
{
    int data_[10]; // dummy variable to prevent small object optimization in
                   // std::function
public:
    static int count;

    FunctionObject() {
        ++count;
        for (int i = 0; i < 10; ++i) data_[i] = i;
    }

    FunctionObject(const FunctionObject&) {++count;}
    ~FunctionObject() {--count; ((void)data_); }

    int operator()() const { return 42; }
    int operator()(int i) const { return i; }
    int operator()(int i, int) const { return i; }
    int operator()(int i, int, int) const { return i; }
};

int FunctionObject::count = 0;

class MemFunClass
{
    int data_[10]; // dummy variable to prevent small object optimization in
                   // std::function
public:
    static int count;

    MemFunClass() {
        ++count;
        for (int i = 0; i < 10; ++i) data_[i] = 0;
    }

    MemFunClass(const MemFunClass&) {++count; ((void)data_); }

    ~MemFunClass() {--count;}

    int foo() const { return 42; }
    int foo(int i) const { return i; }
    int foo(int i, int) const { return i; }
    int foo(int i, int, int) const { return i; }
};

int MemFunClass::count = 0;

int FreeFunction() { return 42; }
int FreeFunction(int i) {return i;}
int FreeFunction(int i, int) { return i; }
int FreeFunction(int i, int, int) { return i; }

#endif // FUNCTION_TYPES_H
