//===-------------------- test_exception_storage.cpp ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// XFAIL: c++17

#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <__threading_support>
#include <unistd.h>

#include "../src/cxa_exception.hpp"

typedef __cxxabiv1::__cxa_eh_globals globals_t ;

void *thread_code (void *parm) {
    size_t *result = (size_t *) parm;
    globals_t *glob1, *glob2;
    
    glob1 = __cxxabiv1::__cxa_get_globals ();
    if ( NULL == glob1 )
        std::cerr << "Got null result from __cxa_get_globals" << std::endl;

    glob2 = __cxxabiv1::__cxa_get_globals_fast ();
    if ( glob1 != glob2 )
        std::cerr << "Got different globals!" << std::endl;
    
    *result = (size_t) glob1;
    sleep ( 1 );
    return parm;
    }

#ifndef _LIBCXXABI_HAS_NO_THREADS
#define NUMTHREADS  10
size_t                 thread_globals [ NUMTHREADS ] = { 0 };
std::__libcpp_thread_t   threads        [ NUMTHREADS ];
#endif

int main () {
    int retVal = 0;

#ifndef _LIBCXXABI_HAS_NO_THREADS
//  Make the threads, let them run, and wait for them to finish
    for ( int i = 0; i < NUMTHREADS; ++i )
        std::__libcpp_thread_create ( threads + i, thread_code, (void *) (thread_globals + i));
    for ( int i = 0; i < NUMTHREADS; ++i )
        std::__libcpp_thread_join ( &threads [ i ] );

    for ( int i = 0; i < NUMTHREADS; ++i )
        if ( 0 == thread_globals [ i ] ) {
            std::cerr << "Thread #" << i << " had a zero global" << std::endl;
            retVal = 1;
            }
        
    std::sort ( thread_globals, thread_globals + NUMTHREADS );
    for ( int i = 1; i < NUMTHREADS; ++i )
        if ( thread_globals [ i - 1 ] == thread_globals [ i ] ) {
            std::cerr << "Duplicate thread globals (" << i-1 << " and " << i << ")" << std::endl;
            retVal = 2;
            }
#else // _LIBCXXABI_HAS_NO_THREADS
    size_t thread_globals;
    // Check that __cxa_get_globals() is not NULL.
    if (thread_code(&thread_globals) == 0) {
        retVal = 1;
    }
#endif // !_LIBCXXABI_HAS_NO_THREADS
    return retVal;
}
