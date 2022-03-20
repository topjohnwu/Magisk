// Copyright (c) 2018-present, iQIYI, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// Created by caikelun on 2018-04-11.

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <regex.h>
#include <setjmp.h>
#include <errno.h>
#include "queue.h"
#include "tree.h"
#include "xh_errno.h"
#include "xh_log.h"
#include "xh_elf.h"
#include "xh_version.h"
#include "xh_core.h"

#define XH_CORE_DEBUG 0

//registered hook info collection
typedef struct xh_core_hook_info
{
#if XH_CORE_DEBUG
    char     *pathname_regex_str;
#endif
    regex_t   pathname_regex;
    char     *symbol;
    void     *new_func;
    void    **old_func;
    TAILQ_ENTRY(xh_core_hook_info,) link;
} xh_core_hook_info_t;
typedef TAILQ_HEAD(xh_core_hook_info_queue, xh_core_hook_info,) xh_core_hook_info_queue_t;

//ignored hook info collection
typedef struct xh_core_ignore_info
{
#if XH_CORE_DEBUG
    char     *pathname_regex_str;
#endif
    regex_t   pathname_regex;
    char     *symbol; //NULL meaning for all symbols
    TAILQ_ENTRY(xh_core_ignore_info,) link;
} xh_core_ignore_info_t;
typedef TAILQ_HEAD(xh_core_ignore_info_queue, xh_core_ignore_info,) xh_core_ignore_info_queue_t;

//required info from /proc/self/maps
typedef struct xh_core_map_info
{
    char      *pathname;
    uintptr_t  base_addr;
    xh_elf_t   elf;
    RB_ENTRY(xh_core_map_info) link;
} xh_core_map_info_t;
static __inline__ int xh_core_map_info_cmp(xh_core_map_info_t *a, xh_core_map_info_t *b)
{
    return strcmp(a->pathname, b->pathname);
}
typedef RB_HEAD(xh_core_map_info_tree, xh_core_map_info) xh_core_map_info_tree_t;
RB_GENERATE_STATIC(xh_core_map_info_tree, xh_core_map_info, link, xh_core_map_info_cmp)

//signal handler for SIGSEGV
//for xh_elf_init(), xh_elf_hook(), xh_elf_check_elfheader()
static int              xh_core_sigsegv_enable = 1; //enable by default
static struct sigaction xh_core_sigsegv_act_old;
static volatile int     xh_core_sigsegv_flag = 0;
static sigjmp_buf       xh_core_sigsegv_env;
static void xh_core_sigsegv_handler(int sig)
{
    (void)sig;
    
    if(xh_core_sigsegv_flag)
        siglongjmp(xh_core_sigsegv_env, 1);
    else
        sigaction(SIGSEGV, &xh_core_sigsegv_act_old, NULL);
}
static int xh_core_add_sigsegv_handler()
{
    struct sigaction act;

    if(!xh_core_sigsegv_enable) return 0;
    
    if(0 != sigemptyset(&act.sa_mask)) return (0 == errno ? XH_ERRNO_UNKNOWN : errno);
    act.sa_handler = xh_core_sigsegv_handler;
    
    if(0 != sigaction(SIGSEGV, &act, &xh_core_sigsegv_act_old))
        return (0 == errno ? XH_ERRNO_UNKNOWN : errno);

    return 0;
}
static void xh_core_del_sigsegv_handler()
{
    if(!xh_core_sigsegv_enable) return;
    
    sigaction(SIGSEGV, &xh_core_sigsegv_act_old, NULL);
}


static xh_core_hook_info_queue_t   xh_core_hook_info   = TAILQ_HEAD_INITIALIZER(xh_core_hook_info);
static xh_core_ignore_info_queue_t xh_core_ignore_info = TAILQ_HEAD_INITIALIZER(xh_core_ignore_info);
static xh_core_map_info_tree_t     xh_core_map_info    = RB_INITIALIZER(&xh_core_map_info);
static pthread_mutex_t             xh_core_mutex       = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t              xh_core_cond        = PTHREAD_COND_INITIALIZER;
static volatile int                xh_core_inited      = 0;
static volatile int                xh_core_init_ok     = 0;
static volatile int                xh_core_async_inited  = 0;
static volatile int                xh_core_async_init_ok = 0;
static pthread_mutex_t             xh_core_refresh_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t                   xh_core_refresh_thread_tid;
static volatile int                xh_core_refresh_thread_running = 0;
static volatile int                xh_core_refresh_thread_do = 0;


