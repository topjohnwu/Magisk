/*
File: tinyprintf.c

Copyright (C) 2004  Kustaa Nyholm

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <stdbool.h>

#include "tinystdio.h"


/*
 * Configuration
 */

/* Enable long int support */
#define PRINTF_LONG_SUPPORT

/* Enable long long int support (implies long int support) */
#define PRINTF_LONG_LONG_SUPPORT

/* Enable %z (size_t) support */
#define PRINTF_SIZE_T_SUPPORT


/*
 * Configuration adjustments
 */
#ifdef PRINTF_SIZE_T_SUPPORT
#include <sys/types.h>
#endif

#ifdef PRINTF_LONG_LONG_SUPPORT
# define PRINTF_LONG_SUPPORT
#endif

/* __SIZEOF_<type>__ defined at least by gcc */
#ifdef __SIZEOF_POINTER__
# define SIZEOF_POINTER __SIZEOF_POINTER__
#endif
#ifdef __SIZEOF_LONG_LONG__
# define SIZEOF_LONG_LONG __SIZEOF_LONG_LONG__
#endif
#ifdef __SIZEOF_LONG__
# define SIZEOF_LONG __SIZEOF_LONG__
#endif
#ifdef __SIZEOF_INT__
# define SIZEOF_INT __SIZEOF_INT__
#endif

#ifdef __GNUC__
# define _TFP_GCC_NO_INLINE_  __attribute__ ((noinline))
#else
# define _TFP_GCC_NO_INLINE_
#endif

/*
 * Implementation
 */
struct param {
    bool lz;            /**<  Leading zeros */
    bool alt;           /**<  alternate form */
    bool uc;            /**<  Upper case (for base16 only) */
    bool align_left;    /**<  0 == align right (default), 1 == align left */
    int width;          /**<  field width */
    char sign;          /**<  The sign to display (if any) */
    unsigned int base;  /**<  number base (e.g.: 8, 10, 16) */
    char *bf;           /**<  Buffer to output */
    char prec;          /**<  Floating point precision */
};


