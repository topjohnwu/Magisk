/*
 * Original code: https://github.com/RikkaApps/Riru/blob/master/riru/src/main/cpp/jni_native_method.cpp
 * The code is modified and sublicensed to GPLv3 for incorporating into Magisk.
 *
 * Copyright (c) 2018-2021, RikkaW
 * Copyright (c) 2021, John 'topjohnwu' Wu
 */

#include <jni.h>

#include <utils.hpp>

#include "inject.hpp"

#define ENABLE_LEGACY_DP 0  // Nobody should use outdated developer preview...

static void nativeForkAndSpecialize_pre(
        JNIEnv *env, jclass clazz, jint &uid, jint &gid, jintArray &gids, jint &runtime_flags,
        jobjectArray &rlimits, jint &mount_external, jstring &se_info, jstring &se_name,
        jintArray &fdsToClose, jintArray &fdsToIgnore, jboolean &is_child_zygote,
        jstring &instructionSet, jstring &appDataDir, jboolean &isTopApp, jobjectArray &pkgDataInfoList,
        jobjectArray &whitelistedDataInfoList, jboolean &bindMountAppDataDirs, jboolean &bindMountAppStorageDirs) {
    LOGD("hook: %s\n", __FUNCTION__);
}

static void nativeForkAndSpecialize_post(JNIEnv *env, jclass clazz, jint uid, jint pid) {
    LOGD("hook: %s\n", __FUNCTION__);
    // Demonstrate self unload in child process
    if (pid == 0)
        self_unload();
}

// -----------------------------------------------------------------

static void nativeSpecializeAppProcess_pre(
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtimeFlags,
        jobjectArray rlimits, jint mountExternal, jstring seInfo, jstring niceName,
        jboolean startChildZygote, jstring instructionSet, jstring appDataDir,
        jboolean &isTopApp, jobjectArray &pkgDataInfoList, jobjectArray &whitelistedDataInfoList,
        jboolean &bindMountAppDataDirs, jboolean &bindMountAppStorageDirs) {
    LOGD("hook: %s\n", __FUNCTION__);
}

static void nativeSpecializeAppProcess_post(JNIEnv *env, jclass clazz) {
    LOGD("hook: %s\n", __FUNCTION__);
}

// -----------------------------------------------------------------

static void nativeForkSystemServer_pre(
        JNIEnv *env, jclass clazz, uid_t &uid, gid_t &gid, jintArray &gids, jint &debug_flags,
        jobjectArray &rlimits, jlong &permittedCapabilities, jlong &effectiveCapabilities) {
    LOGD("hook: %s\n", __FUNCTION__);
}

static void nativeForkSystemServer_post(JNIEnv *env, jclass clazz, jint res) {
    LOGD("hook: %s\n", __FUNCTION__);
}

// -----------------------------------------------------------------

// All possible missing arguments
static union {
    struct {
        jintArray fdsToIgnore;
        jboolean is_child_zygote;
        jboolean isTopApp;
        jobjectArray pkgDataInfoList;
        jobjectArray whitelistedDataInfoList;
        jboolean bindMountAppDataDirs;
        jboolean bindMountAppStorageDirs;
    };
    size_t missing_arg_buf[8];  // Easy access to wipe all variables at once
};

#define DCL_JNI(ret, name, sig, ...) \
const static char name##_sig[] = sig; \
static ret name(__VA_ARGS__)

// -----------------------------------------------------------------

#define pre_fork() \
    memset(missing_arg_buf, 0, sizeof(missing_arg_buf)); \
    nativeForkAndSpecialize_pre( env, clazz, uid, gid, gids, runtime_flags, \
    rlimits, mount_external, se_info, se_name, fdsToClose, fdsToIgnore, is_child_zygote, \
    instructionSet, appDataDir, isTopApp, pkgDataInfoList, whitelistedDataInfoList, \
    bindMountAppDataDirs, bindMountAppStorageDirs) \

