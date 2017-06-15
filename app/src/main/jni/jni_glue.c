//
// Java entry point
//

#include <jni.h>
#include "zipadjust.h"

JNIEXPORT void JNICALL
Java_com_topjohnwu_magisk_utils_ZipUtils_zipAdjust(JNIEnv *env, jclass type, jstring filenameIn_,
                                                    jstring filenameOut_) {
    const char *filenameIn = (*env)->GetStringUTFChars(env, filenameIn_, 0);
    const char *filenameOut = (*env)->GetStringUTFChars(env, filenameOut_, 0);

    // TODO
    zipadjust(filenameIn, filenameOut, 0);

    (*env)->ReleaseStringUTFChars(env, filenameIn_, filenameIn);
    (*env)->ReleaseStringUTFChars(env, filenameOut_, filenameOut);
}