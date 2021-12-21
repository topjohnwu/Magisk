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

// template<class F> function(F);

// Allow incomplete argument types in the __is_callable check

#include <functional>
#include <cassert>

struct X{
    typedef std::function<void(X&)> callback_type;
    virtual ~X() {}
private:
    callback_type _cb;
};

struct IncompleteReturnType {
  std::function<IncompleteReturnType ()> fn;
};


int called = 0;
IncompleteReturnType test_fn() {
  ++called;
  IncompleteReturnType I;
  return I;
}

// See llvm.org/PR34298
void test_pr34298()
{
  static_assert(std::is_copy_constructible<IncompleteReturnType>::value, "");
  static_assert(std::is_copy_assignable<IncompleteReturnType>::value, "");
  {
    IncompleteReturnType X;
    X.fn = test_fn;
    const IncompleteReturnType& CX = X;
    IncompleteReturnType X2 = CX;
    assert(X2.fn);
    assert(called == 0);
    X2.fn();
    assert(called == 1);
  }
  {
    IncompleteReturnType Empty;
    IncompleteReturnType X2 = Empty;
    assert(!X2.fn);
  }
}

int main() {
  test_pr34298();
}