#define orig_fork(ver, ...) \
    jint pid = reinterpret_cast<decltype(&nativeForkAndSpecialize_##ver)> \
    (JNI::Zygote::nativeForkAndSpecialize_orig->fnPtr)(__VA_ARGS__)

#define post_fork() \
    nativeForkAndSpecialize_post(env, clazz, uid, pid); \
    return pid

#define DCL_FORK_AND_SPECIALIZE(ver, sig, ...) \
DCL_JNI(jint, nativeForkAndSpecialize_##ver, sig, __VA_ARGS__)

DCL_FORK_AND_SPECIALIZE(m,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[ILjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring se_name,
        jintArray fdsToClose, jstring instructionSet, jstring appDataDir) {
    pre_fork();
    orig_fork(m, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, se_name, fdsToClose, instructionSet, appDataDir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(o,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[ILjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring se_name,
        jintArray fdsToClose, jintArray fdsToIgnore, jstring instructionSet, jstring appDataDir) {
    pre_fork();
    orig_fork(o, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, se_name, fdsToClose, fdsToIgnore, instructionSet, appDataDir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(p,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring se_name,
        jintArray fdsToClose, jintArray fdsToIgnore, jboolean is_child_zygote,
        jstring instructionSet, jstring appDataDir) {
    pre_fork();
    orig_fork(p, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            se_name, fdsToClose, fdsToIgnore, is_child_zygote, instructionSet, appDataDir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(q_alt,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;Z)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring se_name,
        jintArray fdsToClose, jintArray fdsToIgnore, jboolean is_child_zygote,
        jstring instructionSet, jstring appDataDir, jboolean isTopApp) {
    pre_fork();
    orig_fork(q_alt, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            se_name, fdsToClose, fdsToIgnore, is_child_zygote, instructionSet, appDataDir, isTopApp);
    post_fork();
}

#if ENABLE_LEGACY_DP
DCL_FORK_AND_SPECIALIZE(r_dp2,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring se_name,
        jintArray fdsToClose, jintArray fdsToIgnore, jboolean is_child_zygote,
        jstring instructionSet, jstring appDataDir, jboolean isTopApp, jobjectArray pkgDataInfoList) {
    pre_fork();
    orig_fork(r_dp2, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            se_name, fdsToClose, fdsToIgnore, is_child_zygote, instructionSet, appDataDir,
            isTopApp, pkgDataInfoList);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(r_dp3,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;Z)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring se_name,
        jintArray fdsToClose, jintArray fdsToIgnore, jboolean is_child_zygote,
        jstring instructionSet, jstring appDataDir, jboolean isTopApp, jobjectArray pkgDataInfoList,
        jboolean bindMountAppStorageDirs) {
    pre_fork();
    orig_fork(r_dp3, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, se_name, fdsToClose, fdsToIgnore, is_child_zygote, instructionSet,
            appDataDir, isTopApp, pkgDataInfoList, bindMountAppStorageDirs);
    post_fork();
}
#endif // ENABLE_LEGACY_DP

DCL_FORK_AND_SPECIALIZE(r,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;[Ljava/lang/String;ZZ)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring se_name,
        jintArray fdsToClose, jintArray fdsToIgnore, jboolean is_child_zygote,
        jstring instructionSet, jstring appDataDir, jboolean isTopApp, jobjectArray pkgDataInfoList,
        jobjectArray whitelistedDataInfoList, jboolean bindMountAppDataDirs, jboolean bindMountAppStorageDirs) {
    pre_fork();
    orig_fork(r, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            se_name, fdsToClose, fdsToIgnore, is_child_zygote, instructionSet, appDataDir, isTopApp,
            pkgDataInfoList, whitelistedDataInfoList, bindMountAppDataDirs, bindMountAppStorageDirs);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(samsung_m,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;[ILjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint category, jint accessInfo,
        jstring se_name, jintArray fdsToClose, jstring instructionSet, jstring appDataDir) {
    pre_fork();
    orig_fork(samsung_m, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, category, accessInfo, se_name, fdsToClose, instructionSet, appDataDir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(samsung_n,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;[ILjava/lang/String;Ljava/lang/String;I)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint category, jint accessInfo,
        jstring se_name, jintArray fdsToClose, jstring instructionSet, jstring appDataDir, jint a1) {
    pre_fork();
    orig_fork(samsung_n, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, category, accessInfo, se_name, fdsToClose, instructionSet, appDataDir, a1);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(samsung_o,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;[I[ILjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint category, jint accessInfo,
        jstring se_name, jintArray fdsToClose, jintArray fdsToIgnore, jstring instructionSet,
        jstring appDataDir) {
    pre_fork();
    orig_fork(samsung_o, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, category, accessInfo, se_name, fdsToClose, fdsToIgnore,
            instructionSet, appDataDir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(samsung_p,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint category, jint accessInfo,
        jstring se_name, jintArray fdsToClose, jintArray fdsToIgnore, jboolean is_child_zygote,
        jstring instructionSet, jstring appDataDir) {
    pre_fork();
    orig_fork(samsung_p, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, category, accessInfo, se_name, fdsToClose, fdsToIgnore, is_child_zygote,
            instructionSet, appDataDir);
    post_fork();
}

#define DCL_FORK(ver) { \
    "nativeForkAndSpecialize", \
    nativeForkAndSpecialize_##ver##_sig, \
    (void *) &nativeForkAndSpecialize_##ver \
}

// -----------------------------------------------------------------

#define pre_spec() \
    memset(missing_arg_buf, 0, sizeof(missing_arg_buf)); \
    nativeSpecializeAppProcess_pre( \
    env, clazz, uid, gid, gids, runtimeFlags, rlimits, mountExternal, seInfo, niceName, \
    startChildZygote, instructionSet, appDataDir, isTopApp, pkgDataInfoList, \
    whitelistedDataInfoList, bindMountAppDataDirs, bindMountAppStorageDirs)

#define orig_spec(ver, ...) \
    reinterpret_cast<decltype(&nativeSpecializeAppProcess_##ver)> \
    (JNI::Zygote::nativeSpecializeAppProcess_orig->fnPtr)(__VA_ARGS__)

#define post_spec() nativeSpecializeAppProcess_post(env, clazz)

#define DCL_SPECIALIZE_APP(ver, sig, ...) \
DCL_JNI(void, nativeSpecializeAppProcess_##ver, sig, __VA_ARGS__)

DCL_SPECIALIZE_APP(q,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtimeFlags,
        jobjectArray rlimits, jint mountExternal, jstring seInfo, jstring niceName,
        jboolean startChildZygote, jstring instructionSet, jstring appDataDir) {
    pre_spec();
    orig_spec(q, env, clazz, uid, gid, gids, runtimeFlags, rlimits, mountExternal, seInfo,
            niceName, startChildZygote, instructionSet, appDataDir);
    post_spec();
}

DCL_SPECIALIZE_APP(q_alt,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Z)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtimeFlags,
        jobjectArray rlimits, jint mountExternal, jstring seInfo, jstring niceName,
        jboolean startChildZygote, jstring instructionSet, jstring appDataDir,
        jboolean isTopApp) {
    pre_spec();
    orig_spec(q_alt, env, clazz, uid, gid, gids, runtimeFlags, rlimits, mountExternal, seInfo,
            niceName, startChildZygote, instructionSet, appDataDir, isTopApp);
    post_spec();
}

#if ENABLE_LEGACY_DP
DCL_SPECIALIZE_APP(r_dp2,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtimeFlags,
        jobjectArray rlimits, jint mountExternal, jstring seInfo, jstring niceName,
        jboolean startChildZygote, jstring instructionSet, jstring appDataDir,
        jboolean isTopApp, jobjectArray pkgDataInfoList) {
    pre_spec();
    orig_spec(r_dp2, env, clazz, uid, gid, gids, runtimeFlags, rlimits, mountExternal, seInfo,
            niceName, startChildZygote, instructionSet, appDataDir, isTopApp, pkgDataInfoList);
    post_spec();
}

DCL_SPECIALIZE_APP(r_dp3,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;Z)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtimeFlags,
        jobjectArray rlimits, jint mountExternal, jstring seInfo, jstring niceName,
        jboolean startChildZygote, jstring instructionSet, jstring appDataDir,
        jboolean isTopApp, jobjectArray pkgDataInfoList, jboolean bindMountAppStorageDirs) {
    pre_spec();
    orig_spec(r_dp3, env, clazz, uid, gid, gids, runtimeFlags, rlimits, mountExternal, seInfo,
            niceName, startChildZygote, instructionSet, appDataDir, isTopApp, pkgDataInfoList,
            bindMountAppStorageDirs);
    post_spec();
}
#endif // ENABLE_LEGACY_DP

DCL_SPECIALIZE_APP(r,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;[Ljava/lang/String;ZZ)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtimeFlags,
        jobjectArray rlimits, jint mountExternal, jstring seInfo, jstring niceName,
        jboolean startChildZygote, jstring instructionSet, jstring appDataDir,
        jboolean isTopApp, jobjectArray pkgDataInfoList, jobjectArray whitelistedDataInfoList,
        jboolean bindMountAppDataDirs, jboolean bindMountAppStorageDirs) {
    pre_spec();
    orig_spec(r, env, clazz, uid, gid, gids, runtimeFlags, rlimits, mountExternal, seInfo, niceName,
            startChildZygote, instructionSet, appDataDir, isTopApp, pkgDataInfoList,
            whitelistedDataInfoList, bindMountAppDataDirs, bindMountAppStorageDirs);
    post_spec();
}

DCL_SPECIALIZE_APP(samsung_q,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;ZLjava/lang/String;Ljava/lang/String;)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtimeFlags,
        jobjectArray rlimits, jint mountExternal, jstring seInfo, jint space, jint accessInfo,
        jstring niceName, jboolean startChildZygote, jstring instructionSet, jstring appDataDir) {
    pre_spec();
    orig_spec(samsung_q, env, clazz, uid, gid, gids, runtimeFlags, rlimits, mountExternal,
            seInfo, space, accessInfo, niceName, startChildZygote, instructionSet, appDataDir);
    post_spec();
}

#define DCL_SPEC(ver) { \
    "nativeSpecializeAppProcess", \
    nativeSpecializeAppProcess_##ver##_sig, \
    (void *) &nativeSpecializeAppProcess_##ver \
}

// -----------------------------------------------------------------

#define pre_server() \
    memset(missing_arg_buf, 0, sizeof(missing_arg_buf)); \
    nativeForkSystemServer_pre(env, clazz, uid, gid, gids, runtimeFlags, \
    rlimits, permittedCapabilities, effectiveCapabilities)

#define orig_server(ver, ...) \
    jint pid = reinterpret_cast<decltype(&nativeForkSystemServer_##ver)> \
    (JNI::Zygote::nativeForkSystemServer_orig->fnPtr)(__VA_ARGS__)

#define post_server() \
    nativeForkSystemServer_post(env, clazz, pid); \
    return pid

#define DCL_FORK_SERVER(ver, sig, ...) \
DCL_JNI(jint, nativeForkSystemServer_##ver, sig, __VA_ARGS__)

DCL_FORK_SERVER(m, "(II[II[[IJJ)I",
        JNIEnv *env, jclass clazz, uid_t uid, gid_t gid, jintArray gids, jint runtimeFlags,
        jobjectArray rlimits, jlong permittedCapabilities, jlong effectiveCapabilities) {
    pre_server();
    orig_server(m, env, clazz, uid, gid, gids, runtimeFlags, rlimits, permittedCapabilities,
            effectiveCapabilities);
    post_server();
}

DCL_FORK_SERVER(samsung_q, "(II[IIII[[IJJ)I",
        JNIEnv *env, jclass clazz, uid_t uid, gid_t gid, jintArray gids, jint runtimeFlags,
        jint space, jint accessInfo, jobjectArray rlimits, jlong permittedCapabilities,
        jlong effectiveCapabilities) {
    pre_server();
    orig_server(samsung_q, env, clazz, uid, gid, gids, runtimeFlags, space, accessInfo, rlimits,
            permittedCapabilities, effectiveCapabilities);
    post_server();
}

#define DCL_SERVER(ver) { \
    "nativeForkSystemServer", \
    nativeForkSystemServer_##ver##_sig, \
    (void *) &nativeForkSystemServer_##ver \
}

/*
 * On Android 9+, in very rare cases, SystemProperties.set("sys.user." + userId + ".ce_available", "true")
 * will throw an exception (no idea if this is caused by hooking) and user data will be wiped.
 * Hook it and clear the exception to prevent this problem from happening.
 *
 * https://cs.android.com/android/platform/superproject/+/android-9.0.0_r34:frameworks/base/services/core/java/com/android/server/pm/UserDataPreparer.java;l=107;bpv=0;bpt=0
 */
static void SystemProperties_set(JNIEnv *env, jobject clazz, jstring keyJ, jstring valJ) {
    const char *key = env->GetStringUTFChars(keyJ, JNI_FALSE);
    char user[16];
    bool no_throw = sscanf(key, "sys.user.%[^.].ce_available", user) == 1;
    env->ReleaseStringUTFChars(keyJ, key);

    reinterpret_cast<decltype(&SystemProperties_set)>
    (JNI::SystemProperties::native_set_orig->fnPtr)(env, clazz, keyJ, valJ);

    jthrowable exception = env->ExceptionOccurred();
    if (exception && no_throw) {
        LOGW("prevented data destroy");

        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

namespace JNI {

    namespace Zygote {
        JNINativeMethod *nativeForkAndSpecialize_orig = nullptr;
        JNINativeMethod *nativeSpecializeAppProcess_orig = nullptr;
        JNINativeMethod *nativeForkSystemServer_orig = nullptr;

        const JNINativeMethod nativeForkAndSpecialize_methods[] = {
            DCL_FORK(m), DCL_FORK(o), DCL_FORK(p),
            DCL_FORK(q_alt), DCL_FORK(r),
            DCL_FORK(samsung_m), DCL_FORK(samsung_n),
            DCL_FORK(samsung_o), DCL_FORK(samsung_p),
#if ENABLE_LEGACY_DP
            DCL_FORK(r_dp2), DCL_FORK(r_dp3)
#endif
        };
        const int nativeForkAndSpecialize_methods_num = std::size(nativeForkAndSpecialize_methods);

        const JNINativeMethod nativeSpecializeAppProcess_methods[] = {
            DCL_SPEC(q), DCL_SPEC(q_alt),
            DCL_SPEC(r), DCL_SPEC(samsung_q),
#if ENABLE_LEGACY_DP
            DCL_SPEC(r_dp2), DCL_SPEC(r_dp3)
#endif
        };
        const int nativeSpecializeAppProcess_methods_num = std::size(nativeSpecializeAppProcess_methods);

        const JNINativeMethod nativeForkSystemServer_methods[] = {
            DCL_SERVER(m), DCL_SERVER(samsung_q)
        };
        const int nativeForkSystemServer_methods_num = std::size(nativeForkSystemServer_methods);
    }

    namespace SystemProperties {
        JNINativeMethod *native_set_orig = nullptr;

        const JNINativeMethod native_set_methods[] = {{
            "native_set",
            "(Ljava/lang/String;Ljava/lang/String;)V",
            (void *) &SystemProperties_set
        }};
    }
}
