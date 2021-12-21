//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>
// REQUIRES: c++98 || c++03 || c++11 || c++14

// class function<R(ArgTypes...)>

// template<class F, class A> function(allocator_arg_t, const A&, F);

#include <functional>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"
#include "test_allocator.h"
#include "count_new.hpp"
#include "../function_types.h"


#if TEST_STD_VER >= 11
struct RValueCallable {
    template <class ...Args>
    void operator()(Args&&...) && {}
};
struct LValueCallable {
    template <class ...Args>
    void operator()(Args&&...) & {}
};
#endif

class DummyClass {};

template <class FuncType, class AllocType>
void test_FunctionObject(AllocType& alloc)
{
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
    FunctionObject target;
    assert(FunctionObject::count == 1);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    std::function<FuncType> f2(std::allocator_arg, alloc, target);
    assert(FunctionObject::count == 2);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(f2.template target<FunctionObject>());
    assert(f2.template target<FuncType>() == 0);
    assert(f2.template target<FuncType*>() == 0);
    }
    assert(FunctionObject::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
}


template <class FuncType, class AllocType>
void test_FreeFunction(AllocType& alloc)
{
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
    FuncType* target = &FreeFunction;
    assert(globalMemCounter.checkOutstandingNewEq(0));
    std::function<FuncType> f2(std::allocator_arg, alloc, target);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f2.template target<FuncType*>());
    assert(*f2.template target<FuncType*>() == target);
    assert(f2.template target<FuncType>() == 0);
    assert(f2.template target<DummyClass>() == 0);
    }
    assert(globalMemCounter.checkOutstandingNewEq(0));
}

template <class TargetType, class FuncType, class AllocType>
void test_MemFunClass(AllocType& alloc)
{
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
    TargetType target = &MemFunClass::foo;
    assert(globalMemCounter.checkOutstandingNewEq(0));
    std::function<FuncType> f2(std::allocator_arg, alloc, target);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(f2.template target<TargetType>());
    assert(*f2.template target<TargetType>() == target);
    assert(f2.template target<FuncType*>() == 0);
    }
    assert(globalMemCounter.checkOutstandingNewEq(0));
}

template <class Alloc>
void test_for_alloc(Alloc& alloc) {
    test_FunctionObject<int()>(alloc);
    test_FunctionObject<int(int)>(alloc);
    test_FunctionObject<int(int, int)>(alloc);
    test_FunctionObject<int(int, int, int)>(alloc);

    test_FreeFunction<int()>(alloc);
    test_FreeFunction<int(int)>(alloc);
    test_FreeFunction<int(int, int)>(alloc);
    test_FreeFunction<int(int, int, int)>(alloc);

    test_MemFunClass<int(MemFunClass::*)() const, int(MemFunClass&)>(alloc);
    test_MemFunClass<int(MemFunClass::*)(int) const, int(MemFunClass&, int)>(alloc);
    test_MemFunClass<int(MemFunClass::*)(int, int) const, int(MemFunClass&, int, int)>(alloc);
}

int main()
{
    {
        bare_allocator<DummyClass> bare_alloc;
        test_for_alloc(bare_alloc);
    }
    {
        non_default_test_allocator<DummyClass> non_default_alloc(42);
        test_for_alloc(non_default_alloc);
    }
#if TEST_STD_VER >= 11
    {
        using Fn = std::function<void(int, int, int)>;
        static_assert(std::is_constructible<Fn, std::allocator_arg_t, std::allocator<int>, LValueCallable&>::value, "");
        static_assert(std::is_constructible<Fn, std::allocator_arg_t, std::allocator<int>, LValueCallable>::value, "");
        static_assert(!std::is_constructible<Fn, std::allocator_arg_t, std::allocator<int>, RValueCallable&>::value, "");
        static_assert(!std::is_constructible<Fn, std::allocator_arg_t, std::allocator<int>, RValueCallable>::value, "");
    }
#endif

}
