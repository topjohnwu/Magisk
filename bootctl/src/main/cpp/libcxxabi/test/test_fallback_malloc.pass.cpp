//===--------------------- test_fallback_malloc.cpp -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <deque>

#include <__threading_support>

typedef std::deque<void *> container;

// #define  DEBUG_FALLBACK_MALLOC
#define INSTRUMENT_FALLBACK_MALLOC
#include "../src/fallback_malloc.cpp"

container alloc_series ( size_t sz ) {
    container ptrs;
    void *p;
    
    while ( NULL != ( p = fallback_malloc ( sz )))
        ptrs.push_back ( p );
    return ptrs;
    }

container alloc_series ( size_t sz, float growth ) {
    container ptrs;
    void *p;
    
    while ( NULL != ( p = fallback_malloc ( sz ))) {
        ptrs.push_back ( p );
        sz *= growth;
        }

    return ptrs;
    }

container alloc_series ( const size_t *first, size_t len ) {
    container ptrs;
    const size_t *last = first + len;
    void * p;
    
    for ( const size_t *iter = first; iter != last; ++iter ) {
        if ( NULL == (p = fallback_malloc ( *iter )))
            break;
        ptrs.push_back ( p );
        }

    return ptrs;
    }

void *pop ( container &c, bool from_end ) {
    void *ptr;
    if ( from_end ) {
        ptr = c.back ();
        c.pop_back ();
        }
    else {
        ptr = c.front ();
        c.pop_front ();
        }
    return ptr;
    }

void exhaustion_test1 () {
    container ptrs;
    
    init_heap ();
    std::cout << "Constant exhaustion tests" << std::endl;
    
//  Delete in allocation order
    ptrs = alloc_series ( 32 );
    std::cout << "Allocated " << ptrs.size () << " 32 byte chunks" << std::endl;
    print_free_list ();
    for ( container::iterator iter = ptrs.begin (); iter != ptrs.end (); ++iter )
        fallback_free ( *iter );
    print_free_list ();
    std::cout << "----" << std::endl;

//  Delete in reverse order
    ptrs = alloc_series ( 32 );
    std::cout << "Allocated " << ptrs.size () << " 32 byte chunks" << std::endl;
    for ( container::reverse_iterator iter = ptrs.rbegin (); iter != ptrs.rend (); ++iter )
        fallback_free ( *iter );
    print_free_list ();
    std::cout << "----" << std::endl;

//  Alternate deletions
    ptrs = alloc_series ( 32 );
    std::cout << "Allocated " << ptrs.size () << " 32 byte chunks" << std::endl;
    while ( ptrs.size () > 0 )
        fallback_free ( pop ( ptrs, ptrs.size () % 1 == 1 ));
    print_free_list ();
    }
            
void exhaustion_test2 () {
    container ptrs;
    init_heap ();
    
    std::cout << "Growing exhaustion tests" << std::endl;

//  Delete in allocation order
    ptrs = alloc_series ( 32, 1.5 );
    std::cout << "Allocated " << ptrs.size () << " { 32, 48, 72, 108, 162 ... }  byte chunks" << std::endl;
    print_free_list ();
    for ( container::iterator iter = ptrs.begin (); iter != ptrs.end (); ++iter )
        fallback_free ( *iter );
    print_free_list ();
    std::cout << "----" << std::endl;
    
//  Delete in reverse order
    print_free_list ();
    ptrs = alloc_series ( 32, 1.5 );
    std::cout << "Allocated " << ptrs.size () << " { 32, 48, 72, 108, 162 ... }  byte chunks" << std::endl;
    for ( container::reverse_iterator iter = ptrs.rbegin (); iter != ptrs.rend (); ++iter )
        fallback_free ( *iter );
    print_free_list ();
    std::cout << "----" << std::endl;

//  Alternate deletions
    ptrs = alloc_series ( 32, 1.5 );
    std::cout << "Allocated " << ptrs.size () << " { 32, 48, 72, 108, 162 ... }  byte chunks" << std::endl;
    while ( ptrs.size () > 0 )
        fallback_free ( pop ( ptrs, ptrs.size () % 1 == 1 ));
    print_free_list (); 
    
    }

void exhaustion_test3 () {
    const size_t allocs [] = { 124, 60, 252, 60, 4 };
    container ptrs;
    init_heap ();
    
    std::cout << "Complete exhaustion tests" << std::endl;

//  Delete in allocation order
    ptrs = alloc_series ( allocs, sizeof ( allocs ) / sizeof ( allocs[0] ));
    std::cout << "Allocated " << ptrs.size () << " chunks" << std::endl;
    print_free_list ();
    for ( container::iterator iter = ptrs.begin (); iter != ptrs.end (); ++iter )
        fallback_free ( *iter );
    print_free_list ();
    std::cout << "----" << std::endl;
    
//  Delete in reverse order
    print_free_list ();
    ptrs = alloc_series ( allocs, sizeof ( allocs ) / sizeof ( allocs[0] ));
    std::cout << "Allocated " << ptrs.size () << " chunks" << std::endl;
    for ( container::reverse_iterator iter = ptrs.rbegin (); iter != ptrs.rend (); ++iter )
        fallback_free ( *iter );
    print_free_list ();
    std::cout << "----" << std::endl;

//  Alternate deletions
    ptrs = alloc_series ( allocs, sizeof ( allocs ) / sizeof ( allocs[0] ));
    std::cout << "Allocated " << ptrs.size () << " chunks" << std::endl;
    while ( ptrs.size () > 0 )
        fallback_free ( pop ( ptrs, ptrs.size () % 1 == 1 ));
    print_free_list (); 
    
    }

    
int main () {
    print_free_list ();

    char *p = (char *) fallback_malloc ( 1024 );    // too big!
    std::cout << "fallback_malloc ( 1024 ) --> " << (unsigned long ) p << std::endl;
    print_free_list ();
    
    p = (char *) fallback_malloc ( 32 );
    std::cout << "fallback_malloc ( 32 ) --> " << (unsigned long) (p - heap) << std::endl;
    if ( !is_fallback_ptr ( p ))
        std::cout << "### p is not a fallback pointer!!" << std::endl;
    
    print_free_list ();
    fallback_free ( p );
    print_free_list ();
    
    std::cout << std::endl;
    exhaustion_test1 (); std::cout << std::endl;
    exhaustion_test2 (); std::cout << std::endl;
    exhaustion_test3 (); std::cout << std::endl;
    return 0;
    }
