#include <string.h>
#include <malloc.h>

void *memset(void *dst, int ch, size_t n) {
    return __builtin_memset(dst, ch, n);
}

void *memmove(void *dst, const void *src, size_t n) {
    return __builtin_memmove(dst, src, n);
}

void *memcpy(void *dst, const void *src, size_t size) {
    return __builtin_memcpy(dst, src, size);
}

int memcmp(const void *lhs, const void *rhs, size_t n) {
    return __builtin_memcmp(lhs, rhs, n);
}

void *memchr(const void *ptr, int ch, size_t count) {
    return __builtin_memchr(ptr, ch, count);
}

char *strchr(const char *str, int ch) {
    return __builtin_strchr(str, ch);
}

int strcmp(const char *lhs, const char *rhs) {
    return __builtin_strcmp(lhs, rhs);
}

size_t strlen(const char *str) {
    return __builtin_strlen(str);
}

char *strcpy(char *restrict dest, const char *restrict src) {
    return __builtin_strcpy(dest, src);
}

char *strdup(const char *str) {
    size_t siz;
    char *copy;
    siz = strlen(str) + 1;
    if ((copy = malloc(siz)) == NULL)
        return NULL;
    memcpy(copy, str, siz);
    return copy;
}

// memmem source: bionic/libc/upstream-openbsd/lib/libc/string/memmem.c

static char *twobyte_memmem(const unsigned char *h, size_t k, const unsigned char *n) {
    uint16_t nw = n[0]<<8 | n[1], hw = h[0]<<8 | h[1];
    for (h+=2, k-=2; k; k--, hw = hw<<8 | *h++)
        if (hw == nw) return (char *)h-2;
    return hw == nw ? (char *)h-2 : 0;
}

static char *threebyte_memmem(const unsigned char *h, size_t k, const unsigned char *n) {
    uint32_t nw = n[0]<<24 | n[1]<<16 | n[2]<<8;
    uint32_t hw = h[0]<<24 | h[1]<<16 | h[2]<<8;
    for (h+=3, k-=3; k; k--, hw = (hw|*h++)<<8)
        if (hw == nw) return (char *)h-3;
    return hw == nw ? (char *)h-3 : 0;
}

static char *fourbyte_memmem(const unsigned char *h, size_t k, const unsigned char *n) {
    uint32_t nw = n[0]<<24 | n[1]<<16 | n[2]<<8 | n[3];
    uint32_t hw = h[0]<<24 | h[1]<<16 | h[2]<<8 | h[3];
    for (h+=4, k-=4; k; k--, hw = hw<<8 | *h++)
        if (hw == nw) return (char *)h-4;
    return hw == nw ? (char *)h-4 : 0;
}

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

#define BITOP(a,b,op) \
 ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

/*
 * Maxime Crochemore and Dominique Perrin, Two-way string-matching,
 * Journal of the ACM, 38(3):651-675, July 1991.
 *
 */
