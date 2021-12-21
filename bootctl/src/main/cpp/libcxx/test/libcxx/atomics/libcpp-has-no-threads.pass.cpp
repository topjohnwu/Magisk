//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// XFAIL: libcpp-has-no-threads

#ifdef _LIBCPP_HAS_NO_THREADS
#error This should be XFAIL'd for the purpose of detecting that the LIT feature\
   'libcpp-has-no-threads' is available iff _LIBCPP_HAS_NO_THREADS is defined
#endif

int main()
{
}
