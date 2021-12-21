//===------------------------- cxa_exception.cpp --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//  
//  This file implements the "Exception Handling APIs"
//  http://mentorembedded.github.io/cxx-abi/abi-eh.html
//  http://www.intel.com/design/itanium/downloads/245358.htm
//  
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>

#include "__cxxabi_config.h"
#include "cxa_exception.hpp"
#include "cxa_handlers.hpp"
#include "private_typeinfo.h"
#include "unwind.h"

#if defined(__SEH__) && !defined(__USING_SJLJ_EXCEPTIONS__)
#include <windows.h>
#include <winnt.h>

extern "C" EXCEPTION_DISPOSITION _GCC_specific_handler(PEXCEPTION_RECORD,
                                                       void *, PCONTEXT,
                                                       PDISPATCHER_CONTEXT,
                                                       _Unwind_Personality_Fn);
#endif

/*
    Exception Header Layout:

+---------------------------+-----------------------------+---------------+
| __cxa_exception           | _Unwind_Exception CLNGC++\0 | thrown object |
+---------------------------+-----------------------------+---------------+
                                                          ^
                                                          |
  +-------------------------------------------------------+
  |
+---------------------------+-----------------------------+
| __cxa_dependent_exception | _Unwind_Exception CLNGC++\1 |
+---------------------------+-----------------------------+

    Exception Handling Table Layout:

+-----------------+--------+
| lpStartEncoding | (char) |
+---------+-------+--------+---------------+-----------------------+
| lpStart | (encoded with lpStartEncoding) | defaults to funcStart |
+---------+-----+--------+-----------------+---------------+-------+
| ttypeEncoding | (char) | Encoding of the type_info table |
+---------------+-+------+----+----------------------------+----------------+
| classInfoOffset | (ULEB128) | Offset to type_info table, defaults to null |
+-----------------++--------+-+----------------------------+----------------+
| callSiteEncoding | (char) | Encoding for Call Site Table |
+------------------+--+-----+-----+------------------------+--------------------------+
| callSiteTableLength | (ULEB128) | Call Site Table length, used to find Action table |
+---------------------+-----------+---------------------------------------------------+
#ifndef __USING_SJLJ_EXCEPTIONS__
+---------------------+-----------+------------------------------------------------+
| Beginning of Call Site Table            The current ip lies within the           |
| ...                                     (start, length) range of one of these    |
|                                         call sites. There may be action needed.  |
| +-------------+---------------------------------+------------------------------+ |
| | start       | (encoded with callSiteEncoding) | offset relative to funcStart | |
| | length      | (encoded with callSiteEncoding) | length of code fragment      | |
| | landingPad  | (encoded with callSiteEncoding) | offset relative to lpStart   | |
| | actionEntry | (ULEB128)                       | Action Table Index 1-based   | |
| |             |                                 | actionEntry == 0 -> cleanup  | |
| +-------------+---------------------------------+------------------------------+ |
| ...                                                                              |
+----------------------------------------------------------------------------------+
#else  // __USING_SJLJ_EXCEPTIONS__
+---------------------+-----------+------------------------------------------------+
| Beginning of Call Site Table            The current ip is a 1-based index into   |
| ...                                     this table.  Or it is -1 meaning no      |
|                                         action is needed.  Or it is 0 meaning    |
|                                         terminate.                               |
| +-------------+---------------------------------+------------------------------+ |
| | landingPad  | (ULEB128)                       | offset relative to lpStart   | |
| | actionEntry | (ULEB128)                       | Action Table Index 1-based   | |
| |             |                                 | actionEntry == 0 -> cleanup  | |
| +-------------+---------------------------------+------------------------------+ |
| ...                                                                              |
+----------------------------------------------------------------------------------+
#endif  // __USING_SJLJ_EXCEPTIONS__
+---------------------------------------------------------------------+
| Beginning of Action Table       ttypeIndex == 0 : cleanup           |
| ...                             ttypeIndex  > 0 : catch             |
|                                 ttypeIndex  < 0 : exception spec    |
| +--------------+-----------+--------------------------------------+ |
| | ttypeIndex   | (SLEB128) | Index into type_info Table (1-based) | |
| | actionOffset | (SLEB128) | Offset into next Action Table entry  | |
| +--------------+-----------+--------------------------------------+ |
| ...                                                                 |
+---------------------------------------------------------------------+-----------------+
| type_info Table, but classInfoOffset does *not* point here!                           |
| +----------------+------------------------------------------------+-----------------+ |
| | Nth type_info* | Encoded with ttypeEncoding, 0 means catch(...) | ttypeIndex == N | |
| +----------------+------------------------------------------------+-----------------+ |
| ...                                                                                   |
| +----------------+------------------------------------------------+-----------------+ |
| | 1st type_info* | Encoded with ttypeEncoding, 0 means catch(...) | ttypeIndex == 1 | |
| +----------------+------------------------------------------------+-----------------+ |
| +---------------------------------------+-----------+------------------------------+  |
| | 1st ttypeIndex for 1st exception spec | (ULEB128) | classInfoOffset points here! |  |
| | ...                                   | (ULEB128) |                              |  |
| | Mth ttypeIndex for 1st exception spec | (ULEB128) |                              |  |
| | 0                                     | (ULEB128) |                              |  |
| +---------------------------------------+------------------------------------------+  |
| ...                                                                                   |
| +---------------------------------------+------------------------------------------+  |
| | 0                                     | (ULEB128) | throw()                      |  |
| +---------------------------------------+------------------------------------------+  |
| ...                                                                                   |
| +---------------------------------------+------------------------------------------+  |
| | 1st ttypeIndex for Nth exception spec | (ULEB128) |                              |  |
| | ...                                   | (ULEB128) |                              |  |
| | Mth ttypeIndex for Nth exception spec | (ULEB128) |                              |  |
| | 0                                     | (ULEB128) |                              |  |
| +---------------------------------------+------------------------------------------+  |
+---------------------------------------------------------------------------------------+

Notes:

*  ttypeIndex in the Action Table, and in the exception spec table, is an index,
     not a byte count, if positive.  It is a negative index offset of
     classInfoOffset and the sizeof entry depends on ttypeEncoding.
   But if ttypeIndex is negative, it is a positive 1-based byte offset into the
     type_info Table.
   And if ttypeIndex is zero, it refers to a catch (...).

*  landingPad can be 0, this implies there is nothing to be done.

*  landingPad != 0 and actionEntry == 0 implies a cleanup needs to be done
     @landingPad.

*  A cleanup can also be found under landingPad != 0 and actionEntry != 0 in
     the Action Table with ttypeIndex == 0.
*/