static char *twoway_memmem(
        const unsigned char *h, const unsigned char *z, const unsigned char *n, size_t l) {
    size_t i, ip, jp, k, p, ms, p0, mem, mem0;
    size_t byteset[32 / sizeof(size_t)] = { 0 };
    size_t shift[256];

    /* Computing length of needle and fill shift table */
    for (i=0; i<l; i++)
        BITOP(byteset, n[i], |=), shift[n[i]] = i+1;

    /* Compute maximal suffix */
    ip = -1; jp = 0; k = p = 1;
    while (jp+k<l) {
        if (n[ip+k] == n[jp+k]) {
            if (k == p) {
                jp += p;
                k = 1;
            } else k++;
        } else if (n[ip+k] > n[jp+k]) {
            jp += k;
            k = 1;
            p = jp - ip;
        } else {
            ip = jp++;
            k = p = 1;
        }
    }
    ms = ip;
    p0 = p;

    /* And with the opposite comparison */
    ip = -1; jp = 0; k = p = 1;
    while (jp+k<l) {
        if (n[ip+k] == n[jp+k]) {
            if (k == p) {
                jp += p;
                k = 1;
            } else k++;
        } else if (n[ip+k] < n[jp+k]) {
            jp += k;
            k = 1;
            p = jp - ip;
        } else {
            ip = jp++;
            k = p = 1;
        }
    }
    if (ip+1 > ms+1) ms = ip;
    else p = p0;

    /* Periodic needle? */
    if (memcmp(n, n+p, ms+1)) {
        mem0 = 0;
        p = MAX(ms, l-ms-1) + 1;
    } else mem0 = l-p;
    mem = 0;

    /* Search loop */
    for (;;) {
        /* If remainder of haystack is shorter than needle, done */
        if (z-h < l) return 0;

        /* Check last byte first; advance by shift on mismatch */
        if (BITOP(byteset, h[l-1], &)) {
            k = l-shift[h[l-1]];
            if (k) {
                if (k < mem) k = mem;
                h += k;
                mem = 0;
                continue;
            }
        } else {
            h += l;
            mem = 0;
            continue;
        }

        /* Compare right half */
        for (k=MAX(ms+1,mem); k<l && n[k] == h[k]; k++);
        if (k < l) {
            h += k-ms;
            mem = 0;
            continue;
        }
        /* Compare left half */
        for (k=ms+1; k>mem && n[k-1] == h[k-1]; k--);
        if (k <= mem) return (char *)h;
        h += p;
        mem = mem0;
    }
}

void *memmem(const void *h0, size_t k, const void *n0, size_t l) {
    const unsigned char *h = h0, *n = n0;

    /* Return immediately on empty needle */
    if (!l) return (void *)h;

    /* Return immediately when needle is longer than haystack */
    if (k<l) return 0;

    /* Use faster algorithms for short needles */
    h = memchr(h0, *n, k);
    if (!h || l==1) return (void *)h;
    k -= h - (const unsigned char *)h0;
    if (k<l) return 0;
    if (l==2) return twobyte_memmem(h, k, n);
    if (l==3) return threebyte_memmem(h, k, n);
    if (l==4) return fourbyte_memmem(h, k, n);

    return twoway_memmem(h, h+k, n, l);
}

// Source: bionic/libc/upstream-openbsd/lib/libc/string/strlcpy.c
size_t strlcpy(char *dst, const char *src, size_t dsize) {
    const char *osrc = src;
    size_t nleft = dsize;

    /* Copy as many bytes as will fit. */
    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0) {
        if (dsize != 0)
            *dst = '\0';		/* NUL-terminate dst */
        while (*src++)
            ;
    }

    return(src - osrc - 1);	/* count does not include NUL */
}

// Source: bionic/libc/upstream-openbsd/lib/libc/string/strtok.c
char *strtok_r(char *s, const char *delim, char **last) {
    const char *spanp;
    int c, sc;
    char *tok;

    if (s == NULL && (s = *last) == NULL)
        return (NULL);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
    cont:
    c = *s++;
    for (spanp = delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {		/* no non-delimiter characters */
        *last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = '\0';
                *last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

// strcasecmp source: bionic/libc/upstream-openbsd/lib/libc/string/strcasecmp.c

typedef unsigned char u_char;

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static const u_char charmap[] = {
        '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
        '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
        '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
        '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
        '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
        '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
        '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
        '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
        '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
        '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
        '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
        '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
        '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
        '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
        '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
        '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
        '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
        '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
        '\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
        '\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
        '\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
        '\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
        '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int strcasecmp(const char *s1, const char *s2) {
    const u_char *cm = charmap;
    const u_char *us1 = (const u_char *)s1;
    const u_char *us2 = (const u_char *)s2;

    while (cm[*us1] == cm[*us2++])
        if (*us1++ == '\0')
            return (0);
    return (cm[*us1] - cm[*--us2]);
}
