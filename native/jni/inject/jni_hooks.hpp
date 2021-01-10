/*
 * Original code: https://github.com/RikkaApps/Riru/blob/master/riru/src/main/cpp/jni_native_method.cpp
 * The code is modified and sublicensed to GPLv3 for incorporating into Magisk.
 *
 * Copyright (c) 2018-2021, RikkaW
 * Copyright (c) 2021, John 'topjohnwu' Wu
 */

#define ENABLE_LEGACY_DP 0  // Nobody should use outdated developer preview...

// All possible missing arguments
static union {
    struct {
        jintArray fds_to_ignore;
        jboolean is_child_zygote;
        jboolean is_top_app;
        jobjectArray pkg_data_info_list;
        jobjectArray whitelisted_data_info_list;
        jboolean mount_data_dirs;
        jboolean mount_storage_dirs;
    };
    size_t args_buf[8];  // Easy access to wipe all variables at once
};

#define DCL_JNI(ret, name, sig, ...) \
const static char name##_sig[] = sig; \
static ret name(__VA_ARGS__)

// -----------------------------------------------------------------

#define pre_fork() \
    HookContext ctx{}; \
    memset(args_buf, 0, sizeof(args_buf)); \
    nativeForkAndSpecialize_pre(&ctx, env, clazz, uid, gid, gids, runtime_flags, \
    rlimits, mount_external, se_info, nice_name, fds_to_close, fds_to_ignore, is_child_zygote, \
    instruction_set, app_data_dir, is_top_app, pkg_data_info_list, whitelisted_data_info_list, \
    mount_data_dirs, mount_storage_dirs)

