//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <cstdio>

#include <cstdio>
#include <type_traits>

#include "test_macros.h"

#ifndef BUFSIZ
#error BUFSIZ not defined
#endif

#ifndef EOF
#error EOF not defined
#endif

#ifndef FILENAME_MAX
#error FILENAME_MAX not defined
#endif

#ifndef FOPEN_MAX
#error FOPEN_MAX not defined
#endif

#ifndef L_tmpnam
#error L_tmpnam not defined
#endif

#ifndef NULL
#error NULL not defined
#endif

#ifndef SEEK_CUR
#error SEEK_CUR not defined
#endif

#ifndef SEEK_END
#error SEEK_END not defined
#endif

#ifndef SEEK_SET
#error SEEK_SET not defined
#endif

#ifndef TMP_MAX
#error TMP_MAX not defined
#endif

#ifndef _IOFBF
#error _IOFBF not defined
#endif

#ifndef _IOLBF
#error _IOLBF not defined
#endif

#ifndef _IONBF
#error _IONBF not defined
#endif

#ifndef stderr
#error stderr not defined
#endif

#ifndef stdin
#error stdin not defined
#endif

#ifndef stdout
#error stdout not defined
#endif

#include <cstdarg>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-zero-length"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

int main()
{
    std::FILE* fp = 0;
    std::fpos_t fpos = std::fpos_t();
    std::size_t s = 0;
    char* cp = 0;
    std::va_list va;
    ((void)fp); // Prevent unused warning
    ((void)fpos); // Prevent unused warning
    ((void)s); // Prevent unused warning
    ((void)cp); // Prevent unused warning
    ((void)va); // Prevent unused warning
    static_assert((std::is_same<decltype(std::fclose(fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::fflush(fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::setbuf(fp,cp)), void>::value), "");
    static_assert((std::is_same<decltype(std::vfprintf(fp,"",va)), int>::value), "");
    static_assert((std::is_same<decltype(std::fprintf(fp," ")), int>::value), "");
    static_assert((std::is_same<decltype(std::fscanf(fp,"")), int>::value), "");
    static_assert((std::is_same<decltype(std::snprintf(cp,0," ")), int>::value), "");
    static_assert((std::is_same<decltype(std::sprintf(cp," ")), int>::value), "");
    static_assert((std::is_same<decltype(std::sscanf("","")), int>::value), "");
    static_assert((std::is_same<decltype(std::vfprintf(fp,"",va)), int>::value), "");
    static_assert((std::is_same<decltype(std::vfscanf(fp,"",va)), int>::value), "");
    static_assert((std::is_same<decltype(std::vsnprintf(cp,0," ",va)), int>::value), "");
    static_assert((std::is_same<decltype(std::vsprintf(cp," ",va)), int>::value), "");
    static_assert((std::is_same<decltype(std::vsscanf("","",va)), int>::value), "");
    static_assert((std::is_same<decltype(std::fgetc(fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::fgets(cp,0,fp)), char*>::value), "");
    static_assert((std::is_same<decltype(std::fputc(0,fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::fputs("",fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::getc(fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::putc(0,fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::ungetc(0,fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::fread((void*)0,0,0,fp)), std::size_t>::value), "");
    static_assert((std::is_same<decltype(std::fwrite((const void*)0,0,0,fp)), std::size_t>::value), "");
    static_assert((std::is_same<decltype(std::fgetpos(fp, &fpos)), int>::value), "");
    static_assert((std::is_same<decltype(std::fseek(fp, 0,0)), int>::value), "");
    static_assert((std::is_same<decltype(std::fsetpos(fp, &fpos)), int>::value), "");
    static_assert((std::is_same<decltype(std::ftell(fp)), long>::value), "");
    static_assert((std::is_same<decltype(std::rewind(fp)), void>::value), "");
    static_assert((std::is_same<decltype(std::clearerr(fp)), void>::value), "");
    static_assert((std::is_same<decltype(std::feof(fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::ferror(fp)), int>::value), "");
    static_assert((std::is_same<decltype(std::perror("")), void>::value), "");

#ifndef _LIBCPP_HAS_NO_GLOBAL_FILESYSTEM_NAMESPACE
    static_assert((std::is_same<decltype(std::fopen("", "")), std::FILE*>::value), "");
    static_assert((std::is_same<decltype(std::freopen("", "", fp)), std::FILE*>::value), "");
    static_assert((std::is_same<decltype(std::remove("")), int>::value), "");
    static_assert((std::is_same<decltype(std::rename("","")), int>::value), "");
    static_assert((std::is_same<decltype(std::tmpfile()), std::FILE*>::value), "");
    static_assert((std::is_same<decltype(std::tmpnam(cp)), char*>::value), "");
#endif

#ifndef _LIBCPP_HAS_NO_STDIN
    static_assert((std::is_same<decltype(std::getchar()), int>::value), "");
#if TEST_STD_VER <= 11
    static_assert((std::is_same<decltype(std::gets(cp)), char*>::value), "");
#endif
    static_assert((std::is_same<decltype(std::scanf(" ")), int>::value), "");
    static_assert((std::is_same<decltype(std::vscanf("",va)), int>::value), "");
#endif

#ifndef _LIBCPP_HAS_NO_STDOUT
    static_assert((std::is_same<decltype(std::printf(" ")), int>::value), "");
    static_assert((std::is_same<decltype(std::putchar(0)), int>::value), "");
    static_assert((std::is_same<decltype(std::puts("")), int>::value), "");
    static_assert((std::is_same<decltype(std::vprintf(" ",va)), int>::value), "");
#endif
}
