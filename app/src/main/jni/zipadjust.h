#ifndef MAGISKMANAGER_ZIPADJUST_H_H
#define MAGISKMANAGER_ZIPADJUST_H_H

#include <android/log.h>

int zipadjust(int decompress);

extern size_t insize, outsize, alloc;
extern unsigned char *fin, *fout;

#define  LOG_TAG    "zipadjust"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#endif //MAGISKMANAGER_ZIPADJUST_H_H
