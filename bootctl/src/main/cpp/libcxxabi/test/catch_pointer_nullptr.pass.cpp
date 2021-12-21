//===--------------------- catch_pointer_nullptr.cpp ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, libcxxabi-no-exceptions

#include <cassert>
#include <cstdlib>

struct A {};

void test1()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (int* p)
    {
        assert(!p);
    }
    catch (long*)
    {
        assert(false);
    }
}

void test2()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (A* p)
    {
        assert(!p);
    }
    catch (int*)
    {
        assert(false);
    }
}

template <class Catch>
void catch_nullptr_test() {
  try {
    throw nullptr;
    assert(false);
  } catch (Catch c) {
    assert(!c);
  } catch (...) {
    assert(false);
  }
}


int main()
{
  // catch naked nullptrs
  test1();
  test2();

  catch_nullptr_test<int*>();
  catch_nullptr_test<int**>();
  catch_nullptr_test<int A::*>();
  catch_nullptr_test<const int A::*>();
  catch_nullptr_test<int A::**>();
}
