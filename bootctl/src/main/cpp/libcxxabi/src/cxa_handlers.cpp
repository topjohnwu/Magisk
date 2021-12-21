//===------------------------- cxa_handlers.cpp ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//
// This file implements the functionality associated with the terminate_handler,
//   unexpected_handler, and new_handler.
//===----------------------------------------------------------------------===//

#include <stdexcept>
#include <new>
#include <exception>
#include "abort_message.h"
#include "cxxabi.h"
#include "cxa_handlers.hpp"
#include "cxa_exception.hpp"
#include "private_typeinfo.h"
#include "include/atomic_support.h"

namespace std
{

unexpected_handler
get_unexpected() _NOEXCEPT
{
    return __libcpp_atomic_load(&__cxa_unexpected_handler, _AO_Acquire);
}

void
__unexpected(unexpected_handler func)
{
    func();
    // unexpected handler should not return
    abort_message("unexpected_handler unexpectedly returned");
}

__attribute__((noreturn))
void
unexpected()
{
    __unexpected(get_unexpected());
}

terminate_handler
get_terminate() _NOEXCEPT
{
    return __libcpp_atomic_load(&__cxa_terminate_handler, _AO_Acquire);
}

void
__terminate(terminate_handler func) _NOEXCEPT
{
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCXXABI_NO_EXCEPTIONS
        func();
        // handler should not return
        abort_message("terminate_handler unexpectedly returned");
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    }
    catch (...)
    {
        // handler should not throw exception
        abort_message("terminate_handler unexpectedly threw an exception");
    }
#endif  // _LIBCXXABI_NO_EXCEPTIONS
}

__attribute__((noreturn))
void
terminate() _NOEXCEPT
{
    // If there might be an uncaught exception
    using namespace __cxxabiv1;
    __cxa_eh_globals* globals = __cxa_get_globals_fast();
    if (globals)
    {
        __cxa_exception* exception_header = globals->caughtExceptions;
        if (exception_header)
        {
            _Unwind_Exception* unwind_exception =
                reinterpret_cast<_Unwind_Exception*>(exception_header + 1) - 1;
            if (__isOurExceptionClass(unwind_exception))
                __terminate(exception_header->terminateHandler);
        }
    }
    __terminate(get_terminate());
}

extern "C" {
new_handler __cxa_new_handler = 0;
}

new_handler
set_new_handler(new_handler handler) _NOEXCEPT
{
    return __libcpp_atomic_exchange(&__cxa_new_handler, handler, _AO_Acq_Rel);
}

new_handler
get_new_handler() _NOEXCEPT
{
    return __libcpp_atomic_load(&__cxa_new_handler, _AO_Acquire);
}

}  // std