int xh_core_register(const char *pathname_regex_str, const char *symbol,
                     void *new_func, void **old_func)
{
    xh_core_hook_info_t *hi;
    regex_t              regex;

    if(NULL == pathname_regex_str || NULL == symbol || NULL == new_func) return XH_ERRNO_INVAL;

    if(xh_core_inited)
    {
        XH_LOG_ERROR("do not register hook after refresh(): %s, %s", pathname_regex_str, symbol);
        return XH_ERRNO_INVAL;
    }

    if(0 != regcomp(&regex, pathname_regex_str, REG_NOSUB)) return XH_ERRNO_INVAL;

    if(NULL == (hi = malloc(sizeof(xh_core_hook_info_t)))) return XH_ERRNO_NOMEM;
    if(NULL == (hi->symbol = strdup(symbol)))
    {
        free(hi);
        return XH_ERRNO_NOMEM;
    }
#if XH_CORE_DEBUG
    if(NULL == (hi->pathname_regex_str = strdup(pathname_regex_str)))
    {
        free(hi->symbol);
        free(hi);
        return XH_ERRNO_NOMEM;
    }
#endif
    hi->pathname_regex = regex;
    hi->new_func = new_func;
    hi->old_func = old_func;
    
    pthread_mutex_lock(&xh_core_mutex);
    TAILQ_INSERT_TAIL(&xh_core_hook_info, hi, link);
    pthread_mutex_unlock(&xh_core_mutex);

    return 0;
}

int xh_core_ignore(const char *pathname_regex_str, const char *symbol)
{
    xh_core_ignore_info_t *ii;
    regex_t                regex;

    if(NULL == pathname_regex_str) return XH_ERRNO_INVAL;

    if(xh_core_inited)
    {
        XH_LOG_ERROR("do not ignore hook after refresh(): %s, %s", pathname_regex_str, symbol ? symbol : "ALL");
        return XH_ERRNO_INVAL;
    }

    if(0 != regcomp(&regex, pathname_regex_str, REG_NOSUB)) return XH_ERRNO_INVAL;

    if(NULL == (ii = malloc(sizeof(xh_core_ignore_info_t)))) return XH_ERRNO_NOMEM;
    if(NULL != symbol)
    {
        if(NULL == (ii->symbol = strdup(symbol)))
        {
            free(ii);
            return XH_ERRNO_NOMEM;
        }
    }
    else
    {
        ii->symbol = NULL; //ignore all symbols
    }
#if XH_CORE_DEBUG
    if(NULL == (ii->pathname_regex_str = strdup(pathname_regex_str)))
    {
        free(ii->symbol);
        free(ii);
        return XH_ERRNO_NOMEM;
    }
#endif
    ii->pathname_regex = regex;

    pthread_mutex_lock(&xh_core_mutex);
    TAILQ_INSERT_TAIL(&xh_core_ignore_info, ii, link);
    pthread_mutex_unlock(&xh_core_mutex);

    return 0;
}

static int xh_core_check_elf_header(uintptr_t base_addr, const char *pathname)
{
    if(!xh_core_sigsegv_enable)
    {
        return xh_elf_check_elfheader(base_addr);
    }
    else
    {
        int ret = XH_ERRNO_UNKNOWN;
        
        xh_core_sigsegv_flag = 1;
        if(0 == sigsetjmp(xh_core_sigsegv_env, 1))
        {
            ret = xh_elf_check_elfheader(base_addr);
        }
        else
        {
            ret = XH_ERRNO_SEGVERR;
            XH_LOG_WARN("catch SIGSEGV when check_elfheader: %s", pathname);
        }
        xh_core_sigsegv_flag = 0;
        return ret;
    }
}

