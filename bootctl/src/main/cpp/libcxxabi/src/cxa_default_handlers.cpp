//===------------------------- cxa_default_handlers.cpp -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//
// This file implements the default terminate_handler and unexpected_handler.
//===----------------------------------------------------------------------===//

#include <stdexcept>
#include <new>
#include <exception>
#include <cstdlib>
#include "abort_message.h"
#include "cxxabi.h"
#include "cxa_handlers.hpp"
#include "cxa_exception.hpp"
#include "private_typeinfo.h"
#include "include/atomic_support.h"

#if !defined(LIBCXXABI_SILENT_TERMINATE)
static const char* cause = "uncaught";

__attribute__((noreturn))
static void demangling_terminate_handler()
{
    // If there might be an uncaught exception
    using namespace __cxxabiv1;
    __cxa_eh_globals* globals = __cxa_get_globals_fast();
    if (globals)
    {
        __cxa_exception* exception_header = globals->caughtExceptions;
        // If there is an uncaught exception
        if (exception_header)
        {
            _Unwind_Exception* unwind_exception =
                reinterpret_cast<_Unwind_Exception*>(exception_header + 1) - 1;
            if (__isOurExceptionClass(unwind_exception))
            {
                void* thrown_object =
                    __getExceptionClass(unwind_exception) == kOurDependentExceptionClass ?
                        ((__cxa_dependent_exception*)exception_header)->primaryException :
                        exception_header + 1;
                const __shim_type_info* thrown_type =
                    static_cast<const __shim_type_info*>(exception_header->exceptionType);
#if !defined(__ANDROID__)
                // Try to get demangled name of thrown_type
                int status;
                char buf[1024];
                size_t len = sizeof(buf);
                const char* name = __cxa_demangle(thrown_type->name(), buf, &len, &status);
                if (status != 0)
                    name = thrown_type->name();
#else
                const char* name = thrown_type->name();
#endif
                // If the uncaught exception can be caught with std::exception&
                const __shim_type_info* catch_type =
				 static_cast<const __shim_type_info*>(&typeid(std::exception));
                if (catch_type->can_catch(thrown_type, thrown_object))
                {
                    // Include the what() message from the exception
                    const std::exception* e = static_cast<const std::exception*>(thrown_object);
                    abort_message("terminating with %s exception of type %s: %s",
                                  cause, name, e->what());
                }
                else
                    // Else just note that we're terminating with an exception
                    abort_message("terminating with %s exception of type %s",
                                   cause, name);
            }
            else
                // Else we're terminating with a foreign exception
                abort_message("terminating with %s foreign exception", cause);
        }
    }
    // Else just note that we're terminating
    abort_message("terminating");
}

__attribute__((noreturn))
static void demangling_unexpected_handler()
{
    cause = "unexpected";
    std::terminate();
}

static std::terminate_handler default_terminate_handler = demangling_terminate_handler;
static std::terminate_handler default_unexpected_handler = demangling_unexpected_handler;
#else
static std::terminate_handler default_terminate_handler = std::abort;
static std::terminate_handler default_unexpected_handler = std::terminate;
#endif

//
// Global variables that hold the pointers to the current handler
//
_LIBCXXABI_DATA_VIS
std::terminate_handler __cxa_terminate_handler = default_terminate_handler;

_LIBCXXABI_DATA_VIS
std::unexpected_handler __cxa_unexpected_handler = default_unexpected_handler;

namespace std
{

unexpected_handler
set_unexpected(unexpected_handler func) _NOEXCEPT
{
    if (func == 0)
        func = default_unexpected_handler;
    return __libcpp_atomic_exchange(&__cxa_unexpected_handler, func,
                                    _AO_Acq_Rel);
}

terminate_handler
set_terminate(terminate_handler func) _NOEXCEPT
{
    if (func == 0)
        func = default_terminate_handler;
    return __libcpp_atomic_exchange(&__cxa_terminate_handler, func,
                                    _AO_Acq_Rel);
}

}
