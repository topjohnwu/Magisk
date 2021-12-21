//===--------------------- stdlib_new_delete.cpp --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//
// This file implements the new and delete operators.
//===----------------------------------------------------------------------===//

#define _LIBCPP_BUILDING_LIBRARY
#include "__cxxabi_config.h"
#include <new>
#include <cstdlib>

#if !defined(_THROW_BAD_ALLOC) || !defined(_NOEXCEPT) || !defined(_LIBCXXABI_WEAK)
#error The _THROW_BAD_ALLOC, _NOEXCEPT, and _LIBCXXABI_WEAK libc++ macros must \
       already be defined by libc++.
#endif
// Implement all new and delete operators as weak definitions
// in this shared library, so that they can be overridden by programs
// that define non-weak copies of the functions.

_LIBCXXABI_WEAK
void *
operator new(std::size_t size) _THROW_BAD_ALLOC
{
    if (size == 0)
        size = 1;
    void* p;
    while ((p = ::malloc(size)) == 0)
    {
        // If malloc fails and there is a new_handler,
        // call it to try free up memory.
        std::new_handler nh = std::get_new_handler();
        if (nh)
            nh();
        else
#ifndef _LIBCXXABI_NO_EXCEPTIONS
            throw std::bad_alloc();
#else
            break;
#endif
    }
    return p;
}

_LIBCXXABI_WEAK
void*
operator new(size_t size, const std::nothrow_t&) _NOEXCEPT
{
    void* p = 0;
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCXXABI_NO_EXCEPTIONS
        p = ::operator new(size);
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    }
    catch (...)
    {
    }
#endif  // _LIBCXXABI_NO_EXCEPTIONS
    return p;
}

_LIBCXXABI_WEAK
void*
operator new[](size_t size) _THROW_BAD_ALLOC
{
    return ::operator new(size);
}

_LIBCXXABI_WEAK
void*
operator new[](size_t size, const std::nothrow_t&) _NOEXCEPT
{
    void* p = 0;
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCXXABI_NO_EXCEPTIONS
        p = ::operator new[](size);
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    }
    catch (...)
    {
    }
#endif  // _LIBCXXABI_NO_EXCEPTIONS
    return p;
}

_LIBCXXABI_WEAK
void
operator delete(void* ptr) _NOEXCEPT
{
    if (ptr)
        ::free(ptr);
}

_LIBCXXABI_WEAK
void
operator delete(void* ptr, const std::nothrow_t&) _NOEXCEPT
{
    ::operator delete(ptr);
}

_LIBCXXABI_WEAK
void
operator delete(void* ptr, size_t) _NOEXCEPT
{
    ::operator delete(ptr);
}

_LIBCXXABI_WEAK
void
operator delete[] (void* ptr) _NOEXCEPT
{
    ::operator delete(ptr);
}

_LIBCXXABI_WEAK
void
operator delete[] (void* ptr, const std::nothrow_t&) _NOEXCEPT
{
    ::operator delete[](ptr);
}

_LIBCXXABI_WEAK
void
operator delete[] (void* ptr, size_t) _NOEXCEPT
{
    ::operator delete[](ptr);
}

#if !defined(_LIBCPP_HAS_NO_LIBRARY_ALIGNED_ALLOCATION)

_LIBCXXABI_WEAK
void *
operator new(std::size_t size, std::align_val_t alignment) _THROW_BAD_ALLOC
{
    if (size == 0)
        size = 1;
    if (static_cast<size_t>(alignment) < sizeof(void*))
      alignment = std::align_val_t(sizeof(void*));
    void* p;
#if defined(_LIBCPP_WIN32API)
    while ((p = _aligned_malloc(size, static_cast<size_t>(alignment))) == nullptr)
#else
    while (::posix_memalign(&p, static_cast<size_t>(alignment), size) != 0)
#endif
    {
        // If posix_memalign fails and there is a new_handler,
        // call it to try free up memory.
        std::new_handler nh = std::get_new_handler();
        if (nh)
            nh();
        else {
#ifndef _LIBCXXABI_NO_EXCEPTIONS
            throw std::bad_alloc();
#else
            p = nullptr; // posix_memalign doesn't initialize 'p' on failure
            break;
#endif
        }
    }
    return p;
}

_LIBCXXABI_WEAK
void*
operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) _NOEXCEPT
{
    void* p = 0;
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCXXABI_NO_EXCEPTIONS
        p = ::operator new(size, alignment);
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    }
    catch (...)
    {
    }
#endif  // _LIBCXXABI_NO_EXCEPTIONS
    return p;
}

_LIBCXXABI_WEAK
void*
operator new[](size_t size, std::align_val_t alignment) _THROW_BAD_ALLOC
{
    return ::operator new(size, alignment);
}

_LIBCXXABI_WEAK
void*
operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) _NOEXCEPT
{
    void* p = 0;
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCXXABI_NO_EXCEPTIONS
        p = ::operator new[](size, alignment);
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    }
    catch (...)
    {
    }
#endif  // _LIBCXXABI_NO_EXCEPTIONS
    return p;
}

_LIBCXXABI_WEAK
void
operator delete(void* ptr, std::align_val_t) _NOEXCEPT
{
    if (ptr)
#if defined(_LIBCPP_WIN32API)
        ::_aligned_free(ptr);
#else
        ::free(ptr);
#endif
}

_LIBCXXABI_WEAK
void
operator delete(void* ptr, std::align_val_t alignment, const std::nothrow_t&) _NOEXCEPT
{
    ::operator delete(ptr, alignment);
}

_LIBCXXABI_WEAK
void
operator delete(void* ptr, size_t, std::align_val_t alignment) _NOEXCEPT
{
    ::operator delete(ptr, alignment);
}

_LIBCXXABI_WEAK
void
operator delete[] (void* ptr, std::align_val_t alignment) _NOEXCEPT
{
    ::operator delete(ptr, alignment);
}

_LIBCXXABI_WEAK
void
operator delete[] (void* ptr, std::align_val_t alignment, const std::nothrow_t&) _NOEXCEPT
{
    ::operator delete[](ptr, alignment);
}

_LIBCXXABI_WEAK
void
operator delete[] (void* ptr, size_t, std::align_val_t alignment) _NOEXCEPT
{
    ::operator delete[](ptr, alignment);
}

#endif // !_LIBCPP_HAS_NO_LIBRARY_ALIGNED_ALLOCATION
