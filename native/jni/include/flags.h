#pragma once

/* Include this header anywhere you access MAGISK_DEBUG, MAGISK_VERSION, MAGISK_VER_CODE.
 *
 * This file is only for more precise incremental builds. We can make sure code that uses
 * external flags are re-compiled by updating the timestamp of this file
 * */

#ifndef MAGISK_VERSION
#define MAGISK_VERSION 99.99
#endif

#ifndef MAGISK_VER_CODE
#define MAGISK_VER_CODE 99999
#endif