namespace __cxxabiv1
{

namespace
{

template <class AsType>
uintptr_t readPointerHelper(const uint8_t*& p) {
    AsType value;
    memcpy(&value, p, sizeof(AsType));
    p += sizeof(AsType);
    return static_cast<uintptr_t>(value);
}

} // end namespace

extern "C"
{

// private API

// Heavily borrowed from llvm/examples/ExceptionDemo/ExceptionDemo.cpp

// DWARF Constants
enum
{
    DW_EH_PE_absptr   = 0x00,
    DW_EH_PE_uleb128  = 0x01,
    DW_EH_PE_udata2   = 0x02,
    DW_EH_PE_udata4   = 0x03,
    DW_EH_PE_udata8   = 0x04,
    DW_EH_PE_sleb128  = 0x09,
    DW_EH_PE_sdata2   = 0x0A,
    DW_EH_PE_sdata4   = 0x0B,
    DW_EH_PE_sdata8   = 0x0C,
    DW_EH_PE_pcrel    = 0x10,
    DW_EH_PE_textrel  = 0x20,
    DW_EH_PE_datarel  = 0x30,
    DW_EH_PE_funcrel  = 0x40,
    DW_EH_PE_aligned  = 0x50,
    DW_EH_PE_indirect = 0x80,
    DW_EH_PE_omit     = 0xFF
};

/// Read a uleb128 encoded value and advance pointer 
/// See Variable Length Data Appendix C in: 
/// @link http://dwarfstd.org/Dwarf4.pdf @unlink
/// @param data reference variable holding memory pointer to decode from
/// @returns decoded value
static
uintptr_t
readULEB128(const uint8_t** data)
{
    uintptr_t result = 0;
    uintptr_t shift = 0;
    unsigned char byte;
    const uint8_t *p = *data;
    do
    {
        byte = *p++;
        result |= static_cast<uintptr_t>(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);
    *data = p;
    return result;
}

/// Read a sleb128 encoded value and advance pointer 
/// See Variable Length Data Appendix C in: 
/// @link http://dwarfstd.org/Dwarf4.pdf @unlink
/// @param data reference variable holding memory pointer to decode from
/// @returns decoded value
static
intptr_t
readSLEB128(const uint8_t** data)
{
    uintptr_t result = 0;
    uintptr_t shift = 0;
    unsigned char byte;
    const uint8_t *p = *data;
    do
    {
        byte = *p++;
        result |= static_cast<uintptr_t>(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);
    *data = p;
    if ((byte & 0x40) && (shift < (sizeof(result) << 3)))
        result |= static_cast<uintptr_t>(~0) << shift;
    return static_cast<intptr_t>(result);
}

/// Read a pointer encoded value and advance pointer 
/// See Variable Length Data in: 
/// @link http://dwarfstd.org/Dwarf3.pdf @unlink
/// @param data reference variable holding memory pointer to decode from
/// @param encoding dwarf encoding type
/// @returns decoded value
static
uintptr_t
readEncodedPointer(const uint8_t** data, uint8_t encoding)
{
    uintptr_t result = 0;
    if (encoding == DW_EH_PE_omit) 
        return result;
    const uint8_t* p = *data;
    // first get value 
    switch (encoding & 0x0F)
    {
    case DW_EH_PE_absptr:
        result = readPointerHelper<uintptr_t>(p);
        break;
    case DW_EH_PE_uleb128:
        result = readULEB128(&p);
        break;
    case DW_EH_PE_sleb128:
        result = static_cast<uintptr_t>(readSLEB128(&p));
        break;
    case DW_EH_PE_udata2:
        result = readPointerHelper<uint16_t>(p);
        break;
    case DW_EH_PE_udata4:
        result = readPointerHelper<uint32_t>(p);
        break;
    case DW_EH_PE_udata8:
        result = readPointerHelper<uint64_t>(p);
        break;
    case DW_EH_PE_sdata2:
        result = readPointerHelper<int16_t>(p);
        break;
    case DW_EH_PE_sdata4:
        result = readPointerHelper<int32_t>(p);
        break;
    case DW_EH_PE_sdata8:
        result = readPointerHelper<int64_t>(p);
        break;
    default:
        // not supported 
        abort();
        break;
    }
    // then add relative offset 
    switch (encoding & 0x70)
    {
    case DW_EH_PE_absptr:
        // do nothing 
        break;
    case DW_EH_PE_pcrel:
        if (result)
            result += (uintptr_t)(*data);
        break;
    case DW_EH_PE_textrel:
    case DW_EH_PE_datarel:
    case DW_EH_PE_funcrel:
    case DW_EH_PE_aligned:
    default:
        // not supported 
        abort();
        break;
    }
    // then apply indirection 
    if (result && (encoding & DW_EH_PE_indirect))
        result = *((uintptr_t*)result);
    *data = p;
    return result;
}

static
void
call_terminate(bool native_exception, _Unwind_Exception* unwind_exception)
{
    __cxa_begin_catch(unwind_exception);
    if (native_exception)
    {
        // Use the stored terminate_handler if possible
        __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception+1) - 1;
        std::__terminate(exception_header->terminateHandler);
    }
    std::terminate();
}

#if defined(_LIBCXXABI_ARM_EHABI)
static const void* read_target2_value(const void* ptr)
{
    uintptr_t offset = *reinterpret_cast<const uintptr_t*>(ptr);
    if (!offset)
        return 0;
    // "ARM EABI provides a TARGET2 relocation to describe these typeinfo
    // pointers. The reason being it allows their precise semantics to be
    // deferred to the linker. For bare-metal they turn into absolute
    // relocations. For linux they turn into GOT-REL relocations."
    // https://gcc.gnu.org/ml/gcc-patches/2009-08/msg00264.html
#if defined(LIBCXXABI_BAREMETAL)
    return reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(ptr) +
                                         offset);
#else
    return *reinterpret_cast<const void **>(reinterpret_cast<uintptr_t>(ptr) +
                                            offset);
#endif
}

static const __shim_type_info*
get_shim_type_info(uint64_t ttypeIndex, const uint8_t* classInfo,
                   uint8_t ttypeEncoding, bool native_exception,
                   _Unwind_Exception* unwind_exception)
{
    if (classInfo == 0)
    {
        // this should not happen.  Indicates corrupted eh_table.
        call_terminate(native_exception, unwind_exception);
    }

    assert(((ttypeEncoding == DW_EH_PE_absptr) ||  // LLVM or GCC 4.6
            (ttypeEncoding == DW_EH_PE_pcrel) ||  // GCC 4.7 baremetal
            (ttypeEncoding == (DW_EH_PE_pcrel | DW_EH_PE_indirect))) &&  // GCC 4.7 linux
           "Unexpected TTypeEncoding");
    (void)ttypeEncoding;

    const uint8_t* ttypePtr = classInfo - ttypeIndex * sizeof(uintptr_t);
    return reinterpret_cast<const __shim_type_info *>(
        read_target2_value(ttypePtr));
}
#else // !defined(_LIBCXXABI_ARM_EHABI)
static
const __shim_type_info*
get_shim_type_info(uint64_t ttypeIndex, const uint8_t* classInfo,
                   uint8_t ttypeEncoding, bool native_exception,
                   _Unwind_Exception* unwind_exception)
{
    if (classInfo == 0)
    {
        // this should not happen.  Indicates corrupted eh_table.
        call_terminate(native_exception, unwind_exception);
    }
    switch (ttypeEncoding & 0x0F)
    {
    case DW_EH_PE_absptr:
        ttypeIndex *= sizeof(void*);
        break;
    case DW_EH_PE_udata2:
    case DW_EH_PE_sdata2:
        ttypeIndex *= 2;
        break;
    case DW_EH_PE_udata4:
    case DW_EH_PE_sdata4:
        ttypeIndex *= 4;
        break;
    case DW_EH_PE_udata8:
    case DW_EH_PE_sdata8:
        ttypeIndex *= 8;
        break;
    default:
        // this should not happen.   Indicates corrupted eh_table.
        call_terminate(native_exception, unwind_exception);
    }
    classInfo -= ttypeIndex;
    return (const __shim_type_info*)readEncodedPointer(&classInfo, ttypeEncoding);
}
#endif // !defined(_LIBCXXABI_ARM_EHABI)

/*
    This is checking a thrown exception type, excpType, against a possibly empty
    list of catchType's which make up an exception spec.

    An exception spec acts like a catch handler, but in reverse.  This "catch
    handler" will catch an excpType if and only if none of the catchType's in
    the list will catch a excpType.  If any catchType in the list can catch an
    excpType, then this exception spec does not catch the excpType.
*/
#if defined(_LIBCXXABI_ARM_EHABI)
static
bool
exception_spec_can_catch(int64_t specIndex, const uint8_t* classInfo,
                         uint8_t ttypeEncoding, const __shim_type_info* excpType,
                         void* adjustedPtr, _Unwind_Exception* unwind_exception)
{
    if (classInfo == 0)
    {
        // this should not happen.   Indicates corrupted eh_table.
        call_terminate(false, unwind_exception);
    }

    assert(((ttypeEncoding == DW_EH_PE_absptr) ||  // LLVM or GCC 4.6
            (ttypeEncoding == DW_EH_PE_pcrel) ||  // GCC 4.7 baremetal
            (ttypeEncoding == (DW_EH_PE_pcrel | DW_EH_PE_indirect))) &&  // GCC 4.7 linux
           "Unexpected TTypeEncoding");
    (void)ttypeEncoding;

    // specIndex is negative of 1-based byte offset into classInfo;
    specIndex = -specIndex;
    --specIndex;
    const void** temp = reinterpret_cast<const void**>(
        reinterpret_cast<uintptr_t>(classInfo) +
        static_cast<uintptr_t>(specIndex) * sizeof(uintptr_t));
    // If any type in the spec list can catch excpType, return false, else return true
    //    adjustments to adjustedPtr are ignored.
    while (true)
    {
        // ARM EHABI exception specification table (filter table) consists of
        // several pointers which will directly point to the type info object
        // (instead of ttypeIndex).  The table will be terminated with 0.
        const void** ttypePtr = temp++;
        if (*ttypePtr == 0)
            break;
        // We can get the __shim_type_info simply by performing a
        // R_ARM_TARGET2 relocation, and cast the result to __shim_type_info.
        const __shim_type_info* catchType =
            static_cast<const __shim_type_info*>(read_target2_value(ttypePtr));
        void* tempPtr = adjustedPtr;
        if (catchType->can_catch(excpType, tempPtr))
            return false;
    }
    return true;
}
#else
static
bool
exception_spec_can_catch(int64_t specIndex, const uint8_t* classInfo,
                         uint8_t ttypeEncoding, const __shim_type_info* excpType,
                         void* adjustedPtr, _Unwind_Exception* unwind_exception)
{
    if (classInfo == 0)
    {
        // this should not happen.   Indicates corrupted eh_table.
        call_terminate(false, unwind_exception);
    }
    // specIndex is negative of 1-based byte offset into classInfo;
    specIndex = -specIndex;
    --specIndex;
    const uint8_t* temp = classInfo + specIndex;
    // If any type in the spec list can catch excpType, return false, else return true
    //    adjustments to adjustedPtr are ignored.
    while (true)
    {
        uint64_t ttypeIndex = readULEB128(&temp);
        if (ttypeIndex == 0)
            break;
        const __shim_type_info* catchType = get_shim_type_info(ttypeIndex,
                                                               classInfo,
                                                               ttypeEncoding,
                                                               true,
                                                               unwind_exception);
        void* tempPtr = adjustedPtr;
        if (catchType->can_catch(excpType, tempPtr))
            return false;
    }
    return true;
}
#endif

static
void*
get_thrown_object_ptr(_Unwind_Exception* unwind_exception)
{
    // Even for foreign exceptions, the exception object is *probably* at unwind_exception + 1
    //    Regardless, this library is prohibited from touching a foreign exception
    void* adjustedPtr = unwind_exception + 1;
    if (__getExceptionClass(unwind_exception) == kOurDependentExceptionClass)
        adjustedPtr = ((__cxa_dependent_exception*)adjustedPtr - 1)->primaryException;
    return adjustedPtr;
}

namespace
{

struct scan_results
{
    int64_t        ttypeIndex;   // > 0 catch handler, < 0 exception spec handler, == 0 a cleanup
    const uint8_t* actionRecord;         // Currently unused.  Retained to ease future maintenance.
    const uint8_t* languageSpecificData;  // Needed only for __cxa_call_unexpected
    uintptr_t      landingPad;   // null -> nothing found, else something found
    void*          adjustedPtr;  // Used in cxa_exception.cpp
    _Unwind_Reason_Code reason;  // One of _URC_FATAL_PHASE1_ERROR,
                                 //        _URC_FATAL_PHASE2_ERROR,
                                 //        _URC_CONTINUE_UNWIND,
                                 //        _URC_HANDLER_FOUND
};

}  // unnamed namespace

static
void
set_registers(_Unwind_Exception* unwind_exception, _Unwind_Context* context,
              const scan_results& results)
{
#if defined(__USING_SJLJ_EXCEPTIONS__)
#define __builtin_eh_return_data_regno(regno) regno
#endif
  _Unwind_SetGR(context, __builtin_eh_return_data_regno(0),
                reinterpret_cast<uintptr_t>(unwind_exception));
  _Unwind_SetGR(context, __builtin_eh_return_data_regno(1),
                static_cast<uintptr_t>(results.ttypeIndex));
  _Unwind_SetIP(context, results.landingPad);
}

/*
    There are 3 types of scans needed:

    1.  Scan for handler with native or foreign exception.  If handler found,
        save state and return _URC_HANDLER_FOUND, else return _URC_CONTINUE_UNWIND.
        May also report an error on invalid input.
        May terminate for invalid exception table.
        _UA_SEARCH_PHASE

    2.  Scan for handler with foreign exception.  Must return _URC_HANDLER_FOUND,
        or call terminate.
        _UA_CLEANUP_PHASE && _UA_HANDLER_FRAME && !native_exception

    3.  Scan for cleanups.  If a handler is found and this isn't forced unwind,
        then terminate, otherwise ignore the handler and keep looking for cleanup.
        If a cleanup is found, return _URC_HANDLER_FOUND, else return _URC_CONTINUE_UNWIND.
        May also report an error on invalid input.
        May terminate for invalid exception table.
        _UA_CLEANUP_PHASE && !_UA_HANDLER_FRAME
*/

static void scan_eh_tab(scan_results &results, _Unwind_Action actions,
                        bool native_exception,
                        _Unwind_Exception *unwind_exception,
                        _Unwind_Context *context) {
    // Initialize results to found nothing but an error
    results.ttypeIndex = 0;
    results.actionRecord = 0;
    results.languageSpecificData = 0;
    results.landingPad = 0;
    results.adjustedPtr = 0;
    results.reason = _URC_FATAL_PHASE1_ERROR;
    // Check for consistent actions
    if (actions & _UA_SEARCH_PHASE)
    {
        // Do Phase 1
        if (actions & (_UA_CLEANUP_PHASE | _UA_HANDLER_FRAME | _UA_FORCE_UNWIND))
        {
            // None of these flags should be set during Phase 1
            //   Client error
            results.reason = _URC_FATAL_PHASE1_ERROR;
            return;
        }
    }
    else if (actions & _UA_CLEANUP_PHASE)
    {
        if ((actions & _UA_HANDLER_FRAME) && (actions & _UA_FORCE_UNWIND))
        {
            // _UA_HANDLER_FRAME should only be set if phase 1 found a handler.
            // If _UA_FORCE_UNWIND is set, phase 1 shouldn't have happened.
            //    Client error
            results.reason = _URC_FATAL_PHASE2_ERROR;
            return;
        }
    }
    else // Neither _UA_SEARCH_PHASE nor _UA_CLEANUP_PHASE is set
    {
        // One of these should be set.
        //   Client error
        results.reason = _URC_FATAL_PHASE1_ERROR;
        return;
    }
    // Start scan by getting exception table address
    const uint8_t *lsda = (const uint8_t *)_Unwind_GetLanguageSpecificData(context);
    if (lsda == 0)
    {
        // There is no exception table
        results.reason = _URC_CONTINUE_UNWIND;
        return;
    }
    results.languageSpecificData = lsda;
    // Get the current instruction pointer and offset it before next
    // instruction in the current frame which threw the exception.
    uintptr_t ip = _Unwind_GetIP(context) - 1;
    // Get beginning current frame's code (as defined by the 
    // emitted dwarf code)
    uintptr_t funcStart = _Unwind_GetRegionStart(context);
#ifdef __USING_SJLJ_EXCEPTIONS__
    if (ip == uintptr_t(-1))
    {
        // no action
        results.reason = _URC_CONTINUE_UNWIND;
        return;
    }
    else if (ip == 0)
        call_terminate(native_exception, unwind_exception);
    // ip is 1-based index into call site table
#else  // !__USING_SJLJ_EXCEPTIONS__
    uintptr_t ipOffset = ip - funcStart;
#endif  // !defined(_USING_SLJL_EXCEPTIONS__)
    const uint8_t* classInfo = NULL;
    // Note: See JITDwarfEmitter::EmitExceptionTable(...) for corresponding
    //       dwarf emission
    // Parse LSDA header.
    uint8_t lpStartEncoding = *lsda++;
    const uint8_t* lpStart = (const uint8_t*)readEncodedPointer(&lsda, lpStartEncoding);
    if (lpStart == 0)
        lpStart = (const uint8_t*)funcStart;
    uint8_t ttypeEncoding = *lsda++;
    if (ttypeEncoding != DW_EH_PE_omit)
    {
        // Calculate type info locations in emitted dwarf code which
        // were flagged by type info arguments to llvm.eh.selector
        // intrinsic
        uintptr_t classInfoOffset = readULEB128(&lsda);
        classInfo = lsda + classInfoOffset;
    }
    // Walk call-site table looking for range that 
    // includes current PC. 
    uint8_t callSiteEncoding = *lsda++;
#ifdef __USING_SJLJ_EXCEPTIONS__
    (void)callSiteEncoding;  // When using SjLj exceptions, callSiteEncoding is never used
#endif
    uint32_t callSiteTableLength = static_cast<uint32_t>(readULEB128(&lsda));
    const uint8_t* callSiteTableStart = lsda;
    const uint8_t* callSiteTableEnd = callSiteTableStart + callSiteTableLength;
    const uint8_t* actionTableStart = callSiteTableEnd;
    const uint8_t* callSitePtr = callSiteTableStart;
    while (callSitePtr < callSiteTableEnd)
    {
        // There is one entry per call site.
#ifndef __USING_SJLJ_EXCEPTIONS__
        // The call sites are non-overlapping in [start, start+length)
        // The call sites are ordered in increasing value of start
        uintptr_t start = readEncodedPointer(&callSitePtr, callSiteEncoding);
        uintptr_t length = readEncodedPointer(&callSitePtr, callSiteEncoding);
        uintptr_t landingPad = readEncodedPointer(&callSitePtr, callSiteEncoding);
        uintptr_t actionEntry = readULEB128(&callSitePtr);
        if ((start <= ipOffset) && (ipOffset < (start + length)))
#else  // __USING_SJLJ_EXCEPTIONS__
        // ip is 1-based index into this table
        uintptr_t landingPad = readULEB128(&callSitePtr);
        uintptr_t actionEntry = readULEB128(&callSitePtr);
        if (--ip == 0)
#endif  // __USING_SJLJ_EXCEPTIONS__
        {
            // Found the call site containing ip.
#ifndef __USING_SJLJ_EXCEPTIONS__
            if (landingPad == 0)
            {
                // No handler here
                results.reason = _URC_CONTINUE_UNWIND;
                return;
            }
            landingPad = (uintptr_t)lpStart + landingPad;
#else  // __USING_SJLJ_EXCEPTIONS__
            ++landingPad;
#endif  // __USING_SJLJ_EXCEPTIONS__
            if (actionEntry == 0)
            {
                // Found a cleanup
                // If this is a type 1 or type 2 search, there are no handlers
                // If this is a type 3 search, you want to install the cleanup.
                if ((actions & _UA_CLEANUP_PHASE) && !(actions & _UA_HANDLER_FRAME))
                {
                    results.ttypeIndex = 0;  // Redundant but clarifying
                    results.landingPad = landingPad;
                    results.reason = _URC_HANDLER_FOUND;
                    return;
                }
                // No handler here
                results.reason = _URC_CONTINUE_UNWIND;
                return;
            }
            // Convert 1-based byte offset into
            const uint8_t* action = actionTableStart + (actionEntry - 1);
            // Scan action entries until you find a matching handler, cleanup, or the end of action list
            while (true)
            {
                const uint8_t* actionRecord = action;
                int64_t ttypeIndex = readSLEB128(&action);
                if (ttypeIndex > 0)
                {
                    // Found a catch, does it actually catch?
                    // First check for catch (...)
                    const __shim_type_info* catchType =
                        get_shim_type_info(static_cast<uint64_t>(ttypeIndex),
                                           classInfo, ttypeEncoding,
                                           native_exception, unwind_exception);
                    if (catchType == 0)
                    {
                        // Found catch (...) catches everything, including foreign exceptions
                        // If this is a type 1 search save state and return _URC_HANDLER_FOUND
                        // If this is a type 2 search save state and return _URC_HANDLER_FOUND
                        // If this is a type 3 search !_UA_FORCE_UNWIND, we should have found this in phase 1!
                        // If this is a type 3 search _UA_FORCE_UNWIND, ignore handler and continue scan
                        if ((actions & _UA_SEARCH_PHASE) || (actions & _UA_HANDLER_FRAME))
                        {
                            // Save state and return _URC_HANDLER_FOUND
                            results.ttypeIndex = ttypeIndex;
                            results.actionRecord = actionRecord;
                            results.landingPad = landingPad;
                            results.adjustedPtr = get_thrown_object_ptr(unwind_exception);
                            results.reason = _URC_HANDLER_FOUND;
                            return;
                        }
                        else if (!(actions & _UA_FORCE_UNWIND))
                        {
                            // It looks like the exception table has changed
                            //    on us.  Likely stack corruption!
                            call_terminate(native_exception, unwind_exception);
                        }
                    }
                    // Else this is a catch (T) clause and will never
                    //    catch a foreign exception
                    else if (native_exception)
                    {
                        __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception+1) - 1;
                        void* adjustedPtr = get_thrown_object_ptr(unwind_exception);
                        const __shim_type_info* excpType =
                            static_cast<const __shim_type_info*>(exception_header->exceptionType);
                        if (adjustedPtr == 0 || excpType == 0)
                        {
                            // Something very bad happened
                            call_terminate(native_exception, unwind_exception);
                        }
                        if (catchType->can_catch(excpType, adjustedPtr))
                        {
                            // Found a matching handler
                            // If this is a type 1 search save state and return _URC_HANDLER_FOUND
                            // If this is a type 3 search and !_UA_FORCE_UNWIND, we should have found this in phase 1!
                            // If this is a type 3 search and _UA_FORCE_UNWIND, ignore handler and continue scan
                            if (actions & _UA_SEARCH_PHASE)
                            {
                                // Save state and return _URC_HANDLER_FOUND
                                results.ttypeIndex = ttypeIndex;
                                results.actionRecord = actionRecord;
                                results.landingPad = landingPad;
                                results.adjustedPtr = adjustedPtr;
                                results.reason = _URC_HANDLER_FOUND;
                                return;
                            }
                            else if (!(actions & _UA_FORCE_UNWIND))
                            {
                                // It looks like the exception table has changed
                                //    on us.  Likely stack corruption!
                                call_terminate(native_exception, unwind_exception);
                            }
                        }
                    }
                    // Scan next action ...
                }
                else if (ttypeIndex < 0)
                {
                    // Found an exception spec.  If this is a foreign exception,
                    //   it is always caught.
                    if (native_exception)
                    {
                        // Does the exception spec catch this native exception?
                        __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception+1) - 1;
                        void* adjustedPtr = get_thrown_object_ptr(unwind_exception);
                        const __shim_type_info* excpType =
                            static_cast<const __shim_type_info*>(exception_header->exceptionType);
                        if (adjustedPtr == 0 || excpType == 0)
                        {
                            // Something very bad happened
                            call_terminate(native_exception, unwind_exception);
                        }
                        if (exception_spec_can_catch(ttypeIndex, classInfo,
                                                     ttypeEncoding, excpType,
                                                     adjustedPtr, unwind_exception))
                        {
                            // native exception caught by exception spec
                            // If this is a type 1 search, save state and return _URC_HANDLER_FOUND
                            // If this is a type 3 search !_UA_FORCE_UNWIND, we should have found this in phase 1!
                            // If this is a type 3 search _UA_FORCE_UNWIND, ignore handler and continue scan
                            if (actions & _UA_SEARCH_PHASE)
                            {
                                // Save state and return _URC_HANDLER_FOUND
                                results.ttypeIndex = ttypeIndex;
                                results.actionRecord = actionRecord;
                                results.landingPad = landingPad;
                                results.adjustedPtr = adjustedPtr;
                                results.reason = _URC_HANDLER_FOUND;
                                return;
                            }
                            else if (!(actions & _UA_FORCE_UNWIND))
                            {
                                // It looks like the exception table has changed
                                //    on us.  Likely stack corruption!
                                call_terminate(native_exception, unwind_exception);
                            }
                        }
                    }
                    else
                    {
                        // foreign exception caught by exception spec
                        // If this is a type 1 search, save state and return _URC_HANDLER_FOUND
                        // If this is a type 2 search, save state and return _URC_HANDLER_FOUND
                        // If this is a type 3 search !_UA_FORCE_UNWIND, we should have found this in phase 1!
                        // If this is a type 3 search _UA_FORCE_UNWIND, ignore handler and continue scan
                        if ((actions & _UA_SEARCH_PHASE) || (actions & _UA_HANDLER_FRAME))
                        {
                            // Save state and return _URC_HANDLER_FOUND
                            results.ttypeIndex = ttypeIndex;
                            results.actionRecord = actionRecord;
                            results.landingPad = landingPad;
                            results.adjustedPtr = get_thrown_object_ptr(unwind_exception);
                            results.reason = _URC_HANDLER_FOUND;
                            return;
                        }
                        else if (!(actions & _UA_FORCE_UNWIND))
                        {
                            // It looks like the exception table has changed
                            //    on us.  Likely stack corruption!
                            call_terminate(native_exception, unwind_exception);
                        }
                    }
                    // Scan next action ...
                }
                else  // ttypeIndex == 0
                {
                    // Found a cleanup
                    // If this is a type 1 search, ignore it and continue scan
                    // If this is a type 2 search, ignore it and continue scan
                    // If this is a type 3 search, save state and return _URC_HANDLER_FOUND
                    if ((actions & _UA_CLEANUP_PHASE) && !(actions & _UA_HANDLER_FRAME))
                    {
                        // Save state and return _URC_HANDLER_FOUND
                        results.ttypeIndex = ttypeIndex;
                        results.actionRecord = actionRecord;
                        results.landingPad = landingPad;
                        results.adjustedPtr = get_thrown_object_ptr(unwind_exception);
                        results.reason = _URC_HANDLER_FOUND;
                        return;
                    }
                }
                const uint8_t* temp = action;
                int64_t actionOffset = readSLEB128(&temp);
                if (actionOffset == 0)
                {
                    // End of action list, no matching handler or cleanup found
                    results.reason = _URC_CONTINUE_UNWIND;
                    return;
                }
                // Go to next action
                action += actionOffset;
            }  // there is no break out of this loop, only return
        }
#ifndef __USING_SJLJ_EXCEPTIONS__
        else if (ipOffset < start)
        {
            // There is no call site for this ip
            // Something bad has happened.  We should never get here.
            // Possible stack corruption.
            call_terminate(native_exception, unwind_exception);
        }
#endif  // !__USING_SJLJ_EXCEPTIONS__
    }  // there might be some tricky cases which break out of this loop

