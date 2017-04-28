/* magisk.h - Let MagiskBoot use the same error handling API as main magisk program
 */

#ifndef _MAGISK_H_
#define _MAGISK_H_

#define LOGE(err, ...) { fprintf(stderr, __VA_ARGS__); exit(err); }
#define PLOGE(fmt, args...) { fprintf(stderr, fmt " failed with %d: %s\n\n", ##args, errno, strerror(errno)); exit(1); }

#endif
