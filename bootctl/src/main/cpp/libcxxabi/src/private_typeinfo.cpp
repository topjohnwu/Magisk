//===----------------------- private_typeinfo.cpp -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "private_typeinfo.h"

// The flag _LIBCXX_DYNAMIC_FALLBACK is used to make dynamic_cast more
// forgiving when type_info's mistakenly have hidden visibility and thus
// multiple type_infos can exist for a single type.
// 
// When _LIBCXX_DYNAMIC_FALLBACK is defined, and only in the case where
// there is a detected inconsistency in the type_info hierarchy during a
// dynamic_cast, then the equality operation will fall back to using strcmp
// on type_info names to determine type_info equality.
// 
// This change happens *only* under dynamic_cast, and only when
// dynamic_cast is faced with the choice:  abort, or possibly give back the
// wrong answer.  If when the dynamic_cast is done with this fallback
// algorithm and an inconsistency is still detected, dynamic_cast will call
// abort with an appropriate message.
// 
// The current implementation of _LIBCXX_DYNAMIC_FALLBACK requires a
// printf-like function called syslog:
// 
//     void syslog(int facility_priority, const char* format, ...);
// 
// If you want this functionality but your platform doesn't have syslog,
// just implement it in terms of fprintf(stderr, ...).
// 
// _LIBCXX_DYNAMIC_FALLBACK is currently off by default.


#include <string.h>


#ifdef _LIBCXX_DYNAMIC_FALLBACK
#include "abort_message.h"
#include <sys/syslog.h>
#endif

// On Windows, typeids are different between DLLs and EXEs, so comparing
// type_info* will work for typeids from the same compiled file but fail
// for typeids from a DLL and an executable. Among other things, exceptions
// are not caught by handlers since can_catch() returns false.
//
// Defining _LIBCXX_DYNAMIC_FALLBACK does not help since can_catch() calls 
// is_equal() with use_strcmp=false so the string names are not compared.

#ifdef _WIN32
#include <string.h>
#endif

static inline
bool
is_equal(const std::type_info* x, const std::type_info* y, bool use_strcmp)
{
#ifndef _WIN32
    if (!use_strcmp)
        return x == y;
    return strcmp(x->name(), y->name()) == 0;
#else
    (void) use_strcmp;
    return (x == y) || (strcmp(x->name(), y->name()) == 0);
#endif
}