    // It is possible that no eh table entry specify how to handle
    // this exception. By spec, terminate it immediately.
    call_terminate(native_exception, unwind_exception);
}

// public API

/*
The personality function branches on actions like so:

_UA_SEARCH_PHASE

    If _UA_CLEANUP_PHASE or _UA_HANDLER_FRAME or _UA_FORCE_UNWIND there's
      an error from above, return _URC_FATAL_PHASE1_ERROR.

    Scan for anything that could stop unwinding:

       1.  A catch clause that will catch this exception
           (will never catch foreign).
       2.  A catch (...) (will always catch foreign).
       3.  An exception spec that will catch this exception
           (will always catch foreign).
    If a handler is found
        If not foreign
            Save state in header
        return _URC_HANDLER_FOUND
    Else a handler not found
        return _URC_CONTINUE_UNWIND

_UA_CLEANUP_PHASE

    If _UA_HANDLER_FRAME
        If _UA_FORCE_UNWIND
            How did this happen?  return _URC_FATAL_PHASE2_ERROR
        If foreign
            Do _UA_SEARCH_PHASE to recover state
        else
            Recover state from header
        Transfer control to landing pad.  return _URC_INSTALL_CONTEXT
    
    Else

        This branch handles both normal C++ non-catching handlers (cleanups)
          and forced unwinding.    
        Scan for anything that can not stop unwinding:
    
            1.  A cleanup.

        If a cleanup is found
            transfer control to it. return _URC_INSTALL_CONTEXT
        Else a cleanup is not found: return _URC_CONTINUE_UNWIND
*/