static void xh_core_hook_impl(xh_core_map_info_t *mi)
{
    //init
    if(0 != xh_elf_init(&(mi->elf), mi->base_addr, mi->pathname)) return;
    
    //hook
    xh_core_hook_info_t   *hi;
    xh_core_ignore_info_t *ii;
    int ignore;
    TAILQ_FOREACH(hi, &xh_core_hook_info, link) //find hook info
    {
        if(0 == regexec(&(hi->pathname_regex), mi->pathname, 0, NULL, 0))
        {
            ignore = 0;
            TAILQ_FOREACH(ii, &xh_core_ignore_info, link) //find ignore info
            {
                if(0 == regexec(&(ii->pathname_regex), mi->pathname, 0, NULL, 0))
                {
                    if(NULL == ii->symbol) //ignore all symbols
                        return;

                    if(0 == strcmp(ii->symbol, hi->symbol)) //ignore the current symbol
                    {
                        ignore = 1;
                        break;
                    }
                }
            }

            if(0 == ignore)
                xh_elf_hook(&(mi->elf), hi->symbol, hi->new_func, hi->old_func);
        }
    }
}

static void xh_core_hook(xh_core_map_info_t *mi)
{
    if(!xh_core_sigsegv_enable)
    {
        xh_core_hook_impl(mi);
    }
    else
    {    
        xh_core_sigsegv_flag = 1;
        if(0 == sigsetjmp(xh_core_sigsegv_env, 1))
        {
            xh_core_hook_impl(mi);
        }
        else
        {
            XH_LOG_WARN("catch SIGSEGV when init or hook: %s", mi->pathname);
        }
        xh_core_sigsegv_flag = 0;
    }
}

static void xh_core_refresh_impl()
{
    char                     line[512];
    FILE                    *fp;
    uintptr_t                base_addr;
    char                     perm[5];
    unsigned long            offset;
    int                      pathname_pos;
    char                    *pathname;
    size_t                   pathname_len;
    xh_core_map_info_t      *mi, *mi_tmp;
    xh_core_map_info_t       mi_key;
    xh_core_hook_info_t     *hi;
    xh_core_ignore_info_t   *ii;
    int                      match;
    xh_core_map_info_tree_t  map_info_refreshed = RB_INITIALIZER(&map_info_refreshed);

    if(NULL == (fp = fopen("/proc/self/maps", "r")))
    {
        XH_LOG_ERROR("fopen /proc/self/maps failed");
        return;
    }

    while(fgets(line, sizeof(line), fp))
    {
        if(sscanf(line, "%"PRIxPTR"-%*lx %4s %lx %*x:%*x %*d%n", &base_addr, perm, &offset, &pathname_pos) != 3) continue;

        //check permission
        if(perm[0] != 'r') continue;
        if(perm[3] != 'p') continue; //do not touch the shared memory

        //check offset
        //
        //We are trying to find ELF header in memory.
        //It can only be found at the beginning of a mapped memory regions
        //whose offset is 0.
        if(0 != offset) continue;

        //get pathname
        while(isspace(line[pathname_pos]) && pathname_pos < (int)(sizeof(line) - 1))
            pathname_pos += 1;
        if(pathname_pos >= (int)(sizeof(line) - 1)) continue;
        pathname = line + pathname_pos;
        pathname_len = strlen(pathname);
        if(0 == pathname_len) continue;
        if(pathname[pathname_len - 1] == '\n')
        {
            pathname[pathname_len - 1] = '\0';
            pathname_len -= 1;
        }
        if(0 == pathname_len) continue;
        if('[' == pathname[0]) continue;

        //check pathname
        //if we need to hook this elf?
        match = 0;
        TAILQ_FOREACH(hi, &xh_core_hook_info, link) //find hook info
        {
            if(0 == regexec(&(hi->pathname_regex), pathname, 0, NULL, 0))
            {
                TAILQ_FOREACH(ii, &xh_core_ignore_info, link) //find ignore info
                {
                    if(0 == regexec(&(ii->pathname_regex), pathname, 0, NULL, 0))
                    {
                        if(NULL == ii->symbol)
                            goto check_finished;

                        if(0 == strcmp(ii->symbol, hi->symbol))
                            goto check_continue;
                    }
                }

                match = 1;
            check_continue:
                break;
            }
        }
    check_finished:
        if(0 == match) continue;

        //check elf header format
        //We are trying to do ELF header checking as late as possible.
        if(0 != xh_core_check_elf_header(base_addr, pathname)) continue;
        
        //check existed map item
        mi_key.pathname = pathname;
        if(NULL != (mi = RB_FIND(xh_core_map_info_tree, &xh_core_map_info, &mi_key)))
        {
            //exist
            RB_REMOVE(xh_core_map_info_tree, &xh_core_map_info, mi);
            
            //repeated?
            //We only keep the first one, that is the real base address
            if(NULL != RB_INSERT(xh_core_map_info_tree, &map_info_refreshed, mi))
            {
#if XH_CORE_DEBUG
                XH_LOG_DEBUG("repeated map info when update: %s", line);
#endif
                free(mi->pathname);
                free(mi);
                continue;
            }

            //re-hook if base_addr changed
            if(mi->base_addr != base_addr)
            {
                mi->base_addr = base_addr;
                xh_core_hook(mi);
            }
        }
        else
        {
            //not exist, create a new map info
            if(NULL == (mi = (xh_core_map_info_t *)malloc(sizeof(xh_core_map_info_t)))) continue;
            if(NULL == (mi->pathname = strdup(pathname)))
            {
                free(mi);
                continue;
            }
            mi->base_addr = base_addr;

            //repeated?
            //We only keep the first one, that is the real base address
            if(NULL != RB_INSERT(xh_core_map_info_tree, &map_info_refreshed, mi))
            {
#if XH_CORE_DEBUG
                XH_LOG_DEBUG("repeated map info when create: %s", line);
#endif
                free(mi->pathname);
                free(mi);
                continue;
            }

            //hook
            xh_core_hook(mi); //hook
        }
    }
    fclose(fp);

    //free all missing map item, maybe dlclosed?
    RB_FOREACH_SAFE(mi, xh_core_map_info_tree, &xh_core_map_info, mi_tmp)
    {
#if XH_CORE_DEBUG
        XH_LOG_DEBUG("remove missing map info: %s", mi->pathname);
#endif
        RB_REMOVE(xh_core_map_info_tree, &xh_core_map_info, mi);
        if(mi->pathname) free(mi->pathname);
        free(mi);
    }

    //save the new refreshed map info tree
    xh_core_map_info = map_info_refreshed;

    XH_LOG_INFO("map refreshed");
    
#if XH_CORE_DEBUG
    RB_FOREACH(mi, xh_core_map_info_tree, &xh_core_map_info)
        XH_LOG_DEBUG("  %"PRIxPTR" %s\n", mi->base_addr, mi->pathname);
#endif
}

