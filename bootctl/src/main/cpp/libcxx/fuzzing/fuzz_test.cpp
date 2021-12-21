// -*- C++ -*-
//===------------------------- fuzz_test.cpp ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//  A simple program for running regressions on the fuzzing routines.
//  This code is not part of any shipping product.
//
//  To build:
//      clang++ -std=c++11 fuzz_test.cpp fuzzing.cpp
//
//  To use:
//      fuzz_test -r partial_sort [-v] files...
//
//  Each file should contain a test case.

//  TODO: should add some memory tracking, too.


#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <map>
#include <chrono>

#include "fuzzing.h"

//  ==== Count memory allocations ====

struct MemoryCounters {
    size_t totalAllocationCount;
    size_t netAllocationCount;
    size_t totalBytesAllocated;
    };

MemoryCounters gMemoryCounters;

void ZeroMemoryCounters() {
    gMemoryCounters.totalAllocationCount = 0;
    gMemoryCounters.netAllocationCount = 0;
    gMemoryCounters.totalBytesAllocated = 0;
}

void* operator new(std::size_t size)
{
    if (size == 0) size = 1;
    void *p = ::malloc(size);
    if (p == NULL)
        throw std::bad_alloc();
    gMemoryCounters.totalAllocationCount += 1;
    gMemoryCounters.netAllocationCount  += 1;
    gMemoryCounters.totalBytesAllocated += size;
    return p;
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    try { return operator new(size); }
    catch (const std::bad_alloc &) {}
    return nullptr;
}

void* operator new[](std::size_t size)
{
    return ::operator new(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    try { return operator new(size); }
    catch (const std::bad_alloc &) {}
    return nullptr;
}

void  operator delete(void* ptr) noexcept
{
    if (ptr)
        ::free(ptr);
    gMemoryCounters.netAllocationCount -= 1;
}

void  operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    ::operator delete(ptr);
}

void  operator delete[](void* ptr) noexcept
{
    ::operator delete(ptr);
}

void  operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    ::operator delete(ptr);
}

//  ==== End count memory allocations ====


typedef int (*FuzzProc) (const uint8_t *data, size_t size);

const std::map<std::string, FuzzProc> procs = {
    {"sort",                fuzzing::sort},
    {"stable_sort",         fuzzing::stable_sort},
    {"partition",           fuzzing::partition},
    {"partition_copy",      fuzzing::partition_copy},
    {"stable_partition",    fuzzing::stable_partition},
    {"unique",              fuzzing::unique},
    {"unique_copy",         fuzzing::unique_copy},
    {"nth_element",         fuzzing::nth_element},
    {"partial_sort",        fuzzing::partial_sort},
    {"partial_sort_copy",   fuzzing::partial_sort_copy},
    {"make_heap",           fuzzing::make_heap},
    {"push_heap",           fuzzing::push_heap},
    {"pop_heap",            fuzzing::pop_heap},
    {"regex_ECMAScript",    fuzzing::regex_ECMAScript},
    {"regex_POSIX",         fuzzing::regex_POSIX},
    {"regex_extended",      fuzzing::regex_extended},
    {"regex_awk",           fuzzing::regex_awk},
    {"regex_grep",          fuzzing::regex_grep},
    {"regex_egrep",         fuzzing::regex_egrep},
    {"search",              fuzzing::search}
};



bool verbose = false;

void test_one(const char *filename, FuzzProc fp)
{
    std::vector<uint8_t> v;
    std::ifstream f (filename, std::ios::binary);
    if (!f.is_open())
        std::cerr << "## Can't open '" << filename << "'" << std::endl;
    else
    {
        typedef std::istream_iterator<uint8_t> Iter;
        std::copy(Iter(f), Iter(), std::back_inserter(v));
        if (verbose)
            std::cout << "File '" << filename << "' contains " << v.size() << " entries" << std::endl;
        ZeroMemoryCounters();
        const auto start_time = std::chrono::high_resolution_clock::now();
        int ret = fp (v.data(), v.size());
        const auto finish_time = std::chrono::high_resolution_clock::now();
        MemoryCounters mc = gMemoryCounters;
        if (ret != 0)
            std::cerr << "## Failure code: " << ret << std::endl;
        if (verbose)
        {
            std::cout << "Execution time: "
                << std::chrono::duration_cast<std::chrono::milliseconds>(finish_time - start_time).count()
                << " milliseconds" << std::endl;
            std::cout << "Memory: " 
                      << mc.totalBytesAllocated  << " bytes allocated ("
                      << mc.totalAllocationCount << " allocations); "
                      << mc.netAllocationCount   << " allocations remain" << std::endl;
        }
    }
}

void usage (const char *name)
{
    std::cout << "Usage: " << name << " -r proc [-v] files..." << std::endl;
    std::cout << "Supported routines:" << std::endl;
    for (const auto &p : procs)
        std::cout << "    " << p.first << std::endl;
    std::cout << std::endl;
}

// Poor man's command-line options
const std::string dashR("-r");
const std::string dashV("-v");

int main(int argc, char *argv[])
{
    if (argc < 4 || dashR != argv[1] || procs.find(argv[2]) == procs.end())
        usage(argv[0]);
    else {
        FuzzProc fp = procs.find(argv[2])->second;
        int firstFile = 3;
        if (dashV == argv[firstFile])
        {
            verbose = true;
            ++firstFile;
        }
        for (int i = firstFile; i < argc; ++i)
            test_one(argv[i], fp);
        }
}
