/* This file is here because is uses some same macros in magisk.h
 * So we have to remove them from su.h.
 * However, if we want to build our own binary, we still have to define them
 */


#ifndef _INDEP_BIN_H_
#define _INDEP_BIN_H_

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "su"

// fallback to using /system/bin/log.
// can't use liblog.so because this is a static binary.
#ifndef LOGE
#define LOGE exec_loge
#endif
#ifndef LOGD
#define LOGD exec_logd
#endif
#ifndef LOGW
#define LOGW exec_logw
#endif

#include <errno.h>
#include <string.h>
#define PLOGE(fmt,args...) LOGE(fmt " failed with %d: %s", ##args, errno, strerror(errno))
#define PLOGEV(fmt,err,args...) LOGE(fmt " failed with %d: %s", ##args, err, strerror(err))

int su_main(int argc, char *argv[]);

#endif