//
// Java entry point
//

#include <jni.h>
#include <stdlib.h>
#include "zipadjust.h"

JNIEXPORT jbyteArray JNICALL
Java_com_topjohnwu_magisk_utils_ZipUtils_zipAdjust(JNIEnv *env, jclass type, jbyteArray jbytes, jint size) {
    fin = (*env)->GetPrimitiveArrayCritical(env, jbytes, NULL);
    insize = (size_t) size;

    zipadjust(0);

    (*env)->ReleasePrimitiveArrayCritical(env, jbytes, fin, 0);

    jbyteArray ret = (*env)->NewByteArray(env, outsize);
    (*env)->SetByteArrayRegion(env, ret, 0, outsize, (const jbyte*) fout);
    free(fout);

    return ret;
}