#if !defined(_LIBCXXABI_ARM_EHABI)
#if defined(__SEH__) && !defined(__USING_SJLJ_EXCEPTIONS__)
static _Unwind_Reason_Code __gxx_personality_imp
#else
_LIBCXXABI_FUNC_VIS _Unwind_Reason_Code
#ifdef __USING_SJLJ_EXCEPTIONS__
__gxx_personality_sj0
#else
__gxx_personality_v0
#endif
#endif
                    (int version, _Unwind_Action actions, uint64_t exceptionClass,
                     _Unwind_Exception* unwind_exception, _Unwind_Context* context)
{
    if (version != 1 || unwind_exception == 0 || context == 0)
        return _URC_FATAL_PHASE1_ERROR;

    bool native_exception = (exceptionClass     & get_vendor_and_language) ==
                            (kOurExceptionClass & get_vendor_and_language);
    scan_results results;
    if (actions & _UA_SEARCH_PHASE)
    {
        // Phase 1 search:  All we're looking for in phase 1 is a handler that
        //   halts unwinding
        scan_eh_tab(results, actions, native_exception, unwind_exception, context);
        if (results.reason == _URC_HANDLER_FOUND)
        {
            // Found one.  Can we cache the results somewhere to optimize phase 2?
            if (native_exception)
            {
                __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception+1) - 1;
                exception_header->handlerSwitchValue = static_cast<int>(results.ttypeIndex);
                exception_header->actionRecord = results.actionRecord;
                exception_header->languageSpecificData = results.languageSpecificData;
                exception_header->catchTemp = reinterpret_cast<void*>(results.landingPad);
                exception_header->adjustedPtr = results.adjustedPtr;
            }
            return _URC_HANDLER_FOUND;
        }
        // Did not find a catching-handler.  Return the results of the scan
        //    (normally _URC_CONTINUE_UNWIND, but could have been _URC_FATAL_PHASE1_ERROR
        //     if we were called improperly).
        return results.reason;
    }
    if (actions & _UA_CLEANUP_PHASE)
    {
        // Phase 2 search:
        //  Did we find a catching handler in phase 1?
        if (actions & _UA_HANDLER_FRAME)
        {
            // Yes, phase 1 said we have a catching handler here.
            // Did we cache the results of the scan?
            if (native_exception)
            {
                // Yes, reload the results from the cache.
                __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception+1) - 1;
                results.ttypeIndex = exception_header->handlerSwitchValue;
                results.actionRecord = exception_header->actionRecord;
                results.languageSpecificData = exception_header->languageSpecificData;
                results.landingPad = reinterpret_cast<uintptr_t>(exception_header->catchTemp);
                results.adjustedPtr = exception_header->adjustedPtr;
            }
            else
            {
                // No, do the scan again to reload the results.
                scan_eh_tab(results, actions, native_exception, unwind_exception, context);
                // Phase 1 told us we would find a handler.  Now in Phase 2 we
                //   didn't find a handler.  The eh table should not be changing!
                if (results.reason != _URC_HANDLER_FOUND)
                    call_terminate(native_exception, unwind_exception);
            }
            // Jump to the handler
            set_registers(unwind_exception, context, results);
            return _URC_INSTALL_CONTEXT;
        }
        // Either we didn't do a phase 1 search (due to forced unwinding), or
        //   phase 1 reported no catching-handlers.
        // Search for a (non-catching) cleanup
        scan_eh_tab(results, actions, native_exception, unwind_exception, context);
        if (results.reason == _URC_HANDLER_FOUND)
        {
            // Found a non-catching handler.  Jump to it:
            set_registers(unwind_exception, context, results);
            return _URC_INSTALL_CONTEXT;
        }
        // Did not find a cleanup.  Return the results of the scan
        //    (normally _URC_CONTINUE_UNWIND, but could have been _URC_FATAL_PHASE2_ERROR
        //     if we were called improperly).
        return results.reason;
    }
    // We were called improperly: neither a phase 1 or phase 2 search
    return _URC_FATAL_PHASE1_ERROR;
}

