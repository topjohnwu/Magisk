//===--------------------------- test_vector2.cpp -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include "cxxabi.h"

#include <iostream>
#include <cstdlib>

void my_terminate () { exit ( 0 ); }

//  Wrapper routines
void *my_alloc2 ( size_t sz ) {
    void *p = std::malloc ( sz );
//  std::printf ( "Allocated %ld bytes at %lx\n", sz, (unsigned long) p );  
    return p;
    }
    
void my_dealloc2 ( void *p ) {
//  std::printf ( "Freeing %lx\n", (unsigned long) p ); 
    std::free ( p ); 
    }

void my_dealloc3 ( void *p, size_t ) {
//  std::printf ( "Freeing %lx (size %ld)\n", (unsigned long) p, sz );  
    std::free ( p ); 
    }

void my_construct ( void *) {
//  std::printf ( "Constructing %lx\n", (unsigned long) p );
    }

void my_destruct  ( void *) {
//  std::printf ( "Destructing  %lx\n", (unsigned long) p );
    }

int gCounter;
void count_construct ( void * ) { ++gCounter; }
void count_destruct  ( void * ) { --gCounter; }


int gConstructorCounter;
int gConstructorThrowTarget;
int gDestructorCounter;
int gDestructorThrowTarget;
void throw_construct ( void * ) { if ( gConstructorCounter   == gConstructorThrowTarget ) throw 1; ++gConstructorCounter; }
void throw_destruct  ( void * ) { if ( ++gDestructorCounter  == gDestructorThrowTarget  ) throw 2; }

struct vec_on_stack {
    void *storage;
    vec_on_stack () : storage ( __cxxabiv1::__cxa_vec_new    (            10, 40, 8, throw_construct, throw_destruct )) {}
    ~vec_on_stack () {          __cxxabiv1::__cxa_vec_delete ( storage,       40, 8,                  throw_destruct );  }
    };


//  Make sure the constructors and destructors are matched
void test_exception_in_destructor ( ) {

//  Try throwing from a destructor while unwinding the stack -- should abort
    gConstructorCounter = gDestructorCounter = 0;
    gConstructorThrowTarget = -1;
    gDestructorThrowTarget  = 5;
    try {
        vec_on_stack v;
        throw 3;
        }
    catch ( int i ) {}

    std::cerr << "should never get here" << std::endl;    
    }



int main () {
    std::set_terminate ( my_terminate );
    test_exception_in_destructor ();
    return 1;       // we failed if we get here
    }
