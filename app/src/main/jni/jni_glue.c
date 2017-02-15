//
// Java entry point
//

#include <jni.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "zipadjust.h"

JNIEXPORT jbyteArray JNICALL
Java_com_topjohnwu_magisk_utils_ZipUtils_zipAdjust___3BI(JNIEnv *env, jclass type,
                                                                   jbyteArray jbytes, jint size) {
    fin = (*env)->GetPrimitiveArrayCritical(env, jbytes, NULL);
    insize = (size_t) size;

    zipadjust(0);

    (*env)->ReleasePrimitiveArrayCritical(env, jbytes, fin, 0);

    jbyteArray ret = (*env)->NewByteArray(env, outsize);
    (*env)->SetByteArrayRegion(env, ret, 0, outsize, (const jbyte*) fout);
    free(fout);

    return ret;
}

JNIEXPORT void JNICALL
Java_com_topjohnwu_magisk_utils_ZipUtils_zipAdjust__Ljava_lang_String_2(JNIEnv *env, jclass type, jstring name) {
    const char *filename = (*env)->GetStringUTFChars(env, name, NULL);
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
        return;

    // Load the file to memory
    insize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    fin = malloc(insize);
    read(fd, fin, insize);

    zipadjust(0);

    close(fd);

    // Open file for output
    fd = open(filename, O_WRONLY | O_TRUNC);
    if (fd < 0)
        return;

    (*env)->ReleaseStringUTFChars(env, name, filename);

    // Write back to file
    lseek(fd, 0, SEEK_SET);
    write(fd, fout, outsize);

    close(fd);
    free(fin);
    free(fout);

}