#if defined(__SEH__) && !defined(__USING_SJLJ_EXCEPTIONS__)
extern "C" _LIBCXXABI_FUNC_VIS EXCEPTION_DISPOSITION
__gxx_personality_seh0(PEXCEPTION_RECORD ms_exc, void *this_frame,
                       PCONTEXT ms_orig_context, PDISPATCHER_CONTEXT ms_disp)
{
  return _GCC_specific_handler(ms_exc, this_frame, ms_orig_context, ms_disp,
                               __gxx_personality_imp);
}
#endif

#else

extern "C" _Unwind_Reason_Code __gnu_unwind_frame(_Unwind_Exception*,
                                                  _Unwind_Context*);

// Helper function to unwind one frame.
// ARM EHABI 7.3 and 7.4: If the personality function returns _URC_CONTINUE_UNWIND, the
// personality routine should update the virtual register set (VRS) according to the
// corresponding frame unwinding instructions (ARM EHABI 9.3.)
static _Unwind_Reason_Code continue_unwind(_Unwind_Exception* unwind_exception,
                                           _Unwind_Context* context)
{
    if (__gnu_unwind_frame(unwind_exception, context) != _URC_OK)
        return _URC_FAILURE;
    return _URC_CONTINUE_UNWIND;
}

// ARM register names
#if !defined(LIBCXXABI_USE_LLVM_UNWINDER)
static const uint32_t REG_UCB = 12;  // Register to save _Unwind_Control_Block
#endif
static const uint32_t REG_SP = 13;

