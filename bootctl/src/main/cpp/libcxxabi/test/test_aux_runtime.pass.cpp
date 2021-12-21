//===-------------------------- test_aux_runtime.cpp ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <typeinfo>
#include <iostream>

//  Test taken from 5.2.8.2
//  When typeid is applied to a glvalue expression whose type is a polymorphic 
//  class type, (10.3), the result refers to a std::type_info object 
//  representing the type of the most derived object (1.8) (that is, the 
//  dynamic type) to which the glvalue refers. If the glvalue expression is 
//  obtained by applying the unary * operator to a pointer(68) and the pointer 
//  is a null pointer value (4.10), the typeid expression throws the 
//  std::bad_typeid exception (18.7.3).
//
//  68) If p is an expression of pointer type, then *p, (*p), *(p), 
//      ((*p)), *((p)), and so on all meet this requirement.
bool bad_typeid_test () {
    class A { virtual void f() {}}; 
    class B { virtual void g() {}}; 
    
    B *bp = NULL;
    try {bool b = typeid(*bp) == typeid (A); ((void)b); }
    catch ( const std::bad_typeid &) { return true; }
    return false;
    }
    

//  The value of a failed cast to pointer type is the null pointer value of 
//  the required result type. A failed cast to reference type throws 
//  std::bad_cast (18.7.2).
bool bad_cast_test () {
    class A { virtual void f() {}};
    class B { virtual void g() {}};
    class D : public virtual A, private B {};   

    D d;
    B *bp = (B*)&d;     // cast needed to break protection
    try { D &dr = dynamic_cast<D&> (*bp); ((void)dr); }
    catch ( const std::bad_cast & ) { return true; }
    return false;
    }
    
int main ( ) {
    int ret_val = 0;
    
    if ( !bad_typeid_test ()) {
        std::cerr << "TypeID test failed!" << std::endl;
        ret_val = 1;
    }
    
    if ( !bad_cast_test ()) {
        std::cerr << "Bad cast test failed!" << std::endl;
        ret_val = 1;
    }
    
    return ret_val;
    }