namespace __cxxabiv1
{

// __shim_type_info

__shim_type_info::~__shim_type_info()
{
}

void __shim_type_info::noop1() const {}
void __shim_type_info::noop2() const {}

// __fundamental_type_info

// This miraculously (compiler magic) emits the type_info's for:
//   1. all of the fundamental types
//   2. pointers to all of the fundamental types
//   3. pointers to all of the const fundamental types
__fundamental_type_info::~__fundamental_type_info()
{
}

// __array_type_info

__array_type_info::~__array_type_info()
{
}

// __function_type_info

__function_type_info::~__function_type_info()
{
}

// __enum_type_info

__enum_type_info::~__enum_type_info()
{
}

// __class_type_info

__class_type_info::~__class_type_info()
{
}

// __si_class_type_info

__si_class_type_info::~__si_class_type_info()
{
}

// __vmi_class_type_info

__vmi_class_type_info::~__vmi_class_type_info()
{
}

// __pbase_type_info

__pbase_type_info::~__pbase_type_info()
{
}

// __pointer_type_info

__pointer_type_info::~__pointer_type_info()
{
}

// __pointer_to_member_type_info

__pointer_to_member_type_info::~__pointer_to_member_type_info()
{
}

// can_catch

// A handler is a match for an exception object of type E if
//   1. The handler is of type cv T or cv T& and E and T are the same type
//      (ignoring the top-level cv-qualifiers), or
//   2. the handler is of type cv T or cv T& and T is an unambiguous public
//       base class of E, or
//   3. the handler is of type cv1 T* cv2 and E is a pointer type that can be
//      converted to the type of the handler by either or both of
//      A. a standard pointer conversion (4.10) not involving conversions to
//         pointers to private or protected or ambiguous classes
//      B. a qualification conversion
//   4. the handler is a pointer or pointer to member type and E is
//      std::nullptr_t.

// adjustedPtr:
// 
// catch (A& a) : adjustedPtr == &a
// catch (A* a) : adjustedPtr == a
// catch (A** a) : adjustedPtr == a
// 
// catch (D2& d2) : adjustedPtr == &d2  (d2 is base class of thrown object)
// catch (D2* d2) : adjustedPtr == d2
// catch (D2*& d2) : adjustedPtr == d2
//
// catch (...) : adjustedPtr == & of the exception
//
// If the thrown type is nullptr_t and the caught type is a pointer to
// member type, adjustedPtr points to a statically-allocated null pointer
// representation of that type.

// Handles bullet 1
bool
__fundamental_type_info::can_catch(const __shim_type_info* thrown_type,
                                   void*&) const
{
    return is_equal(this, thrown_type, false);
}

bool
__array_type_info::can_catch(const __shim_type_info*, void*&) const
{
    // We can get here if someone tries to catch an array by reference.
    //   However if someone tries to throw an array, it immediately gets
    //   converted to a pointer, which will not convert back to an array
    //   at the catch clause.  So this can never catch anything.
    return false;
}

bool
__function_type_info::can_catch(const __shim_type_info*, void*&) const
{
    // We can get here if someone tries to catch a function by reference.
    //   However if someone tries to throw a function, it immediately gets
    //   converted to a pointer, which will not convert back to a function
    //   at the catch clause.  So this can never catch anything.
    return false;
}

// Handles bullet 1
bool
__enum_type_info::can_catch(const __shim_type_info* thrown_type,
                            void*&) const
{
    return is_equal(this, thrown_type, false);
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

// Handles bullets 1 and 2
bool
__class_type_info::can_catch(const __shim_type_info* thrown_type,
                             void*& adjustedPtr) const
{
    // bullet 1
    if (is_equal(this, thrown_type, false))
        return true;
    const __class_type_info* thrown_class_type =
        dynamic_cast<const __class_type_info*>(thrown_type);
    if (thrown_class_type == 0)
        return false;
    // bullet 2
    __dynamic_cast_info info = {thrown_class_type, 0, this, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
    info.number_of_dst_type = 1;
    thrown_class_type->has_unambiguous_public_base(&info, adjustedPtr, public_path);
    if (info.path_dst_ptr_to_static_ptr == public_path)
    {
        adjustedPtr = const_cast<void*>(info.dst_ptr_leading_to_static_ptr);
        return true;
    }
    return false;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

void
__class_type_info::process_found_base_class(__dynamic_cast_info* info,
                                               void* adjustedPtr,
                                               int path_below) const
{
    if (info->dst_ptr_leading_to_static_ptr == 0)
    {
        // First time here
        info->dst_ptr_leading_to_static_ptr = adjustedPtr;
        info->path_dst_ptr_to_static_ptr = path_below;
        info->number_to_static_ptr = 1;
    }
    else if (info->dst_ptr_leading_to_static_ptr == adjustedPtr)
    {
        // We've been here before.  Update path to "most public"
        if (info->path_dst_ptr_to_static_ptr == not_public_path)
            info->path_dst_ptr_to_static_ptr = path_below;
    }
    else
    {
        // We've detected an ambiguous cast from (thrown_class_type, adjustedPtr)
        //   to a static_type
        info->number_to_static_ptr += 1;
        info->path_dst_ptr_to_static_ptr = not_public_path;
        info->search_done = true;
    }
}

void
__class_type_info::has_unambiguous_public_base(__dynamic_cast_info* info,
                                               void* adjustedPtr,
                                               int path_below) const
{
    if (is_equal(this, info->static_type, false))
        process_found_base_class(info, adjustedPtr, path_below);
}

void
__si_class_type_info::has_unambiguous_public_base(__dynamic_cast_info* info,
                                                  void* adjustedPtr,
                                                  int path_below) const
{
    if (is_equal(this, info->static_type, false))
        process_found_base_class(info, adjustedPtr, path_below);
    else
        __base_type->has_unambiguous_public_base(info, adjustedPtr, path_below);
}

void
__base_class_type_info::has_unambiguous_public_base(__dynamic_cast_info* info,
                                                    void* adjustedPtr,
                                                    int path_below) const
{
    ptrdiff_t offset_to_base = 0;
    if (adjustedPtr != nullptr)
    {
        offset_to_base = __offset_flags >> __offset_shift;
        if (__offset_flags & __virtual_mask)
        {
            const char* vtable = *static_cast<const char*const*>(adjustedPtr);
            offset_to_base = *reinterpret_cast<const ptrdiff_t*>(vtable + offset_to_base);
        }
    }
    __base_type->has_unambiguous_public_base(
            info,
            static_cast<char*>(adjustedPtr) + offset_to_base,
            (__offset_flags & __public_mask) ? path_below : not_public_path);
}

void
__vmi_class_type_info::has_unambiguous_public_base(__dynamic_cast_info* info,
                                                   void* adjustedPtr,
                                                   int path_below) const
{
    if (is_equal(this, info->static_type, false))
        process_found_base_class(info, adjustedPtr, path_below);
    else
    {
        typedef const __base_class_type_info* Iter;
        const Iter e = __base_info + __base_count;
        Iter p = __base_info;
        p->has_unambiguous_public_base(info, adjustedPtr, path_below);
        if (++p < e)
        {
            do
            {
                p->has_unambiguous_public_base(info, adjustedPtr, path_below);
                if (info->search_done)
                    break;
            } while (++p < e);
        }
    }
}

// Handles bullet 1 for both pointers and member pointers
bool
__pbase_type_info::can_catch(const __shim_type_info* thrown_type,
                             void*&) const
{
    bool use_strcmp = this->__flags & (__incomplete_class_mask |
                                       __incomplete_mask);
    if (!use_strcmp) {
        const __pbase_type_info* thrown_pbase = dynamic_cast<const __pbase_type_info*>(
                thrown_type);
        if (!thrown_pbase) return false;
        use_strcmp = thrown_pbase->__flags & (__incomplete_class_mask |
                                              __incomplete_mask);
    }
    return is_equal(this, thrown_type, use_strcmp);
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

// Handles bullets 1, 3 and 4
// NOTE: It might not be safe to adjust the pointer if it is not not a pointer
// type. Only adjust the pointer after we know it is safe to do so.
bool
__pointer_type_info::can_catch(const __shim_type_info* thrown_type,
                               void*& adjustedPtr) const
{
    // bullet 4
    if (is_equal(thrown_type, &typeid(std::nullptr_t), false)) {
      adjustedPtr = nullptr;
      return true;
    }

    // bullet 1
    if (__pbase_type_info::can_catch(thrown_type, adjustedPtr)) {
        if (adjustedPtr != NULL)
            adjustedPtr = *static_cast<void**>(adjustedPtr);
        return true;
    }
    // bullet 3
    const __pointer_type_info* thrown_pointer_type =
        dynamic_cast<const __pointer_type_info*>(thrown_type);
    if (thrown_pointer_type == 0)
        return false;
    // Do the dereference adjustment
    if (adjustedPtr != NULL)
        adjustedPtr = *static_cast<void**>(adjustedPtr);
    // bullet 3B and 3C
    if (thrown_pointer_type->__flags & ~__flags & __no_remove_flags_mask)
        return false;
    if (__flags & ~thrown_pointer_type->__flags & __no_add_flags_mask)
        return false;
    if (is_equal(__pointee, thrown_pointer_type->__pointee, false))
        return true;
    // bullet 3A
    if (is_equal(__pointee, &typeid(void), false)) {
        // pointers to functions cannot be converted to void*.
        // pointers to member functions are not handled here.
        const __function_type_info* thrown_function =
            dynamic_cast<const __function_type_info*>(thrown_pointer_type->__pointee);
        return (thrown_function == nullptr);
    }
    // Handle pointer to pointer
    const __pointer_type_info* nested_pointer_type =
        dynamic_cast<const __pointer_type_info*>(__pointee);
    if (nested_pointer_type) {
        if (~__flags & __const_mask) return false;
        return nested_pointer_type->can_catch_nested(thrown_pointer_type->__pointee);
    }

    // Handle pointer to pointer to member
    const __pointer_to_member_type_info* member_ptr_type =
        dynamic_cast<const __pointer_to_member_type_info*>(__pointee);
    if (member_ptr_type) {
        if (~__flags & __const_mask) return false;
        return member_ptr_type->can_catch_nested(thrown_pointer_type->__pointee);
    }

    // Handle pointer to class type
    const __class_type_info* catch_class_type =
        dynamic_cast<const __class_type_info*>(__pointee);
    if (catch_class_type == 0)
        return false;
    const __class_type_info* thrown_class_type =
        dynamic_cast<const __class_type_info*>(thrown_pointer_type->__pointee);
    if (thrown_class_type == 0)
        return false;
    __dynamic_cast_info info = {thrown_class_type, 0, catch_class_type, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
    info.number_of_dst_type = 1;
    thrown_class_type->has_unambiguous_public_base(&info, adjustedPtr, public_path);
    if (info.path_dst_ptr_to_static_ptr == public_path)
    {
        if (adjustedPtr != NULL)
            adjustedPtr = const_cast<void*>(info.dst_ptr_leading_to_static_ptr);
        return true;
    }
    return false;
}

bool __pointer_type_info::can_catch_nested(
    const __shim_type_info* thrown_type) const
{
  const __pointer_type_info* thrown_pointer_type =
        dynamic_cast<const __pointer_type_info*>(thrown_type);
    if (thrown_pointer_type == 0)
        return false;
    // bullet 3B
    if (thrown_pointer_type->__flags & ~__flags)
        return false;
    if (is_equal(__pointee, thrown_pointer_type->__pointee, false))
        return true;
    // If the pointed to types differ then the catch type must be const
    // qualified.
    if (~__flags & __const_mask)
        return false;

    // Handle pointer to pointer
    const __pointer_type_info* nested_pointer_type =
        dynamic_cast<const __pointer_type_info*>(__pointee);
    if (nested_pointer_type) {
        return nested_pointer_type->can_catch_nested(
            thrown_pointer_type->__pointee);
    }

    // Handle pointer to pointer to member
    const __pointer_to_member_type_info* member_ptr_type =
        dynamic_cast<const __pointer_to_member_type_info*>(__pointee);
    if (member_ptr_type) {
        return member_ptr_type->can_catch_nested(thrown_pointer_type->__pointee);
    }

    return false;
}

bool __pointer_to_member_type_info::can_catch(
    const __shim_type_info* thrown_type, void*& adjustedPtr) const {
    // bullet 4
    if (is_equal(thrown_type, &typeid(std::nullptr_t), false)) {
      // We assume that the pointer to member representation is the same for
      // all pointers to data members and for all pointers to member functions.
      struct X {};
      if (dynamic_cast<const __function_type_info*>(__pointee)) {
        static int (X::*const null_ptr_rep)() = nullptr;
        adjustedPtr = const_cast<int (X::**)()>(&null_ptr_rep);
      } else {
        static int X::*const null_ptr_rep = nullptr;
        adjustedPtr = const_cast<int X::**>(&null_ptr_rep);
      }
      return true;
    }

    // bullet 1
    if (__pbase_type_info::can_catch(thrown_type, adjustedPtr))
        return true;

    const __pointer_to_member_type_info* thrown_pointer_type =
        dynamic_cast<const __pointer_to_member_type_info*>(thrown_type);
    if (thrown_pointer_type == 0)
        return false;
    if (thrown_pointer_type->__flags & ~__flags & __no_remove_flags_mask)
        return false;
    if (__flags & ~thrown_pointer_type->__flags & __no_add_flags_mask)
        return false;
    if (!is_equal(__pointee, thrown_pointer_type->__pointee, false))
        return false;
    if (is_equal(__context, thrown_pointer_type->__context, false))
        return true;

    // [except.handle] does not allow the pointer-to-member conversions mentioned
    // in [mem.conv] to take place. For this reason we don't check Derived->Base
    // for Derived->Base conversions.

    return false;
}

bool __pointer_to_member_type_info::can_catch_nested(
    const __shim_type_info* thrown_type) const
{
    const __pointer_to_member_type_info* thrown_member_ptr_type =
        dynamic_cast<const __pointer_to_member_type_info*>(thrown_type);
    if (thrown_member_ptr_type == 0)
        return false;
    if (~__flags & thrown_member_ptr_type->__flags)
        return false;
    if (!is_equal(__pointee, thrown_member_ptr_type->__pointee, false))
        return false;
    if (!is_equal(__context, thrown_member_ptr_type->__context, false))
        return false;
    return true;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

// __dynamic_cast

// static_ptr: pointer to an object of type static_type; nonnull, and since the
//   object is polymorphic, *(void**)static_ptr is a virtual table pointer.
//   static_ptr is &v in the expression dynamic_cast<T>(v).
// static_type: static type of the object pointed to by static_ptr.
// dst_type: destination type of the cast (the "T" in "dynamic_cast<T>(v)").
// src2dst_offset: a static hint about the location of the
//                 source subobject with respect to the complete object;
//                 special negative values are:
//                     -1: no hint
//                     -2: static_type is not a public base of dst_type
//                     -3: static_type is a multiple public base type but never a
//                         virtual base type
//                 otherwise, the static_type type is a unique public nonvirtual
//                 base type of dst_type at offset src2dst_offset from the
//                 origin of dst_type.
//
// (dynamic_ptr, dynamic_type) are the run time type of the complete object
// referred to by static_ptr and a pointer to it.  These can be found from
// static_ptr for polymorphic types.
// static_type is guaranteed to be a polymorphic type.
//
// (dynamic_ptr, dynamic_type) is the root of a DAG that grows upward.  Each
// node of the tree represents a base class/object of its parent (or parents) below.
// Each node is uniquely represented by a pointer to the object, and a pointer
// to a type_info - its type.  Different nodes may have the same pointer and
// different nodes may have the same type.  But only one node has a specific
// (pointer-value, type) pair.  In C++ two objects of the same type can not
// share the same address.
//
// There are two flavors of nodes which have the type dst_type:
//    1.  Those that are derived from (below) (static_ptr, static_type).
//    2.  Those that are not derived from (below) (static_ptr, static_type).
//
// Invariants of the DAG:
//
// There is at least one path from the root (dynamic_ptr, dynamic_type) to
// the node (static_ptr, static_type).  This path may or may not be public.
// There may be more than one such path (some public some not).  Such a path may
// or may not go through a node having type dst_type.
//
// No node of type T appears above a node of the same type.  That means that
// there is only one node with dynamic_type.  And if dynamic_type == dst_type,
// then there is only one dst_type in the DAG.
//
// No node of type dst_type appears above a node of type static_type.  Such
// DAG's are possible in C++, but the compiler computes those dynamic_casts at
// compile time, and only calls __dynamic_cast when dst_type lies below
// static_type in the DAG.
//
// dst_type != static_type:  The compiler computes the dynamic_cast in this case too.
// dynamic_type != static_type:  The compiler computes the dynamic_cast in this case too.
//
// Returns:
//
// If there is exactly one dst_type of flavor 1, and
//    If there is a public path from that dst_type to (static_ptr, static_type), or
//    If there are 0 dst_types of flavor 2, and there is a public path from
//        (dynamic_ptr, dynamic_type) to (static_ptr, static_type) and a public
//        path from (dynamic_ptr, dynamic_type) to the one dst_type, then return
//        a pointer to that dst_type.
// Else if there are 0 dst_types of flavor 1 and exactly 1 dst_type of flavor 2, and
//    if there is a public path from (dynamic_ptr, dynamic_type) to
//    (static_ptr, static_type) and a public path from (dynamic_ptr, dynamic_type)
//    to the one dst_type, then return a pointer to that one dst_type.
// Else return nullptr.
//
// If dynamic_type == dst_type, then the above algorithm collapses to the
// following cheaper algorithm:
//
// If there is a public path from (dynamic_ptr, dynamic_type) to
//    (static_ptr, static_type), then return dynamic_ptr.
// Else return nullptr.

extern "C" _LIBCXXABI_FUNC_VIS void *
__dynamic_cast(const void *static_ptr, const __class_type_info *static_type,
               const __class_type_info *dst_type,
               std::ptrdiff_t src2dst_offset) {
    // Possible future optimization:  Take advantage of src2dst_offset
    // Currently clang always sets src2dst_offset to -1 (no hint).

    // Get (dynamic_ptr, dynamic_type) from static_ptr
    void **vtable = *static_cast<void ** const *>(static_ptr);
    ptrdiff_t offset_to_derived = reinterpret_cast<ptrdiff_t>(vtable[-2]);
    const void* dynamic_ptr = static_cast<const char*>(static_ptr) + offset_to_derived;
    const __class_type_info* dynamic_type = static_cast<const __class_type_info*>(vtable[-1]);

    // Initialize answer to nullptr.  This will be changed from the search
    //    results if a non-null answer is found.  Regardless, this is what will
    //    be returned.
    const void* dst_ptr = 0;
    // Initialize info struct for this search.
    __dynamic_cast_info info = {dst_type, static_ptr, static_type, src2dst_offset, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};

    // Find out if we can use a giant short cut in the search
    if (is_equal(dynamic_type, dst_type, false))
    {
        // Using giant short cut.  Add that information to info.
        info.number_of_dst_type = 1;
        // Do the  search
        dynamic_type->search_above_dst(&info, dynamic_ptr, dynamic_ptr, public_path, false);
#ifdef _LIBCXX_DYNAMIC_FALLBACK
        // The following if should always be false because we should definitely
        //   find (static_ptr, static_type), either on a public or private path
        if (info.path_dst_ptr_to_static_ptr == unknown)
        {
            // We get here only if there is some kind of visibility problem
            //   in client code.
            syslog(LOG_ERR, "dynamic_cast error 1: Both of the following type_info's "
                    "should have public visibility. At least one of them is hidden. %s"
                    ", %s.\n", static_type->name(), dynamic_type->name());
            // Redo the search comparing type_info's using strcmp
            info = {dst_type, static_ptr, static_type, src2dst_offset, 0};
            info.number_of_dst_type = 1;
            dynamic_type->search_above_dst(&info, dynamic_ptr, dynamic_ptr, public_path, true);
        }
#endif  // _LIBCXX_DYNAMIC_FALLBACK
        // Query the search.
        if (info.path_dst_ptr_to_static_ptr == public_path)
            dst_ptr = dynamic_ptr;
    }
    else
    {
        // Not using giant short cut.  Do the search
        dynamic_type->search_below_dst(&info, dynamic_ptr, public_path, false);
 #ifdef _LIBCXX_DYNAMIC_FALLBACK
        // The following if should always be false because we should definitely
        //   find (static_ptr, static_type), either on a public or private path
        if (info.path_dst_ptr_to_static_ptr == unknown &&
            info.path_dynamic_ptr_to_static_ptr == unknown)
        {
            syslog(LOG_ERR, "dynamic_cast error 2: One or more of the following type_info's "
                            "has hidden visibility or is defined in more than one translation "
                            "unit. They should all have public visibility. "
                            "%s, %s, %s.\n", static_type->name(), dynamic_type->name(),
                    dst_type->name());
            // Redo the search comparing type_info's using strcmp
            info = {dst_type, static_ptr, static_type, src2dst_offset, 0};
            dynamic_type->search_below_dst(&info, dynamic_ptr, public_path, true);
        }
#endif  // _LIBCXX_DYNAMIC_FALLBACK
        // Query the search.
        switch (info.number_to_static_ptr)
        {
        case 0:
            if (info.number_to_dst_ptr == 1 &&
                    info.path_dynamic_ptr_to_static_ptr == public_path &&
                    info.path_dynamic_ptr_to_dst_ptr == public_path)
                dst_ptr = info.dst_ptr_not_leading_to_static_ptr;
            break;
        case 1:
            if (info.path_dst_ptr_to_static_ptr == public_path ||
                   (
                       info.number_to_dst_ptr == 0 &&
                       info.path_dynamic_ptr_to_static_ptr == public_path &&
                       info.path_dynamic_ptr_to_dst_ptr == public_path
                   )
               )
                dst_ptr = info.dst_ptr_leading_to_static_ptr;
            break;
        }
    }
    return const_cast<void*>(dst_ptr);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

// Call this function when you hit a static_type which is a base (above) a dst_type.
// Let caller know you hit a static_type.  But only start recording details if
// this is (static_ptr, static_type) -- the node we are casting from.
// If this is (static_ptr, static_type)
//   Record the path (public or not) from the dst_type to here.  There may be
//   multiple paths from the same dst_type to here, record the "most public" one.
//   Record the dst_ptr as pointing to (static_ptr, static_type).
//   If more than one (dst_ptr, dst_type) points to (static_ptr, static_type),
//   then mark this dyanmic_cast as ambiguous and stop the search.
void
__class_type_info::process_static_type_above_dst(__dynamic_cast_info* info,
                                                 const void* dst_ptr,
                                                 const void* current_ptr,
                                                 int path_below) const
{
    // Record that we found a static_type
    info->found_any_static_type = true;
    if (current_ptr == info->static_ptr)
    {
        // Record that we found (static_ptr, static_type)
        info->found_our_static_ptr = true;
        if (info->dst_ptr_leading_to_static_ptr == 0)
        {
            // First time here
            info->dst_ptr_leading_to_static_ptr = dst_ptr;
            info->path_dst_ptr_to_static_ptr = path_below;
            info->number_to_static_ptr = 1;
            // If there is only one dst_type in the entire tree and the path from
            //    there to here is public then we are done!
            if (info->number_of_dst_type == 1 && info->path_dst_ptr_to_static_ptr == public_path)
                info->search_done = true;
        }
        else if (info->dst_ptr_leading_to_static_ptr == dst_ptr)
        {
            // We've been here before.  Update path to "most public"
            if (info->path_dst_ptr_to_static_ptr == not_public_path)
                info->path_dst_ptr_to_static_ptr = path_below;
            // If there is only one dst_type in the entire tree and the path from
            //    there to here is public then we are done!
            if (info->number_of_dst_type == 1 && info->path_dst_ptr_to_static_ptr == public_path)
                info->search_done = true;
        }
        else
        {
            // We've detected an ambiguous cast from (static_ptr, static_type)
            //   to a dst_type
            info->number_to_static_ptr += 1;
            info->search_done = true;
        }
    }
}

// Call this function when you hit a static_type which is not a base (above) a dst_type.
// If this is (static_ptr, static_type)
//   Record the path (public or not) from (dynamic_ptr, dynamic_type) to here.  There may be
//   multiple paths from (dynamic_ptr, dynamic_type) to here, record the "most public" one.
void
__class_type_info::process_static_type_below_dst(__dynamic_cast_info* info,
                                                 const void* current_ptr,
                                                 int path_below) const
{
    if (current_ptr == info->static_ptr)
    {
        // Record the most public path from (dynamic_ptr, dynamic_type) to
        //                                  (static_ptr, static_type)
        if (info->path_dynamic_ptr_to_static_ptr != public_path)
            info->path_dynamic_ptr_to_static_ptr = path_below;
    }
}

// Call this function when searching below a dst_type node.  This function searches
// for a path to (static_ptr, static_type) and for paths to one or more dst_type nodes.
// If it finds a static_type node, there is no need to further search base classes
// above.
// If it finds a dst_type node it should search base classes using search_above_dst
// to find out if this dst_type points to (static_ptr, static_type) or not.
// Either way, the dst_type is recorded as one of two "flavors":  one that does
// or does not point to (static_ptr, static_type).
// If this is neither a static_type nor a dst_type node, continue searching
// base classes above.
// All the hoopla surrounding the search code is doing nothing but looking for
// excuses to stop the search prematurely (break out of the for-loop).  That is,
// the algorithm below is simply an optimization of this:
// void
// __vmi_class_type_info::search_below_dst(__dynamic_cast_info* info,
//                                         const void* current_ptr,
//                                         int path_below) const
// {
//     typedef const __base_class_type_info* Iter;
//     if (this == info->static_type)
//         process_static_type_below_dst(info, current_ptr, path_below);
//     else if (this == info->dst_type)
//     {
//         // Record the most public access path that got us here
//         if (info->path_dynamic_ptr_to_dst_ptr != public_path)
//             info->path_dynamic_ptr_to_dst_ptr = path_below;
//         bool does_dst_type_point_to_our_static_type = false;
//         for (Iter p = __base_info, e= __base_info + __base_count; p < e; ++p)
//         {
//             p->search_above_dst(info, current_ptr, current_ptr, public_path);
//             if (info->found_our_static_ptr)
//                 does_dst_type_point_to_our_static_type = true;
//             // break out early here if you can detect it doesn't matter if you do
//         }
//         if (!does_dst_type_point_to_our_static_type)
//         {
//             // We found a dst_type that doesn't point to (static_ptr, static_type)
//             // So record the address of this dst_ptr and increment the
//             // count of the number of such dst_types found in the tree.
//             info->dst_ptr_not_leading_to_static_ptr = current_ptr;
//             info->number_to_dst_ptr += 1;
//         }
//     }
//     else
//     {
//         // This is not a static_type and not a dst_type.
//         for (Iter p = __base_info, e = __base_info + __base_count; p < e; ++p)
//         {
//             p->search_below_dst(info, current_ptr, public_path);
//             // break out early here if you can detect it doesn't matter if you do
//         }
//     }
// }
void
__vmi_class_type_info::search_below_dst(__dynamic_cast_info* info,
                                        const void* current_ptr,
                                        int path_below,
                                        bool use_strcmp) const
{
    typedef const __base_class_type_info* Iter;
    if (is_equal(this, info->static_type, use_strcmp))
        process_static_type_below_dst(info, current_ptr, path_below);
    else if (is_equal(this, info->dst_type, use_strcmp))
    {
        // We've been here before if we've recorded current_ptr in one of these
        //   two places:
        if (current_ptr == info->dst_ptr_leading_to_static_ptr ||
            current_ptr == info->dst_ptr_not_leading_to_static_ptr)
        {
            // We've seen this node before, and therefore have already searched
            // its base classes above.
            //  Update path to here that is "most public".
            if (path_below == public_path)
                info->path_dynamic_ptr_to_dst_ptr = public_path;
        }
        else  // We have haven't been here before
        {
            // Record the access path that got us here
            //   If there is more than one dst_type this path doesn't matter.
            info->path_dynamic_ptr_to_dst_ptr = path_below;
            bool does_dst_type_point_to_our_static_type = false;
            // Only search above here if dst_type derives from static_type, or
            //    if it is unknown if dst_type derives from static_type.
            if (info->is_dst_type_derived_from_static_type != no)
            {
                // Set up flags to record results from all base classes
                bool is_dst_type_derived_from_static_type = false;

                // We've found a dst_type with a potentially public path to here.
                // We have to assume the path is public because it may become
                //   public later (if we get back to here with a public path).
                // We can stop looking above if:
                //    1.  We've found a public path to (static_ptr, static_type).
                //    2.  We've found an ambiguous cast from (static_ptr, static_type) to a dst_type.
                //        This is detected at the (static_ptr, static_type).
                //    3.  We can prove that there is no public path to (static_ptr, static_type)
                //        above here.
                const Iter e = __base_info + __base_count;
                for (Iter p = __base_info; p < e; ++p)
                {
                    // Zero out found flags
                    info->found_our_static_ptr = false;
                    info->found_any_static_type = false;
                    p->search_above_dst(info, current_ptr, current_ptr, public_path, use_strcmp);
                    if (info->search_done)
                        break;
                    if (info->found_any_static_type)
                    {
                        is_dst_type_derived_from_static_type = true;
                        if (info->found_our_static_ptr)
                        {
                            does_dst_type_point_to_our_static_type = true;
                            // If we found what we're looking for, stop looking above.
                            if (info->path_dst_ptr_to_static_ptr == public_path)
                                break;
                            // We found a private path to (static_ptr, static_type)
                            //   If there is no diamond then there is only one path
                            //   to (static_ptr, static_type) and we just found it.
                            if (!(__flags & __diamond_shaped_mask))
                                break;
                        }
                        else
                        {
                            // If we found a static_type that isn't the one we're looking
                            //    for, and if there are no repeated types above here,
                            //    then stop looking.
                            if (!(__flags & __non_diamond_repeat_mask))
                                break;
                        }
                    }
                }
                // If we found no static_type,s then dst_type doesn't derive
                //   from static_type, else it does.  Record this result so that
                //   next time we hit a dst_type we will know not to search above
                //   it if it doesn't derive from static_type.
                if (is_dst_type_derived_from_static_type)
                    info->is_dst_type_derived_from_static_type = yes;
                else
                    info->is_dst_type_derived_from_static_type = no;
              }
              if (!does_dst_type_point_to_our_static_type)
              {
                  // We found a dst_type that doesn't point to (static_ptr, static_type)
                  // So record the address of this dst_ptr and increment the
                  // count of the number of such dst_types found in the tree.
                  info->dst_ptr_not_leading_to_static_ptr = current_ptr;
                  info->number_to_dst_ptr += 1;
                  // If there exists another dst with a private path to
                  //    (static_ptr, static_type), then the cast from
                  //     (dynamic_ptr, dynamic_type) to dst_type is now ambiguous,
                  //      so stop search.
                  if (info->number_to_static_ptr == 1 &&
                          info->path_dst_ptr_to_static_ptr == not_public_path)
                      info->search_done = true;
              }
        }
    }
    else
    {
        // This is not a static_type and not a dst_type.
        const Iter e = __base_info + __base_count;
        Iter p = __base_info;
        p->search_below_dst(info, current_ptr, path_below, use_strcmp);
        if (++p < e)
        {
            if ((__flags & __diamond_shaped_mask) || info->number_to_static_ptr == 1)
            {
                // If there are multiple paths to a base above from here, or if
                //    a dst_type pointing to (static_ptr, static_type) has been found,
                //    then there is no way to break out of this loop early unless
                //    something below detects the search is done.
                do
                {
                    if (info->search_done)
                        break;
                    p->search_below_dst(info, current_ptr, path_below, use_strcmp);
                } while (++p < e);
            }
            else if (__flags & __non_diamond_repeat_mask)
            {
                // There are not multiple paths to any base class from here and a
                //   dst_type pointing to (static_ptr, static_type) has not yet been
                //   found.
                do
                {
                    if (info->search_done)
                        break;
                    // If we just found a dst_type with a public path to (static_ptr, static_type),
                    //    then the only reason to continue the search is to make sure
                    //    no other dst_type points to (static_ptr, static_type).
                    //    If !diamond, then we don't need to search here.
                    if (info->number_to_static_ptr == 1 &&
                              info->path_dst_ptr_to_static_ptr == public_path)
                        break;
                    p->search_below_dst(info, current_ptr, path_below, use_strcmp);
                } while (++p < e);
            }
            else
            {
                // There are no repeated types above this node.
                // There are no nodes with multiple parents above this node.
                // no dst_type has been found to (static_ptr, static_type)
                do
                {
                    if (info->search_done)
                        break;
                    // If we just found a dst_type with a public path to (static_ptr, static_type),
                    //    then the only reason to continue the search is to make sure sure
                    //    no other dst_type points to (static_ptr, static_type).
                    //    If !diamond, then we don't need to search here.
                    // if we just found a dst_type with a private path to (static_ptr, static_type),
                    //    then we're only looking for a public path to (static_ptr, static_type)
                    //    and to check for other dst_types.
                    //    If !diamond & !repeat, then there is not a pointer to (static_ptr, static_type)
                    //    and not a dst_type under here.
                    if (info->number_to_static_ptr == 1)
                        break;
                    p->search_below_dst(info, current_ptr, path_below, use_strcmp);
                } while (++p < e);
            }
        }
    }
}

// This is the same algorithm as __vmi_class_type_info::search_below_dst but
//   simplified to the case that there is only a single base class.
void
__si_class_type_info::search_below_dst(__dynamic_cast_info* info,
                                       const void* current_ptr,
                                       int path_below,
                                       bool use_strcmp) const
{
    if (is_equal(this, info->static_type, use_strcmp))
        process_static_type_below_dst(info, current_ptr, path_below);
    else if (is_equal(this, info->dst_type, use_strcmp))
    {
        // We've been here before if we've recorded current_ptr in one of these
        //   two places:
        if (current_ptr == info->dst_ptr_leading_to_static_ptr ||
            current_ptr == info->dst_ptr_not_leading_to_static_ptr)
        {
            // We've seen this node before, and therefore have already searched
            // its base classes above.
            //  Update path to here that is "most public".
            if (path_below == public_path)
                info->path_dynamic_ptr_to_dst_ptr = public_path;
        }
        else  // We have haven't been here before
        {
            // Record the access path that got us here
            //   If there is more than one dst_type this path doesn't matter.
            info->path_dynamic_ptr_to_dst_ptr = path_below;
            bool does_dst_type_point_to_our_static_type = false;
            // Only search above here if dst_type derives from static_type, or
            //    if it is unknown if dst_type derives from static_type.
            if (info->is_dst_type_derived_from_static_type != no)
            {
                // Set up flags to record results from all base classes
                bool is_dst_type_derived_from_static_type = false;
                // Zero out found flags
                info->found_our_static_ptr = false;
                info->found_any_static_type = false;
                __base_type->search_above_dst(info, current_ptr, current_ptr, public_path, use_strcmp);
                if (info->found_any_static_type)
                {
                    is_dst_type_derived_from_static_type = true;
                    if (info->found_our_static_ptr)
                        does_dst_type_point_to_our_static_type = true;
                }
                // If we found no static_type,s then dst_type doesn't derive
                //   from static_type, else it does.  Record this result so that
                //   next time we hit a dst_type we will know not to search above
                //   it if it doesn't derive from static_type.
                if (is_dst_type_derived_from_static_type)
                    info->is_dst_type_derived_from_static_type = yes;
                else
                    info->is_dst_type_derived_from_static_type = no;
            }
            if (!does_dst_type_point_to_our_static_type)
            {
                // We found a dst_type that doesn't point to (static_ptr, static_type)
                // So record the address of this dst_ptr and increment the
                // count of the number of such dst_types found in the tree.
                info->dst_ptr_not_leading_to_static_ptr = current_ptr;
                info->number_to_dst_ptr += 1;
                // If there exists another dst with a private path to
                //    (static_ptr, static_type), then the cast from
                //     (dynamic_ptr, dynamic_type) to dst_type is now ambiguous.
                if (info->number_to_static_ptr == 1 &&
                        info->path_dst_ptr_to_static_ptr == not_public_path)
                    info->search_done = true;
            }
        }
    }
    else
    {
        // This is not a static_type and not a dst_type
        __base_type->search_below_dst(info, current_ptr, path_below, use_strcmp);
    }
}

// This is the same algorithm as __vmi_class_type_info::search_below_dst but
//   simplified to the case that there is no base class.
void
__class_type_info::search_below_dst(__dynamic_cast_info* info,
                                    const void* current_ptr,
                                    int path_below,
                                    bool use_strcmp) const
{
    if (is_equal(this, info->static_type, use_strcmp))
        process_static_type_below_dst(info, current_ptr, path_below);
    else if (is_equal(this, info->dst_type, use_strcmp))
    {
        // We've been here before if we've recorded current_ptr in one of these
        //   two places:
        if (current_ptr == info->dst_ptr_leading_to_static_ptr ||
            current_ptr == info->dst_ptr_not_leading_to_static_ptr)
        {
            // We've seen this node before, and therefore have already searched
            // its base classes above.
            //  Update path to here that is "most public".
            if (path_below == public_path)
                info->path_dynamic_ptr_to_dst_ptr = public_path;
        }
        else  // We have haven't been here before
        {
            // Record the access path that got us here
            //   If there is more than one dst_type this path doesn't matter.
            info->path_dynamic_ptr_to_dst_ptr = path_below;
            // We found a dst_type that doesn't point to (static_ptr, static_type)
            // So record the address of this dst_ptr and increment the
            // count of the number of such dst_types found in the tree.
            info->dst_ptr_not_leading_to_static_ptr = current_ptr;
            info->number_to_dst_ptr += 1;
            // If there exists another dst with a private path to
            //    (static_ptr, static_type), then the cast from 
            //     (dynamic_ptr, dynamic_type) to dst_type is now ambiguous.
            if (info->number_to_static_ptr == 1 &&
                    info->path_dst_ptr_to_static_ptr == not_public_path)
                info->search_done = true;
            // We found that dst_type does not derive from static_type
            info->is_dst_type_derived_from_static_type = no;
        }
    }
}

// Call this function when searching above a dst_type node.  This function searches
// for a public path to (static_ptr, static_type).
// This function is guaranteed not to find a node of type dst_type.
// Theoretically this is a very simple function which just stops if it finds a
// static_type node:  All the hoopla surrounding the search code is doing
// nothing but looking for excuses to stop the search prematurely (break out of
// the for-loop).  That is, the algorithm below is simply an optimization of this:
// void
// __vmi_class_type_info::search_above_dst(__dynamic_cast_info* info,
//                                         const void* dst_ptr,
//                                         const void* current_ptr,
//                                         int path_below) const
// {
//     if (this == info->static_type)
//         process_static_type_above_dst(info, dst_ptr, current_ptr, path_below);
//     else
//     {
//         typedef const __base_class_type_info* Iter;
//         // This is not a static_type and not a dst_type
//         for (Iter p = __base_info, e = __base_info + __base_count; p < e; ++p)
//         {
//             p->search_above_dst(info, dst_ptr, current_ptr, public_path);
//             // break out early here if you can detect it doesn't matter if you do
//         }
//     }
// }
void
__vmi_class_type_info::search_above_dst(__dynamic_cast_info* info,
                                        const void* dst_ptr,
                                        const void* current_ptr,
                                        int path_below,
                                        bool use_strcmp) const
{
    if (is_equal(this, info->static_type, use_strcmp))
        process_static_type_above_dst(info, dst_ptr, current_ptr, path_below);
    else
    {
        typedef const __base_class_type_info* Iter;
        // This is not a static_type and not a dst_type
        // Save flags so they can be restored when returning to nodes below.
        bool found_our_static_ptr = info->found_our_static_ptr;
        bool found_any_static_type = info->found_any_static_type;
        // We've found a dst_type below with a path to here.  If the path
        //    to here is not public, there may be another path to here that
        //    is public.  So we have to assume that the path to here is public.
        //  We can stop looking above if:
        //    1.  We've found a public path to (static_ptr, static_type).
        //    2.  We've found an ambiguous cast from (static_ptr, static_type) to a dst_type.
        //        This is detected at the (static_ptr, static_type).
        //    3.  We can prove that there is no public path to (static_ptr, static_type)
        //        above here.
        const Iter e = __base_info + __base_count;
        Iter p = __base_info;
        // Zero out found flags
        info->found_our_static_ptr = false;
        info->found_any_static_type = false;
        p->search_above_dst(info, dst_ptr, current_ptr, path_below, use_strcmp);
        found_our_static_ptr |= info->found_our_static_ptr;
        found_any_static_type |= info->found_any_static_type;
        if (++p < e)
        {
            do
            {
                if (info->search_done)
                    break;
                if (info->found_our_static_ptr)
                {
                    // If we found what we're looking for, stop looking above.
                    if (info->path_dst_ptr_to_static_ptr == public_path)
                        break;
                    // We found a private path to (static_ptr, static_type)
                    //   If there is no diamond then there is only one path
                    //   to (static_ptr, static_type) from here and we just found it.
                    if (!(__flags & __diamond_shaped_mask))
                        break;
                }
                else if (info->found_any_static_type)
                {
                    // If we found a static_type that isn't the one we're looking
                    //    for, and if there are no repeated types above here,
                    //    then stop looking.
                    if (!(__flags & __non_diamond_repeat_mask))
                        break;
                }
                // Zero out found flags
                info->found_our_static_ptr = false;
                info->found_any_static_type = false;
                p->search_above_dst(info, dst_ptr, current_ptr, path_below, use_strcmp);
                found_our_static_ptr |= info->found_our_static_ptr;
                found_any_static_type |= info->found_any_static_type;
            } while (++p < e);
        }
        // Restore flags
        info->found_our_static_ptr = found_our_static_ptr;
        info->found_any_static_type = found_any_static_type;
    }
}

// This is the same algorithm as __vmi_class_type_info::search_above_dst but
//   simplified to the case that there is only a single base class.
void
__si_class_type_info::search_above_dst(__dynamic_cast_info* info,
                                       const void* dst_ptr,
                                       const void* current_ptr,
                                       int path_below,
                                       bool use_strcmp) const
{
    if (is_equal(this, info->static_type, use_strcmp))
        process_static_type_above_dst(info, dst_ptr, current_ptr, path_below);
    else
        __base_type->search_above_dst(info, dst_ptr, current_ptr, path_below, use_strcmp);
}

// This is the same algorithm as __vmi_class_type_info::search_above_dst but
//   simplified to the case that there is no base class.
void
__class_type_info::search_above_dst(__dynamic_cast_info* info,
                                    const void* dst_ptr,
                                    const void* current_ptr,
                                    int path_below,
                                    bool use_strcmp) const
{
    if (is_equal(this, info->static_type, use_strcmp))
        process_static_type_above_dst(info, dst_ptr, current_ptr, path_below);
}

// The search functions for __base_class_type_info are simply convenience
//   functions for adjusting the current_ptr and path_below as the search is
//   passed up to the base class node.

void
__base_class_type_info::search_above_dst(__dynamic_cast_info* info,
                                         const void* dst_ptr,
                                         const void* current_ptr,
                                         int path_below,
                                         bool use_strcmp) const
{
    ptrdiff_t offset_to_base = __offset_flags >> __offset_shift;
    if (__offset_flags & __virtual_mask)
    {
        const char* vtable = *static_cast<const char*const*>(current_ptr);
        offset_to_base = *reinterpret_cast<const ptrdiff_t*>(vtable + offset_to_base);
    }
    __base_type->search_above_dst(info, dst_ptr,
                                  static_cast<const char*>(current_ptr) + offset_to_base,
                                  (__offset_flags & __public_mask) ?
                                      path_below :
                                      not_public_path,
                                  use_strcmp);
}

void
__base_class_type_info::search_below_dst(__dynamic_cast_info* info,
                                         const void* current_ptr,
                                         int path_below,
                                         bool use_strcmp) const
{
    ptrdiff_t offset_to_base = __offset_flags >> __offset_shift;
    if (__offset_flags & __virtual_mask)
    {
        const char* vtable = *static_cast<const char*const*>(current_ptr);
        offset_to_base = *reinterpret_cast<const ptrdiff_t*>(vtable + offset_to_base);
    }
    __base_type->search_below_dst(info,
                                  static_cast<const char*>(current_ptr) + offset_to_base,
                                  (__offset_flags & __public_mask) ?
                                      path_below :
                                      not_public_path,
                                  use_strcmp);
}

}  // __cxxabiv1