static void save_results_to_barrier_cache(_Unwind_Exception* unwind_exception,
                                          const scan_results& results)
{
    unwind_exception->barrier_cache.bitpattern[0] = (uint32_t)results.adjustedPtr;
    unwind_exception->barrier_cache.bitpattern[1] = (uint32_t)results.actionRecord;
    unwind_exception->barrier_cache.bitpattern[2] = (uint32_t)results.languageSpecificData;
    unwind_exception->barrier_cache.bitpattern[3] = (uint32_t)results.landingPad;
    unwind_exception->barrier_cache.bitpattern[4] = (uint32_t)results.ttypeIndex;
}

static void load_results_from_barrier_cache(scan_results& results,
                                            const _Unwind_Exception* unwind_exception)
{
    results.adjustedPtr = (void*)unwind_exception->barrier_cache.bitpattern[0];
    results.actionRecord = (const uint8_t*)unwind_exception->barrier_cache.bitpattern[1];
    results.languageSpecificData = (const uint8_t*)unwind_exception->barrier_cache.bitpattern[2];
    results.landingPad = (uintptr_t)unwind_exception->barrier_cache.bitpattern[3];
    results.ttypeIndex = (int64_t)(int32_t)unwind_exception->barrier_cache.bitpattern[4];
}

extern "C" _LIBCXXABI_FUNC_VIS _Unwind_Reason_Code
__gxx_personality_v0(_Unwind_State state,
                     _Unwind_Exception* unwind_exception,
                     _Unwind_Context* context)
{
    if (unwind_exception == 0 || context == 0)
        return _URC_FATAL_PHASE1_ERROR;

    bool native_exception = __isOurExceptionClass(unwind_exception);

#if !defined(LIBCXXABI_USE_LLVM_UNWINDER)
    // Copy the address of _Unwind_Control_Block to r12 so that
    // _Unwind_GetLanguageSpecificData() and _Unwind_GetRegionStart() can
    // return correct address.
    _Unwind_SetGR(context, REG_UCB, reinterpret_cast<uint32_t>(unwind_exception));
#endif

    // Check the undocumented force unwinding behavior
    bool is_force_unwinding = state & _US_FORCE_UNWIND;
    state &= ~_US_FORCE_UNWIND;

    scan_results results;
    switch (state) {
    case _US_VIRTUAL_UNWIND_FRAME:
        if (is_force_unwinding)
            return continue_unwind(unwind_exception, context);

        // Phase 1 search:  All we're looking for in phase 1 is a handler that halts unwinding
        scan_eh_tab(results, _UA_SEARCH_PHASE, native_exception, unwind_exception, context);
        if (results.reason == _URC_HANDLER_FOUND)
        {
            unwind_exception->barrier_cache.sp = _Unwind_GetGR(context, REG_SP);
            if (native_exception)
                save_results_to_barrier_cache(unwind_exception, results);
            return _URC_HANDLER_FOUND;
        }
        // Did not find the catch handler
        if (results.reason == _URC_CONTINUE_UNWIND)
            return continue_unwind(unwind_exception, context);
        return results.reason;

    case _US_UNWIND_FRAME_STARTING:
        // TODO: Support force unwinding in the phase 2 search.
        // NOTE: In order to call the cleanup functions, _Unwind_ForcedUnwind()
        // will call this personality function with (_US_FORCE_UNWIND |
        // _US_UNWIND_FRAME_STARTING).

        // Phase 2 search
        if (unwind_exception->barrier_cache.sp == _Unwind_GetGR(context, REG_SP))
        {
            // Found a catching handler in phase 1
            if (native_exception)
            {
                // Load the result from the native exception barrier cache.
                load_results_from_barrier_cache(results, unwind_exception);
                results.reason = _URC_HANDLER_FOUND;
            }
            else
            {
                // Search for the catching handler again for the foreign exception.
                scan_eh_tab(results, static_cast<_Unwind_Action>(_UA_CLEANUP_PHASE | _UA_HANDLER_FRAME),
                            native_exception, unwind_exception, context);
                if (results.reason != _URC_HANDLER_FOUND)  // phase1 search should guarantee to find one
                    call_terminate(native_exception, unwind_exception);
            }

            // Install the context for the catching handler
            set_registers(unwind_exception, context, results);
            return _URC_INSTALL_CONTEXT;
        }

        // Either we didn't do a phase 1 search (due to forced unwinding), or
        //  phase 1 reported no catching-handlers.
        // Search for a (non-catching) cleanup
        scan_eh_tab(results, _UA_CLEANUP_PHASE, native_exception, unwind_exception, context);
        if (results.reason == _URC_HANDLER_FOUND)
        {
            // Found a non-catching handler

            // ARM EHABI 8.4.2: Before we can jump to the cleanup handler, we have to setup some
            // internal data structures, so that __cxa_end_cleanup() can get unwind_exception from
            // __cxa_get_globals().
            __cxa_begin_cleanup(unwind_exception);

            // Install the context for the cleanup handler
            set_registers(unwind_exception, context, results);
            return _URC_INSTALL_CONTEXT;
        }

        // Did not find any handler
        if (results.reason == _URC_CONTINUE_UNWIND)
            return continue_unwind(unwind_exception, context);
        return results.reason;

    case _US_UNWIND_FRAME_RESUME:
        return continue_unwind(unwind_exception, context);
    }

    // We were called improperly: neither a phase 1 or phase 2 search
    return _URC_FATAL_PHASE1_ERROR;
}
#endif