#define orig_fork(ver, ...) \
    reinterpret_cast<decltype(&nativeForkAndSpecialize_##ver)> \
    (nativeForkAndSpecialize_orig->fnPtr)(__VA_ARGS__)

#define post_fork() \
    nativeForkAndSpecialize_post(&ctx, env, clazz); \
    return ctx.pid

#define DCL_FORK_AND_SPECIALIZE(ver, sig, ...) \
DCL_JNI(jint, nativeForkAndSpecialize_##ver, sig, __VA_ARGS__)

DCL_FORK_AND_SPECIALIZE(m,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[ILjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jintArray fds_to_close, jstring instruction_set, jstring app_data_dir) {
    pre_fork();
    orig_fork(m, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, nice_name, fds_to_close, instruction_set, app_data_dir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(o,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[ILjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jintArray fds_to_close, jintArray fds_to_ignore, jstring instruction_set, jstring app_data_dir) {
    pre_fork();
    orig_fork(o, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, nice_name, fds_to_close, fds_to_ignore, instruction_set, app_data_dir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(p,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jintArray fds_to_close, jintArray fds_to_ignore, jboolean is_child_zygote,
        jstring instruction_set, jstring app_data_dir) {
    pre_fork();
    orig_fork(p, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(q_alt,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;Z)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jintArray fds_to_close, jintArray fds_to_ignore, jboolean is_child_zygote,
        jstring instruction_set, jstring app_data_dir, jboolean is_top_app) {
    pre_fork();
    orig_fork(q_alt, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir, is_top_app);
    post_fork();
}

#if ENABLE_LEGACY_DP
DCL_FORK_AND_SPECIALIZE(r_dp2,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jintArray fds_to_close, jintArray fds_to_ignore, jboolean is_child_zygote,
        jstring instruction_set, jstring app_data_dir, jboolean is_top_app, jobjectArray pkg_data_info_list) {
    pre_fork();
    orig_fork(r_dp2, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir,
            is_top_app, pkg_data_info_list);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(r_dp3,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;Z)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jintArray fds_to_close, jintArray fds_to_ignore, jboolean is_child_zygote,
        jstring instruction_set, jstring app_data_dir, jboolean is_top_app, jobjectArray pkg_data_info_list,
        jboolean mount_storage_dirs) {
    pre_fork();
    orig_fork(r_dp3, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set,
            app_data_dir, is_top_app, pkg_data_info_list, mount_storage_dirs);
    post_fork();
}
#endif // ENABLE_LEGACY_DP

DCL_FORK_AND_SPECIALIZE(r,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;[Ljava/lang/String;ZZ)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jintArray fds_to_close, jintArray fds_to_ignore, jboolean is_child_zygote,
        jstring instruction_set, jstring app_data_dir, jboolean is_top_app, jobjectArray pkg_data_info_list,
        jobjectArray whitelisted_data_info_list, jboolean mount_data_dirs, jboolean mount_storage_dirs) {
    pre_fork();
    orig_fork(r, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir, is_top_app,
            pkg_data_info_list, whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(samsung_m,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;[ILjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint category, jint accessInfo,
        jstring nice_name, jintArray fds_to_close, jstring instruction_set, jstring app_data_dir) {
    pre_fork();
    orig_fork(samsung_m, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, category, accessInfo, nice_name, fds_to_close, instruction_set, app_data_dir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(samsung_n,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;[ILjava/lang/String;Ljava/lang/String;I)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint category, jint accessInfo,
        jstring nice_name, jintArray fds_to_close, jstring instruction_set, jstring app_data_dir, jint a1) {
    pre_fork();
    orig_fork(samsung_n, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, category, accessInfo, nice_name, fds_to_close, instruction_set, app_data_dir, a1);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(samsung_o,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;[I[ILjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint category, jint accessInfo,
        jstring nice_name, jintArray fds_to_close, jintArray fds_to_ignore, jstring instruction_set,
        jstring app_data_dir) {
    pre_fork();
    orig_fork(samsung_o, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, category, accessInfo, nice_name, fds_to_close, fds_to_ignore,
            instruction_set, app_data_dir);
    post_fork();
}

DCL_FORK_AND_SPECIALIZE(samsung_p,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;[I[IZLjava/lang/String;Ljava/lang/String;)I",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint category, jint accessInfo,
        jstring nice_name, jintArray fds_to_close, jintArray fds_to_ignore, jboolean is_child_zygote,
        jstring instruction_set, jstring app_data_dir) {
    pre_fork();
    orig_fork(samsung_p, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, category, accessInfo, nice_name, fds_to_close, fds_to_ignore, is_child_zygote,
            instruction_set, app_data_dir);
    post_fork();
}

#define DEF_FORK(ver) { \
    "nativeForkAndSpecialize", \
    nativeForkAndSpecialize_##ver##_sig, \
    (void *) &nativeForkAndSpecialize_##ver \
}

// -----------------------------------------------------------------

#define pre_spec() \
    HookContext ctx{}; \
    memset(args_buf, 0, sizeof(args_buf)); \
    nativeSpecializeAppProcess_pre(&ctx, \
    env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info, nice_name, \
    is_child_zygote, instruction_set, app_data_dir, is_top_app, pkg_data_info_list, \
    whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs)

#define orig_spec(ver, ...) \
    reinterpret_cast<decltype(&nativeSpecializeAppProcess_##ver)> \
    (nativeSpecializeAppProcess_orig->fnPtr)(__VA_ARGS__)

#define post_spec() \
    nativeSpecializeAppProcess_post(&ctx, env, clazz)

#define DCL_SPECIALIZE_APP(ver, sig, ...) \
DCL_JNI(void, nativeSpecializeAppProcess_##ver, sig, __VA_ARGS__)

DCL_SPECIALIZE_APP(q,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jboolean is_child_zygote, jstring instruction_set, jstring app_data_dir) {
    pre_spec();
    orig_spec(q, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, is_child_zygote, instruction_set, app_data_dir);
    post_spec();
}

DCL_SPECIALIZE_APP(q_alt,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Z)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jboolean is_child_zygote, jstring instruction_set, jstring app_data_dir,
        jboolean is_top_app) {
    pre_spec();
    orig_spec(q_alt, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, is_child_zygote, instruction_set, app_data_dir, is_top_app);
    post_spec();
}

#if ENABLE_LEGACY_DP
DCL_SPECIALIZE_APP(r_dp2,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jboolean is_child_zygote, jstring instruction_set, jstring app_data_dir,
        jboolean is_top_app, jobjectArray pkg_data_info_list) {
    pre_spec();
    orig_spec(r_dp2, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, is_child_zygote, instruction_set, app_data_dir, is_top_app, pkg_data_info_list);
    post_spec();
}

DCL_SPECIALIZE_APP(r_dp3,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;Z)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jboolean is_child_zygote, jstring instruction_set, jstring app_data_dir,
        jboolean is_top_app, jobjectArray pkg_data_info_list, jboolean mount_storage_dirs) {
    pre_spec();
    orig_spec(r_dp3, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
            nice_name, is_child_zygote, instruction_set, app_data_dir, is_top_app, pkg_data_info_list,
            mount_storage_dirs);
    post_spec();
}
#endif // ENABLE_LEGACY_DP

DCL_SPECIALIZE_APP(r,
        "(II[II[[IILjava/lang/String;Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Z[Ljava/lang/String;[Ljava/lang/String;ZZ)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jstring nice_name,
        jboolean is_child_zygote, jstring instruction_set, jstring app_data_dir,
        jboolean is_top_app, jobjectArray pkg_data_info_list, jobjectArray whitelisted_data_info_list,
        jboolean mount_data_dirs, jboolean mount_storage_dirs) {
    pre_spec();
    orig_spec(r, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external, se_info, nice_name,
            is_child_zygote, instruction_set, app_data_dir, is_top_app, pkg_data_info_list,
            whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs);
    post_spec();
}

DCL_SPECIALIZE_APP(samsung_q,
        "(II[II[[IILjava/lang/String;IILjava/lang/String;ZLjava/lang/String;Ljava/lang/String;)V",
        JNIEnv *env, jclass clazz, jint uid, jint gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jint mount_external, jstring se_info, jint space, jint accessInfo,
        jstring nice_name, jboolean is_child_zygote, jstring instruction_set, jstring app_data_dir) {
    pre_spec();
    orig_spec(samsung_q, env, clazz, uid, gid, gids, runtime_flags, rlimits, mount_external,
            se_info, space, accessInfo, nice_name, is_child_zygote, instruction_set, app_data_dir);
    post_spec();
}

#define DEF_SPEC(ver) { \
    "nativeSpecializeAppProcess", \
    nativeSpecializeAppProcess_##ver##_sig, \
    (void *) &nativeSpecializeAppProcess_##ver \
}

// -----------------------------------------------------------------

#define pre_server() \
    HookContext ctx{}; \
    memset(args_buf, 0, sizeof(args_buf)); \
    nativeForkSystemServer_pre(&ctx, env, clazz, uid, gid, gids, runtime_flags, \
    rlimits, permitted_capabilities, effective_capabilities)

#define orig_server(ver, ...) \
    reinterpret_cast<decltype(&nativeForkSystemServer_##ver)> \
    (nativeForkSystemServer_orig->fnPtr)(__VA_ARGS__)

#define post_server() \
    nativeForkSystemServer_post(&ctx, env, clazz); \
    return ctx.pid

#define DCL_FORK_SERVER(ver, sig, ...) \
DCL_JNI(jint, nativeForkSystemServer_##ver, sig, __VA_ARGS__)

DCL_FORK_SERVER(m, "(II[II[[IJJ)I",
        JNIEnv *env, jclass clazz, uid_t uid, gid_t gid, jintArray gids, jint runtime_flags,
        jobjectArray rlimits, jlong permitted_capabilities, jlong effective_capabilities) {
    pre_server();
    orig_server(m, env, clazz, uid, gid, gids, runtime_flags, rlimits, permitted_capabilities,
            effective_capabilities);
    post_server();
}

DCL_FORK_SERVER(samsung_q, "(II[IIII[[IJJ)I",
        JNIEnv *env, jclass clazz, uid_t uid, gid_t gid, jintArray gids, jint runtime_flags,
        jint space, jint accessInfo, jobjectArray rlimits, jlong permitted_capabilities,
        jlong effective_capabilities) {
    pre_server();
    orig_server(samsung_q, env, clazz, uid, gid, gids, runtime_flags, space, accessInfo, rlimits,
            permitted_capabilities, effective_capabilities);
    post_server();
}

#define DEF_SERVER(ver) { \
    "nativeForkSystemServer", \
    nativeForkSystemServer_##ver##_sig, \
    (void *) &nativeForkSystemServer_##ver \
}

namespace {

const JNINativeMethod nativeForkAndSpecialize_methods[] = {
    DEF_FORK(m), DEF_FORK(o), DEF_FORK(p),
    DEF_FORK(q_alt), DEF_FORK(r),
    DEF_FORK(samsung_m), DEF_FORK(samsung_n),
    DEF_FORK(samsung_o), DEF_FORK(samsung_p),
#if ENABLE_LEGACY_DP
    DEF_FORK(r_dp2), DEF_FORK(r_dp3)
#endif
};
const int nativeForkAndSpecialize_methods_num = std::size(nativeForkAndSpecialize_methods);

const JNINativeMethod nativeSpecializeAppProcess_methods[] = {
    DEF_SPEC(q), DEF_SPEC(q_alt),
    DEF_SPEC(r), DEF_SPEC(samsung_q),
#if ENABLE_LEGACY_DP
    DEF_SPEC(r_dp2), DEF_SPEC(r_dp3)
#endif
};
const int nativeSpecializeAppProcess_methods_num = std::size(
    nativeSpecializeAppProcess_methods);

const JNINativeMethod nativeForkSystemServer_methods[] = {
    DEF_SERVER(m), DEF_SERVER(samsung_q)
};
const int nativeForkSystemServer_methods_num = std::size(nativeForkSystemServer_methods);

}
