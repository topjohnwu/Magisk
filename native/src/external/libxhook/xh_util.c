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

#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <elf.h>
#include <link.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "xh_util.h"
#include "xh_errno.h"
#include "xh_log.h"

#define PAGE_START(addr) ((addr) & PAGE_MASK)
#define PAGE_END(addr)   (PAGE_START(addr + sizeof(uintptr_t) - 1) + PAGE_SIZE)
#define PAGE_COVER(addr) (PAGE_END(addr) - PAGE_START(addr))

int xh_util_get_mem_protect(uintptr_t addr, size_t len, const char *pathname, unsigned int *prot)
{
    uintptr_t  start_addr = addr;
    uintptr_t  end_addr = addr + len;
    FILE      *fp;
    char       line[512];
    uintptr_t  start, end;
    char       perm[5];
    int        load0 = 1;
    int        found_all = 0;

    *prot = 0;
    
    if(NULL == (fp = fopen("/proc/self/maps", "r"))) return XH_ERRNO_BADMAPS;
    
    while(fgets(line, sizeof(line), fp))
    {
        if(NULL != pathname)
            if(NULL == strstr(line, pathname)) continue;
        
        if(sscanf(line, "%"PRIxPTR"-%"PRIxPTR" %4s ", &start, &end, perm) != 3) continue;
        
        if(perm[3] != 'p') continue;
        
        if(start_addr >= start && start_addr < end)
        {
            if(load0)
            {
                //first load segment
                if(perm[0] == 'r') *prot |= PROT_READ;
                if(perm[1] == 'w') *prot |= PROT_WRITE;
                if(perm[2] == 'x') *prot |= PROT_EXEC;
                load0 = 0;
            }
            else
            {
                //others
                if(perm[0] != 'r') *prot &= ~PROT_READ;
                if(perm[1] != 'w') *prot &= ~PROT_WRITE;
                if(perm[2] != 'x') *prot &= ~PROT_EXEC;
            }

            if(end_addr <= end)
            {
                found_all = 1;
                break; //finished
            }
            else
            {
                start_addr = end; //try to find the next load segment
            }
        }
    }
    
    fclose(fp);

    if(!found_all) return XH_ERRNO_SEGVERR;
    
    return 0;
}

int xh_util_get_addr_protect(uintptr_t addr, const char *pathname, unsigned int *prot)
{
    return xh_util_get_mem_protect(addr, sizeof(addr), pathname, prot);
}

int xh_util_set_addr_protect(uintptr_t addr, unsigned int prot)
{
    if(0 != mprotect((void *)PAGE_START(addr), PAGE_COVER(addr), (int)prot))
        return 0 == errno ? XH_ERRNO_UNKNOWN : errno;
    
    return 0;
}

void xh_util_flush_instruction_cache(uintptr_t addr)
{
    __builtin___clear_cache((void *)PAGE_START(addr), (void *)PAGE_END(addr));
}
