========================
Symbol Visibility Macros
========================

.. contents::
   :local:

Overview
========

Libc++ uses various "visibility" macros in order to provide a stable ABI in
both the library and the headers. These macros work by changing the
visibility and inlining characteristics of the symbols they are applied to.

Visibility Macros
=================

**_LIBCPP_HIDDEN**
  Mark a symbol as hidden so it will not be exported from shared libraries.

**_LIBCPP_FUNC_VIS**
  Mark a symbol as being exported by the libc++ library. This attribute must
  be applied to the declaration of all functions exported by the libc++ dylib.

**_LIBCPP_EXPORTED_FROM_ABI**
  Mark a symbol as being exported by the libc++ library. This attribute may
  only be applied to objects defined in the libc++ runtime library. On Windows,
  this macro applies `dllimport`/`dllexport` to the symbol, and on other
  platforms it gives the symbol default visibility.

**_LIBCPP_OVERRIDABLE_FUNC_VIS**
  Mark a symbol as being exported by the libc++ library, but allow it to be
  overridden locally. On non-Windows, this is equivalent to `_LIBCPP_FUNC_VIS`.
  This macro is applied to all `operator new` and `operator delete` overloads.

  **Windows Behavior**: Any symbol marked `dllimport` cannot be overridden
  locally, since `dllimport` indicates the symbol should be bound to a separate
  DLL. All `operator new` and `operator delete` overloads are required to be
  locally overridable, and therefore must not be marked `dllimport`. On Windows,
  this macro therefore expands to `__declspec(dllexport)` when building the
  library and has an empty definition otherwise.

**_LIBCPP_HIDE_FROM_ABI**
  Mark a function as not being part of the ABI of any final linked image that
  uses it.

**_LIBCPP_HIDE_FROM_ABI_AFTER_V1**
  Mark a function as being hidden from the ABI (per `_LIBCPP_HIDE_FROM_ABI`)
  when libc++ is built with an ABI version after ABI v1. This macro is used to
  maintain ABI compatibility for symbols that have been historically exported
  by libc++ in v1 of the ABI, but that we don't want to export in the future.

  This macro works as follows. When we build libc++, we either hide the symbol
  from the ABI (if the symbol is not part of the ABI in the version we're
  building), or we leave it included. From user code (i.e. when we're not
  building libc++), the macro always marks symbols as internal so that programs
  built using new libc++ headers stop relying on symbols that are removed from
  the ABI in a future version. Each time we release a new stable version of the
  ABI, we should create a new _LIBCPP_HIDE_FROM_ABI_AFTER_XXX macro, and we can
  use it to start removing symbols from the ABI after that stable version.

**_LIBCPP_HIDE_FROM_ABI_PER_TU**
  This macro controls whether symbols hidden from the ABI with `_LIBCPP_HIDE_FROM_ABI`
  are local to each translation unit in addition to being local to each final
  linked image. This macro is defined to either 0 or 1. When it is defined to
  1, translation units compiled with different versions of libc++ can be linked
  together, since all non ABI-facing functions are local to each translation unit.
  This allows static archives built with different versions of libc++ to be linked
  together. This also means that functions marked with `_LIBCPP_HIDE_FROM_ABI`
  are not guaranteed to have the same address across translation unit boundaries.

  When the macro is defined to 0, there is no guarantee that translation units
  compiled with different versions of libc++ can interoperate. However, this
  leads to code size improvements, since non ABI-facing functions can be
  deduplicated across translation unit boundaries.

  This macro can be defined by users to control the behavior they want from
  libc++. The default value of this macro (0 or 1) is controlled by whether
  `_LIBCPP_HIDE_FROM_ABI_PER_TU_BY_DEFAULT` is defined, which is intended to
  be used by vendors only (see below).

**_LIBCPP_HIDE_FROM_ABI_PER_TU_BY_DEFAULT**
  This macro controls the default value for `_LIBCPP_HIDE_FROM_ABI_PER_TU`.
  When the macro is defined, per TU ABI insulation is enabled by default, and
  `_LIBCPP_HIDE_FROM_ABI_PER_TU` is defined to 1 unless overridden by users.
  Otherwise, per TU ABI insulation is disabled by default, and
  `_LIBCPP_HIDE_FROM_ABI_PER_TU` is defined to 0 unless overridden by users.

  This macro is intended for vendors to control whether they want to ship
  libc++ with per TU ABI insulation enabled by default. Users can always
  control the behavior they want by defining `_LIBCPP_HIDE_FROM_ABI_PER_TU`
  appropriately.

  By default, this macro is not defined, which means that per TU ABI insulation
  is not provided unless explicitly overridden by users.

**_LIBCPP_TYPE_VIS**
  Mark a type's typeinfo, vtable and members as having default visibility.
  This attribute cannot be used on class templates.

**_LIBCPP_TEMPLATE_VIS**
  Mark a type's typeinfo and vtable as having default visibility.
  This macro has no effect on the visibility of the type's member functions.

  **GCC Behavior**: GCC does not support Clang's `type_visibility(...)`
  attribute. With GCC the `visibility(...)` attribute is used and member
  functions are affected.

  **Windows Behavior**: DLLs do not support dllimport/export on class templates.
  The macro has an empty definition on this platform.