#ifdef PRINTF_LONG_LONG_SUPPORT
static void _TFP_GCC_NO_INLINE_ ulli2a(
    unsigned long long int num, struct param *p)
{
    int n = 0;
    unsigned long long int d = 1;
    char *bf = p->bf;
    while (num / d >= p->base)
        d *= p->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= p->base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : (p->uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
}

static void lli2a(long long int num, struct param *p)
{
    if (num < 0) {
        num = -num;
        p->sign = '-';
    }
    ulli2a(num, p);
}
#endif

#ifdef PRINTF_LONG_SUPPORT
static void uli2a(unsigned long int num, struct param *p)
{
    int n = 0;
    unsigned long int d = 1;
    char *bf = p->bf;
    while (num / d >= p->base)
        d *= p->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= p->base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : (p->uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
}

static void li2a(long num, struct param *p)
{
    if (num < 0) {
        num = -num;
        p->sign = '-';
    }
    uli2a(num, p);
}
#endif

static void ui2a(unsigned int num, struct param *p)
{
    int n = 0;
    unsigned int d = 1;
    char *bf = p->bf;
    while (num / d >= p->base)
        d *= p->base;
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= p->base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : (p->uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
}

static void i2a(int num, struct param *p)
{
    if (num < 0) {
        num = -num;
        p->sign = '-';
    }
    ui2a(num, p);
}

static int a2d(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else
        return -1;
}

static char a2u(char ch, const char **src, int base, int *nump)
{
    const char *p = *src;
    int num = 0;
    int digit;
    while ((digit = a2d(ch)) >= 0) {
        if (digit > base)
            break;
        num = num * base + digit;
        ch = *p++;
    }
    *src = p;
    *nump = num;
    return ch;
}

static void putchw(void *putp, putcf putf, struct param *p)
{
    char ch;
    int n = p->width;
    char *bf = p->bf;

    /* Number of filling characters */
    while (*bf++ && n > 0)
        n--;
    if (p->sign)
        n--;
    if (p->alt && p->base == 16)
        n -= 2;
    else if (p->alt && p->base == 8)
        n--;

    /* Fill with space to align to the right, before alternate or sign */
    if (!p->lz && !p->align_left) {
        while (n-- > 0)
            putf(putp, ' ');
    }

    /* print sign */
    if (p->sign)
        putf(putp, p->sign);

    /* Alternate */
    if (p->alt && p->base == 16) {
        putf(putp, '0');
        putf(putp, (p->uc ? 'X' : 'x'));
    } else if (p->alt && p->base == 8) {
        putf(putp, '0');
    }

    /* Fill with zeros, after alternate or sign */
    if (p->lz) {
        while (n-- > 0)
            putf(putp, '0');
    }

    /* Put actual buffer */
    bf = p->bf;
    while ((ch = *bf++))
        putf(putp, ch);

    /* Fill with space to align to the left, after string */
    if (!p->lz && p->align_left) {
        while (n-- > 0)
            putf(putp, ' ');
    }
}

void tfp_format(void *putp, putcf putf, const char *fmt, va_list va)
{
    struct param p;
    double fval;
    int    temp_buffer[16];
    int    fpart;
    int    fiter;
    int    ffactor;
    int    sign;
#ifdef PRINTF_LONG_SUPPORT
    char bf[23];  /* long = 64b on some architectures */
#else
    char bf[12];  /* int = 32b on some architectures */
#endif
    char ch;
    p.bf = bf;

    while ((ch = *(fmt++))) {
        if (ch != '%') {
            putf(putp, ch);
        } else {
#ifdef PRINTF_LONG_SUPPORT
    char lng = 0;  /* 1 for long, 2 for long long */
#endif
    /* Init parameter struct */
    p.lz         = 0;
    p.alt        = 0;
    p.width      = 0;
    p.align_left = 0;
    p.sign       = 0;
    p.prec       = TINY_PRINTF_FP_PRECISION;

    /* Flags */
    while ((ch = *(fmt++))) {
        switch (ch) {
        case '-':
            p.align_left = 1;
            continue;
        case '0':
            p.lz = 1;
            continue;
        case '#':
            p.alt = 1;
            continue;
        case '+':
            p.sign = 1;
            continue;
        default:
            break;
        }
        break;
    }

    /* Width */
    if (ch >= '0' && ch <= '9') {
        ch = a2u(ch, &fmt, 10, &(p.width));
    }

    /* We accept 'x.y' format but don't support it completely:
     * we ignore the 'y' digit => this ignores 0-fill
     * size and makes it == width (ie. 'x') */
    if (ch == '.') {
      //p.lz = 1;  /* zero-padding */
      /* ignore actual 0-fill size: */
       ch = *(fmt++);
       if (ch >= '0' && ch <= '9')
           p.prec = ch - '0';
       do
       {
           ch = *(fmt++);
       }   while (ch >= '0' && ch <= '9');

    }

#ifdef PRINTF_SIZE_T_SUPPORT
# ifdef PRINTF_LONG_SUPPORT
    if (ch == 'z') {
        ch = *(fmt++);
        if (sizeof(size_t) == sizeof(unsigned long int))
            lng = 1;
#  ifdef PRINTF_LONG_LONG_SUPPORT
        else if (sizeof(size_t) == sizeof(unsigned long long int))
            lng = 2;
#  endif
        } else
# endif
#endif

#ifdef PRINTF_LONG_SUPPORT
    if (ch == 'l') {
        ch = *(fmt++);
        lng = 1;
#ifdef PRINTF_LONG_LONG_SUPPORT
        if (ch == 'l') {
          ch = *(fmt++);
          lng = 2;
        }
#endif
    }
#endif
    switch (ch) {
    case 0:
        goto abort;
    case 'u':
        p.base = 10;
#ifdef PRINTF_LONG_SUPPORT
#ifdef PRINTF_LONG_LONG_SUPPORT
    if (2 == lng)
        ulli2a(va_arg(va, unsigned long long int), &p);
    else
#endif
    if (1 == lng)
        uli2a(va_arg(va, unsigned long int), &p);
    else
#endif
        ui2a(va_arg(va, unsigned int), &p);
    putchw(putp, putf, &p);
                break;
    case 'd':
    case 'i':
        p.base = 10;
#ifdef PRINTF_LONG_SUPPORT
#ifdef PRINTF_LONG_LONG_SUPPORT
    if (2 == lng)
        lli2a(va_arg(va, long long int), &p);
    else
#endif
    if (1 == lng)
        li2a(va_arg(va, long int), &p);
    else
#endif
        i2a(va_arg(va, int), &p);
    putchw(putp, putf, &p);
    break;
#ifdef SIZEOF_POINTER
    case 'p':
        p.alt = 1;
# if defined(SIZEOF_INT) && SIZEOF_POINTER <= SIZEOF_INT
    lng = 0;
# elif defined(SIZEOF_LONG) && SIZEOF_POINTER <= SIZEOF_LONG
    lng = 1;
# elif defined(SIZEOF_LONG_LONG) && SIZEOF_POINTER <= SIZEOF_LONG_LONG
    lng = 2;
# endif
#endif
    case 'x':
    case 'X':
        p.base = 16;
        p.uc = (ch == 'X')?1:0;
#ifdef PRINTF_LONG_SUPPORT
#ifdef PRINTF_LONG_LONG_SUPPORT
    if (2 == lng)
        ulli2a(va_arg(va, unsigned long long int), &p);
    else
#endif
    if (1 == lng)
        uli2a(va_arg(va, unsigned long int), &p);
    else
#endif
        ui2a(va_arg(va, unsigned int), &p);
    putchw(putp, putf, &p);
    break;
    case 'o':
        p.base = 8;
        ui2a(va_arg(va, unsigned int), &p);
        putchw(putp, putf, &p);
        break;
    case 'c':
        putf(putp, (char)(va_arg(va, int)));
        break;
    case 's':
        p.bf = va_arg(va, char *);
        putchw(putp, putf, &p);
        p.bf = bf;
        break;
    case '%':
        putf(putp, ch);
        break;
    case 'f':
    case 'F':
        fval  = va_arg(va, double);
        sign = 0;
        if (fval < 0)
           {
               sign    = 1;
               p.width--;
               fval    = - fval;
           }
           else if (p.sign) {
               sign = 2;
               p.width--;
           }

        fpart = (int)fval;

        fiter = 0;
        while (fpart != 0)
        {
            temp_buffer[fiter++] = fpart % 10;
            fpart = fpart / 10;

        }
        fiter--;
        if (fiter == -1)
            p.width--;
        /* Leading zeros */
        if (p.lz) {

            if (sign == 1)
                putf(putp, '-');
            else if (sign == 2)
                putf(putp, '+');

            while (p.width-- > p.prec + fiter + 2)
            {
                putf(putp, '0');
            }
        }
        else
        {

            while (p.width-- > p.prec + fiter + 2)
            {
                putf(putp, ' ');
            }

            if (sign == 1)
               putf(putp, '-');
            else if (sign == 2)
               putf(putp, '+');

        }

        if (fiter == -1)
            putf(putp, '0');
        while (fiter > -1)
        {
            putf(putp, '0' + (temp_buffer[fiter--]));
        }

        putf(putp, '.');
        ffactor = 1;
        while (p.prec-- > 0)
        {
            ffactor *= 10;
            fpart = (int)((fval - (int)fval)*ffactor);
            if (fpart == 0)
                putf(putp, '0');
        }
        fiter = 0;
        while (fpart != 0)
        {
            temp_buffer[fiter++] = fpart % 10;
            fpart = fpart / 10;

        }
        fiter--;
        while (fiter > -1)
        {
            putf(putp, '0' + (temp_buffer[fiter--]));
        }
        break;
    default:
        break;
            }
        }
    }
 abort:;
}


#if TINYPRINTF_DEFINE_TFP_PRINTF
static putcf stdout_putf;
static void *stdout_putp;

void init_printf(void *putp, putcf putf)
{
    stdout_putf = putf;
    stdout_putp = putp;
}

void tfp_printf(char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    tfp_format(stdout_putp, stdout_putf, fmt, va);
    va_end(va);
}
#endif

#if TINYPRINTF_DEFINE_TFP_SPRINTF
struct _vsnprintf_putcf_data
{
  size_t dest_capacity;
  char *dest;
  size_t num_chars;
};

static void _vsnprintf_putcf(void *p, char c)
{
  struct _vsnprintf_putcf_data *data = (struct _vsnprintf_putcf_data*)p;
  if (data->num_chars < data->dest_capacity)
    data->dest[data->num_chars] = c;
  data->num_chars ++;
}

int tfp_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
  struct _vsnprintf_putcf_data data;

  data.dest = str;
  data.dest_capacity = size ? size - 1 : 0;
  data.num_chars = 0;
  tfp_format(&data, _vsnprintf_putcf, format, ap);

  if (data.num_chars < data.dest_capacity)
    data.dest[data.num_chars] = '\0';
  else if (size)
    data.dest[data.dest_capacity] = '\0';

  return data.num_chars;
}

int tfp_snprintf(char *str, size_t size, const char *format, ...)
{
  va_list ap;
  int retval;

  va_start(ap, format);
  retval = tfp_vsnprintf(str, size, format, ap);
  va_end(ap);
  return retval;
}

struct _vsprintf_putcf_data
{
  char *dest;
  size_t num_chars;
};

static void _vsprintf_putcf(void *p, char c)
{
  struct _vsprintf_putcf_data *data = (struct _vsprintf_putcf_data*)p;
  data->dest[data->num_chars++] = c;
}

int tfp_vsprintf(char *str, const char *format, va_list ap)
{
  struct _vsprintf_putcf_data data;
  data.dest = str;
  data.num_chars = 0;
  tfp_format(&data, _vsprintf_putcf, format, ap);
  data.dest[data.num_chars] = '\0';
  return data.num_chars;
}

int tfp_sprintf(char *str, const char *format, ...)
{
  va_list ap;
  int retval;

  va_start(ap, format);
  retval = tfp_vsprintf(str, format, ap);
  va_end(ap);
  return retval;
}

#endif

int tfp_vsscanf(const char* str, const char* format, va_list ap)
{
        int value, tmp;
        float  fvalue;
        double Fvalue;
        int count = 0;
        int pos;
        char neg, fmt_code;

        for (count = 0; *format != 0 && *str != 0; format++, str++)
        {
            while (*format == ' ' && *format != 0)  format++;

            if (*format == 0)
                    break;

            while (*str == ' ' && *str != 0)    str++;

            if (*str == 0)
                    break;

            if (*format == '%')
            {
                format++;
                if (*format == 'n')
                {
                    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
                    {
                        fmt_code = 'x';
                        str += 2;
                    }
                    else
                    if (str[0] == 'b')
                    {
                        fmt_code = 'b';
                        str++;
                    }
                    else
                        fmt_code = 'd';
                }
                    else
                        fmt_code = *format;

                switch (fmt_code)
                {
                    case 'x':
                    case 'X':
                        for (value = 0, pos = 0; *str != 0; str++, pos++)
                        {
                            if ('0' <= *str && *str <= '9')
                                    tmp = *str - '0';
                            else
                            if ('a' <= *str && *str <= 'f')
                                    tmp = *str - 'a' + 10;
                            else
                            if ('A' <= *str && *str <= 'F')
                                    tmp = *str - 'A' + 10;
                            else
                                    break;

                            value *= 16;
                            value += tmp;
                        }
                        if (pos == 0)
                            return count;
                        *(va_arg(ap, int*)) = value;
                        count++;
                        break;

                    case 'b':
                        for (value = 0, pos = 0; *str != 0; str++, pos++)
                        {
                             if (*str != '0' && *str != '1')
                                break;

                            value *= 2;
                            value += *str - '0';
                        }

                        if (pos == 0)
                            return count;

                        *(va_arg(ap, int*)) = value;
                        count++;
                        break;

                    case 'd':
                        if (*str == '-')
                        {
                            neg = 1;
                            str++;
                        }
                        else
                            neg = 0;
                        for (value = 0, pos = 0; *str != 0; str++, pos++)
                        {
                            if ('0' <= *str && *str <= '9')
                                    value = value*10 + (int)(*str - '0');
                            else
                                 break;
                        }
                        if (pos == 0)
                            return count;
                        *(va_arg(ap, int*)) = neg ? -value : value;
                        count++;
                        break;

                    case 'f':
                        if (*str == '-')
                        {
                            neg = 1;
                            str++;
                        }
                        else
                            neg = 0;

                        int point_flag = 0;
                        int exp        = 0;
                        for (fvalue = 0, pos = 0; *str != 0 ; str++, pos++)
                        {
                            if (*str == '.')
                            {
                                point_flag = 1;
                                str++;
                            }
                            if ('0' <= *str && *str <= '9')
                                fvalue = fvalue*10 + (int)(*str - '0');
                            else
                                 break;

                            if (point_flag == 1)
                                exp++;

                        }

                        if (pos == 0)
                            return count;

                        for (pos = 0; pos < exp; pos++)
                            fvalue = fvalue/10.0;

                        *(va_arg(ap, float*)) = neg ? -fvalue : fvalue;
                        count++;
                        break;

                    case 'F':
                       if (*str == '-')
                       {
                           neg = 1;
                           str++;
                       }
                       else
                           neg = 0;

                       int Fpoint_flag = 0;
                       int Fexp        = 0;
                       for (Fvalue = 0, pos = 0; *str != 0 ; str++, pos++)
                       {

                           if (*str == '.')
                           {
                               Fpoint_flag = 1;
                               str++;
                           }
                           if ('0' <= *str && *str <= '9')
                                Fvalue = Fvalue*10 + (int)(*str - '0');
                           else
                                break;

                           if (Fpoint_flag == 1)
                               Fexp++;

                       }

                       if (pos == 0)
                            return count;
                       for (pos = 0; pos < Fexp; pos++)
                           Fvalue = Fvalue/10.0;
                       *(va_arg(ap, double*)) = neg ? -Fvalue : Fvalue;
                       count++;
                       break;

                    case 'c':
                        *(va_arg(ap, char*)) = *str;
                        count++;
                        break;

                    case 's':
                        pos = 0;
                        char* tab = va_arg(ap, char*);
                        while (*str != ' ' && *str != 0)
                            *(tab++) = *str++;
                        *tab = 0;
                        count++;
                        break;

                    default:
                        return count;
                    }
            }
            else
            {
                if (*format != *str)
                    break;
            }
        }

    return count;
}