__attribute__((noreturn))
_LIBCXXABI_FUNC_VIS void
__cxa_call_unexpected(void* arg)
{
    _Unwind_Exception* unwind_exception = static_cast<_Unwind_Exception*>(arg);
    if (unwind_exception == 0)
        call_terminate(false, unwind_exception);
    __cxa_begin_catch(unwind_exception);
    bool native_old_exception = __isOurExceptionClass(unwind_exception);
    std::unexpected_handler u_handler;
    std::terminate_handler t_handler;
    __cxa_exception* old_exception_header = 0;
    int64_t ttypeIndex;
    const uint8_t* lsda;
    if (native_old_exception)
    {
        old_exception_header = (__cxa_exception*)(unwind_exception+1) - 1;
        t_handler = old_exception_header->terminateHandler;
        u_handler = old_exception_header->unexpectedHandler;
        // If std::__unexpected(u_handler) rethrows the same exception,
        //   these values get overwritten by the rethrow.  So save them now:
#if defined(_LIBCXXABI_ARM_EHABI)
        ttypeIndex = (int64_t)(int32_t)unwind_exception->barrier_cache.bitpattern[4];
        lsda = (const uint8_t*)unwind_exception->barrier_cache.bitpattern[2];
#else
        ttypeIndex = old_exception_header->handlerSwitchValue;
        lsda = old_exception_header->languageSpecificData;
#endif
    }
    else
    {
        t_handler = std::get_terminate();
        u_handler = std::get_unexpected();
    }
    try
    {
        std::__unexpected(u_handler);
    }
    catch (...)
    {
        // If the old exception is foreign, then all we can do is terminate.
        //   We have no way to recover the needed old exception spec.  There's
        //   no way to pass that information here.  And the personality routine
        //   can't call us directly and do anything but terminate() if we throw
        //   from here.
        if (native_old_exception)
        {
            // Have:
            //   old_exception_header->languageSpecificData
            //   old_exception_header->actionRecord
            // Need
            //   const uint8_t* classInfo
            //   uint8_t ttypeEncoding
            uint8_t lpStartEncoding = *lsda++;
            const uint8_t* lpStart = (const uint8_t*)readEncodedPointer(&lsda, lpStartEncoding);
            (void)lpStart;  // purposefully unused.  Just needed to increment lsda.
            uint8_t ttypeEncoding = *lsda++;
            if (ttypeEncoding == DW_EH_PE_omit)
                std::__terminate(t_handler);
            uintptr_t classInfoOffset = readULEB128(&lsda);
            const uint8_t* classInfo = lsda + classInfoOffset;
            // Is this new exception catchable by the exception spec at ttypeIndex?
            // The answer is obviously yes if the new and old exceptions are the same exception
            // If no
            //    throw;
            __cxa_eh_globals* globals = __cxa_get_globals_fast();
            __cxa_exception* new_exception_header = globals->caughtExceptions;
            if (new_exception_header == 0)
                // This shouldn't be able to happen!
                std::__terminate(t_handler);
            bool native_new_exception = __isOurExceptionClass(&new_exception_header->unwindHeader);
            void* adjustedPtr;
            if (native_new_exception && (new_exception_header != old_exception_header))
            {
                const __shim_type_info* excpType =
                    static_cast<const __shim_type_info*>(new_exception_header->exceptionType);
                adjustedPtr =
                    __getExceptionClass(&new_exception_header->unwindHeader) == kOurDependentExceptionClass ?
                        ((__cxa_dependent_exception*)new_exception_header)->primaryException :
                        new_exception_header + 1;
                if (!exception_spec_can_catch(ttypeIndex, classInfo, ttypeEncoding,
                                              excpType, adjustedPtr, unwind_exception))
                {
                    // We need to __cxa_end_catch, but for the old exception,
                    //   not the new one.  This is a little tricky ...
                    // Disguise new_exception_header as a rethrown exception, but
                    //   don't actually rethrow it.  This means you can temporarily
                    //   end the catch clause enclosing new_exception_header without
                    //   __cxa_end_catch destroying new_exception_header.
                    new_exception_header->handlerCount = -new_exception_header->handlerCount;
                    globals->uncaughtExceptions += 1;
                    // Call __cxa_end_catch for new_exception_header
                    __cxa_end_catch();
                    // Call __cxa_end_catch for old_exception_header
                    __cxa_end_catch();
                    // Renter this catch clause with new_exception_header
                    __cxa_begin_catch(&new_exception_header->unwindHeader);
                    // Rethrow new_exception_header
                    throw;
                }
            }
            // Will a std::bad_exception be catchable by the exception spec at
            //   ttypeIndex?
            // If no
            //    throw std::bad_exception();
            const __shim_type_info* excpType =
                static_cast<const __shim_type_info*>(&typeid(std::bad_exception));
            std::bad_exception be;
            adjustedPtr = &be;
            if (!exception_spec_can_catch(ttypeIndex, classInfo, ttypeEncoding,
                                          excpType, adjustedPtr, unwind_exception))
            {
                // We need to __cxa_end_catch for both the old exception and the
                //   new exception.  Technically we should do it in that order.
                //   But it is expedient to do it in the opposite order:
                // Call __cxa_end_catch for new_exception_header
                __cxa_end_catch();
                // Throw std::bad_exception will __cxa_end_catch for
                //   old_exception_header
                throw be;
            }
        }
    }
    std::__terminate(t_handler);
}

}  // extern "C"

}  // __cxxabiv1