**_LIBCPP_ENUM_VIS**
  Mark the typeinfo of an enum as having default visibility. This attribute
  should be applied to all enum declarations.

  **Windows Behavior**: DLLs do not support importing or exporting enumeration
  typeinfo. The macro has an empty definition on this platform.

  **GCC Behavior**: GCC un-hides the typeinfo for enumerations by default, even
  if `-fvisibility=hidden` is specified. Additionally applying a visibility
  attribute to an enum class results in a warning. The macro has an empty
  definition with GCC.

**_LIBCPP_EXTERN_TEMPLATE_TYPE_VIS**
  Mark the member functions, typeinfo, and vtable of the type named in
  a `_LIBCPP_EXTERN_TEMPLATE` declaration as being exported by the libc++ library.
  This attribute must be specified on all extern class template declarations.

  This macro is used to override the `_LIBCPP_TEMPLATE_VIS` attribute
  specified on the primary template and to export the member functions produced
  by the explicit instantiation in the dylib.

  **GCC Behavior**: GCC ignores visibility attributes applied the type in
  extern template declarations and applying an attribute results in a warning.
  However since `_LIBCPP_TEMPLATE_VIS` is the same as
  `__attribute__((visibility("default"))` the visibility is already correct.
  The macro has an empty definition with GCC.

  **Windows Behavior**: `extern template` and `dllexport` are fundamentally
  incompatible *on a class template* on Windows; the former suppresses
  instantiation, while the latter forces it. Specifying both on the same
  declaration makes the class template be instantiated, which is not desirable
  inside headers. This macro therefore expands to `dllimport` outside of libc++
  but nothing inside of it (rather than expanding to `dllexport`); instead, the
  explicit instantiations themselves are marked as exported. Note that this
  applies *only* to extern *class* templates. Extern *function* templates obey
  regular import/export semantics, and applying `dllexport` directly to the
  extern template declaration (i.e. using `_LIBCPP_FUNC_VIS`) is the correct
  thing to do for them.

**_LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS**
  Mark the member functions, typeinfo, and vtable of an explicit instantiation
  of a class template as being exported by the libc++ library. This attribute
  must be specified on all class template explicit instantiations.

  It is only necessary to mark the explicit instantiation itself (as opposed to
  the extern template declaration) as exported on Windows, as discussed above.
  On all other platforms, this macro has an empty definition.

**_LIBCPP_METHOD_TEMPLATE_IMPLICIT_INSTANTIATION_VIS**
  Mark a symbol as hidden so it will not be exported from shared libraries. This
  is intended specifically for method templates of either classes marked with
  `_LIBCPP_TYPE_VIS` or classes with an extern template instantiation
  declaration marked with `_LIBCPP_EXTERN_TEMPLATE_TYPE_VIS`.

  When building libc++ with hidden visibility, we want explicit template
  instantiations to export members, which is consistent with existing Windows
  behavior. We also want classes annotated with `_LIBCPP_TYPE_VIS` to export
  their members, which is again consistent with existing Windows behavior.
  Both these changes are necessary for clients to be able to link against a
  libc++ DSO built with hidden visibility without encountering missing symbols.

  An unfortunate side effect, however, is that method templates of classes
  either marked `_LIBCPP_TYPE_VIS` or with extern template instantiation
  declarations marked with `_LIBCPP_EXTERN_TEMPLATE_TYPE_VIS` also get default
  visibility when instantiated. These methods are often implicitly instantiated
  inside other libraries which use the libc++ headers, and will therefore end up
  being exported from those libraries, since those implicit instantiations will
  receive default visibility. This is not acceptable for libraries that wish to
  control their visibility, and led to PR30642.

  Consequently, all such problematic method templates are explicitly marked
  either hidden (via this macro) or inline, so that they don't leak into client
  libraries. The problematic methods were found by running
  `bad-visibility-finder <https://github.com/smeenai/bad-visibility-finder>`_
  against the libc++ headers after making `_LIBCPP_TYPE_VIS` and
  `_LIBCPP_EXTERN_TEMPLATE_TYPE_VIS` expand to default visibility.

**_LIBCPP_EXCEPTION_ABI**
  Mark the member functions, typeinfo, and vtable of the type as being exported
  by the libc++ library. This macro must be applied to all *exception types*.
  Exception types should be defined directly in namespace `std` and not the
  versioning namespace. This allows throwing and catching some exception types
  between libc++ and libstdc++.

**_LIBCPP_INTERNAL_LINKAGE**
  Mark the affected entity as having internal linkage (i.e. the `static`
  keyword in C). This is only a best effort: when the `internal_linkage`
  attribute is not available, we fall back to forcing the function to be
  inlined, which approximates internal linkage since an externally visible
  symbol is never generated for that function. This is an internal macro
  used as an implementation detail by other visibility macros. Never mark
  a function or a class with this macro directly.

**_LIBCPP_ALWAYS_INLINE**
  Forces inlining of the function it is applied to. For visibility purposes,
  this macro is used to make sure that an externally visible symbol is never
  generated in an object file when the `internal_linkage` attribute is not
  available. This is an internal macro used by other visibility macros, and
  it should not be used directly.

Links
=====

* `[cfe-dev] Visibility in libc++ - 1 <http://lists.llvm.org/pipermail/cfe-dev/2013-July/030610.html>`_
* `[cfe-dev] Visibility in libc++ - 2 <http://lists.llvm.org/pipermail/cfe-dev/2013-August/031195.html>`_
* `[libcxx] Visibility fixes for Windows <http://lists.llvm.org/pipermail/cfe-commits/Week-of-Mon-20130805/085461.html>`_
