
====================
Libc++ ABI stability
====================

Libc++ aims to preserve stable ABI to avoid subtle bugs when code built to the old ABI
is linked with the code build to the new ABI. At the same time, libc++ allows ABI-breaking
improvements and bugfixes for the scenarios when ABI change is not a issue.

To support both cases, libc++ allows specifying the ABI version at the
build time.  The version is defined with a cmake option
LIBCXX_ABI_VERSION. Another option LIBCXX_ABI_UNSTABLE can be used to
include all present ABI breaking features. These options translate
into C++ macro definitions _LIBCPP_ABI_VERSION, _LIBCPP_ABI_UNSTABLE.

Any ABI-changing feature is placed under it's own macro, _LIBCPP_ABI_XXX, which is enabled
based on the value of _LIBCPP_ABI_VERSION. _LIBCPP_ABI_UNSTABLE, if set, enables all features at once.
