#pragma once

/* Include this header anywhere accessing MAGISK_DEBUG, MAGISK_VERSION, MAGISK_VER_CODE.
 *
 * This file is for precise incremental builds. We can make sure code that uses
 * external flags are re-compiled by updating the timestamp of this file
 * */

#define quote(s) #s
#define str(s) quote(s)

#define MAGISK_VERSION  str(__MVSTR)
#define MAGISK_VER_CODE __MCODE

#define SHOW_VER(name) str(name) " v" MAGISK_VERSION "(" str(MAGISK_VER_CODE) ")"
#define FULL_VER(name) SHOW_VER(name) " (by topjohnwu)"

#ifdef __MDBG
#define MAGISK_DEBUG
#endif