static void *xh_core_refresh_thread_func(void *arg)
{
    (void)arg;
    
    pthread_setname_np(pthread_self(), "xh_refresh_loop");

    while(xh_core_refresh_thread_running)
    {
        //waiting for a refresh task or exit
        pthread_mutex_lock(&xh_core_mutex);
        while(!xh_core_refresh_thread_do && xh_core_refresh_thread_running)
        {
            pthread_cond_wait(&xh_core_cond, &xh_core_mutex);
        }
        if(!xh_core_refresh_thread_running)
        {
            pthread_mutex_unlock(&xh_core_mutex);
            break;
        }
        xh_core_refresh_thread_do = 0;
        pthread_mutex_unlock(&xh_core_mutex);

        //refresh
        pthread_mutex_lock(&xh_core_refresh_mutex);
        xh_core_refresh_impl();
        pthread_mutex_unlock(&xh_core_refresh_mutex);
    }

    return NULL;
}

static void xh_core_init_once()
{
    if(xh_core_inited) return;

    pthread_mutex_lock(&xh_core_mutex);

    if(xh_core_inited) goto end;

    xh_core_inited = 1;
    
    //dump debug info
    XH_LOG_INFO("%s\n", xh_version_str_full());
#if XH_CORE_DEBUG
    xh_core_hook_info_t *hi;
    TAILQ_FOREACH(hi, &xh_core_hook_info, link)
        XH_LOG_INFO("  hook: %s @ %s, (%p, %p)\n", hi->symbol, hi->pathname_regex_str,
                    hi->new_func, hi->old_func);
    xh_core_ignore_info_t *ii;
    TAILQ_FOREACH(ii, &xh_core_ignore_info, link)
        XH_LOG_INFO("  ignore: %s @ %s\n", ii->symbol ? ii->symbol : "ALL ",
                    ii->pathname_regex_str);
#endif
    
    //register signal handler
    if(0 != xh_core_add_sigsegv_handler()) goto end;

    //OK
    xh_core_init_ok = 1;

 end:
    pthread_mutex_unlock(&xh_core_mutex);
}

