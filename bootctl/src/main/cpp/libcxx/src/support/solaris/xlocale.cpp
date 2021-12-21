//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifdef __sun__

#include "support/solaris/xlocale.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/localedef.h>

extern "C" {

int isxdigit_l(int __c, locale_t __l) {
    return isxdigit(__c);
}

int iswxdigit_l(wint_t __c, locale_t __l) {
    return isxdigit(__c);
}

// FIXME: This disregards the locale, which is Very Wrong
#define vsnprintf_l(__s, __n, __l, __format, __va)  \
    vsnprintf(__s, __n, __format, __va) 

int snprintf_l(char *__s, size_t __n, locale_t __l, const char *__format, ...)
{
  va_list __va;
  va_start(__va, __format);
  int __res = vsnprintf_l(__s, __n , __l, __format, __va);
  va_end(__va);
  return __res;
}

int asprintf_l(char **__s, locale_t __l, const char *__format, ...) {
  va_list __va;
  va_start(__va, __format);
  // FIXME:
  int __res = vasprintf(__s, __format, __va);
  va_end(__va);
  return __res;
}

int sscanf_l(const char *__s, locale_t __l, const char *__format, ...) {
  va_list __va;
  va_start(__va, __format);
  // FIXME:
  int __res = vsscanf(__s, __format, __va);
  va_end(__va);
  return __res;
}

size_t mbrtowc_l(wchar_t *__pwc, const char *__pmb,
                 size_t __max, mbstate_t *__ps, locale_t __loc) {
  return mbrtowc(__pwc, __pmb, __max, __ps);
}

struct lconv *localeconv_l(locale_t __l) {
  return localeconv();
}

};

#endif // __sun__
