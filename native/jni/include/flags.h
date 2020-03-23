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
#define MAGISK_FULL_VER MAGISK_VERSION "(" str(MAGISK_VER_CODE) ")"

#define NAME_WITH_VER(name) str(name) " " MAGISK_FULL_VER

#ifdef __MDBG
#define MAGISK_DEBUG
#endif