static void xh_core_init_async_once()
{
    if(xh_core_async_inited) return;
    
    pthread_mutex_lock(&xh_core_mutex);
    
    if(xh_core_async_inited) goto end;

    xh_core_async_inited = 1;
    
    //create async refresh thread
    xh_core_refresh_thread_running = 1;
    if(0 != pthread_create(&xh_core_refresh_thread_tid, NULL, &xh_core_refresh_thread_func, NULL))
    {
        xh_core_refresh_thread_running = 0;
        goto end;
    }

    //OK
    xh_core_async_init_ok = 1;
    
 end:
    pthread_mutex_unlock(&xh_core_mutex);
}

int xh_core_refresh(int async)
{
    //init
    xh_core_init_once();
    if(!xh_core_init_ok) return XH_ERRNO_UNKNOWN;

    if(async)
    {
        //init for async
        xh_core_init_async_once();
        if(!xh_core_async_init_ok) return XH_ERRNO_UNKNOWN;
    
        //refresh async
        pthread_mutex_lock(&xh_core_mutex);
        xh_core_refresh_thread_do = 1;
        pthread_cond_signal(&xh_core_cond);
        pthread_mutex_unlock(&xh_core_mutex);
    }
    else
    {
        //refresh sync
        pthread_mutex_lock(&xh_core_refresh_mutex);
        xh_core_refresh_impl();
        pthread_mutex_unlock(&xh_core_refresh_mutex);
    }
    
    return 0;
}

void xh_core_clear()
{
    //stop the async refresh thread
    if(xh_core_async_init_ok)
    {
        pthread_mutex_lock(&xh_core_mutex);
        xh_core_refresh_thread_running = 0;
        pthread_cond_signal(&xh_core_cond);
        pthread_mutex_unlock(&xh_core_mutex);
        
        pthread_join(xh_core_refresh_thread_tid, NULL);
        xh_core_async_init_ok = 0;
    }
    xh_core_async_inited = 0;

    //unregister the sig handler
    if(xh_core_init_ok)
    {
        xh_core_del_sigsegv_handler();
        xh_core_init_ok = 0;
    }
    xh_core_inited = 0;

    pthread_mutex_lock(&xh_core_mutex);
    pthread_mutex_lock(&xh_core_refresh_mutex);
        
    //free all map info
    xh_core_map_info_t *mi, *mi_tmp;
    RB_FOREACH_SAFE(mi, xh_core_map_info_tree, &xh_core_map_info, mi_tmp)
    {
        RB_REMOVE(xh_core_map_info_tree, &xh_core_map_info, mi);
        if(mi->pathname) free(mi->pathname);
        free(mi);
    }

    //free all hook info
    xh_core_hook_info_t *hi, *hi_tmp;
    TAILQ_FOREACH_SAFE(hi, &xh_core_hook_info, link, hi_tmp)
    {
        TAILQ_REMOVE(&xh_core_hook_info, hi, link);
#if XH_CORE_DEBUG
        free(hi->pathname_regex_str);
#endif
        regfree(&(hi->pathname_regex));
        free(hi->symbol);
        free(hi);
    }

    //free all ignore info
    xh_core_ignore_info_t *ii, *ii_tmp;
    TAILQ_FOREACH_SAFE(ii, &xh_core_ignore_info, link, ii_tmp)
    {
        TAILQ_REMOVE(&xh_core_ignore_info, ii, link);
#if XH_CORE_DEBUG
        free(ii->pathname_regex_str);
#endif
        regfree(&(ii->pathname_regex));
        free(ii->symbol);
        free(ii);
    }

    pthread_mutex_unlock(&xh_core_refresh_mutex);
    pthread_mutex_unlock(&xh_core_mutex);
}

void xh_core_enable_debug(int flag)
{
    xh_log_priority = (flag ? ANDROID_LOG_DEBUG : ANDROID_LOG_WARN);
}

void xh_core_enable_sigsegv_protection(int flag)
{
    xh_core_sigsegv_enable = (flag ? 1 : 0);
}
