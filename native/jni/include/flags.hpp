#pragma once

/* Include this header anywhere accessing MAGISK_DEBUG, MAGISK_VERSION, MAGISK_VER_CODE.
 *
 * This file is for precise incremental builds. We can make sure code that uses
 * external flags are re-compiled by updating the timestamp of this file
 * */

#define quote(s) #s
#define str(s) quote(s)

extern const char *MAGISK_VERSION;
extern const int MAGISK_VER_CODE;
extern const char *MAGISK_FULL_VER;

#ifdef __MDBG
#define MAGISK_DEBUG
#endif
