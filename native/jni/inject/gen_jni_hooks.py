#!/usr/bin/env python3

primitives = ['jint', 'jboolean', 'jlong']

class JType:
    def __init__(self, name, sig) -> None:
        self.name = name
        self.sig = sig


class JArray(JType):
    def __init__(self, type) -> None:
        if type.name in primitives:
            name = type.name + 'Array'
        else:
            name = 'jobjectArray'
        super().__init__(name, '[' + type.sig)


class Argument:
    def __init__(self, name, type, set_arg = False) -> None:
        self.name = name
        self.type = type
        self.set_arg = set_arg

    def cpp(self):
        return f'{self.type.name} {self.name}'


class Method:
    def __init__(self, name, args) -> None:
        self.name = name
        self.args = args

    def cpp(self):
        return ', '.join(map(lambda a: a.cpp(), self.args))

    def name_list(self):
        return ', '.join(map(lambda a: a.name, self.args))

    def jni(self):
        return ''.join(map(lambda a: a.type.sig, self.args))


# Common types
jint = JType('jint', 'I')
jintArray = JArray(jint)
jstring = JType('jstring', 'Ljava/lang/String;')
jboolean = JType('jboolean', 'Z')
jlong = JType('jlong', 'J')

# Common args
uid = Argument('uid', jint)
gid = Argument('gid', jint)
gids = Argument('gids', jintArray)
runtime_flags = Argument('runtime_flags', jint)
rlimits = Argument('rlimits', JArray(jintArray))
mount_external = Argument('mount_external', jint)
se_info = Argument('se_info', jstring)
nice_name = Argument('nice_name', jstring)
fds_to_close = Argument('fds_to_close', jintArray)
instruction_set = Argument('instruction_set', jstring)
app_data_dir = Argument('app_data_dir', jstring)

# o
fds_to_ignore = Argument('fds_to_ignore', jintArray)

# p
is_child_zygote = Argument('is_child_zygote', jboolean, True)

# q_alt
is_top_app = Argument('is_top_app', jboolean, True)

# r
pkg_data_info_list = Argument('pkg_data_info_list', JArray(jstring), True)
whitelisted_data_info_list = Argument('whitelisted_data_info_list', JArray(jstring), True)
mount_data_dirs = Argument('mount_data_dirs', jboolean, True)
mount_storage_dirs = Argument('mount_storage_dirs', jboolean, True)

# samsung (non-standard arguments)
i1 = Argument('i1', jint)
i2 = Argument('i2', jint)
i3 = Argument('i3', jint)

# server
permitted_capabilities = Argument('permitted_capabilities', jlong)
effective_capabilities = Argument('effective_capabilities', jlong)

# Method definitions
fork_l = Method('l', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, nice_name, fds_to_close, instruction_set, app_data_dir])

fork_o = Method('o', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, nice_name, fds_to_close, fds_to_ignore, instruction_set, app_data_dir])

fork_p = Method('p', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir])

fork_q_alt = Method('q_alt', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir, is_top_app])

fork_r = Method('r', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir, is_top_app,
    pkg_data_info_list, whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs])

fork_samsung_m = Method('samsung_m', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, i1, i2, nice_name, fds_to_close, instruction_set, app_data_dir])

fork_samsung_n = Method('samsung_n', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, i1, i2, nice_name, fds_to_close, instruction_set, app_data_dir, i3])

fork_samsung_o = Method('samsung_o', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, i1, i2, nice_name, fds_to_close, fds_to_ignore, instruction_set, app_data_dir])

fork_samsung_p = Method('samsung_p', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, i1, i2, nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir])

spec_q = Method('q', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, is_child_zygote, instruction_set, app_data_dir])

spec_q_alt = Method('q_alt', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, is_child_zygote, instruction_set, app_data_dir, is_top_app])

spec_r = Method('r', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info, nice_name,
    is_child_zygote, instruction_set, app_data_dir, is_top_app, pkg_data_info_list,
    whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs])

spec_samsung_q = Method('samsung_q', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, i1, i2, nice_name, is_child_zygote, instruction_set, app_data_dir])

server_m = Method('m', [uid, gid, gids, runtime_flags, rlimits,
    permitted_capabilities, effective_capabilities])

server_samsung_q = Method('samsung_q', [uid, gid, gids, runtime_flags, i1, i2, rlimits,
    permitted_capabilities, effective_capabilities])


def ind(i):
    return '\n' + '    ' * i

def gen_definitions(methods, base_name):
    decl = ''
    if base_name != 'nativeSpecializeAppProcess':
        ret_stat = ind(1) + 'return ctx.pid;'
        cpp_ret = 'jint'
        jni_ret = 'I'
    else:
        ret_stat = ''
        cpp_ret = 'void'
        jni_ret = 'V'
    for m in methods:
        func_name = f'{base_name}_{m.name}'
        decl += ind(0) + f'{cpp_ret} {func_name}(JNIEnv *env, jclass clazz, {m.cpp()}) {{'
        decl += ind(1) + 'HookContext ctx{};'
        if base_name == 'nativeForkSystemServer':
            decl += ind(1) + 'ForkSystemServerArgs args(uid, gid, gids, runtime_flags, permitted_capabilities, effective_capabilities);'
        else:
            decl += ind(1) + 'SpecializeAppProcessArgs args(uid, gid, gids, runtime_flags, mount_external, se_info, nice_name, instruction_set, app_data_dir);'
        for a in m.args:
            if a.set_arg:
                decl += ind(1) + f'args.{a.name} = &{a.name};'
        decl += ind(1) + 'ctx.raw_args = &args;'
        decl += ind(1) + f'{base_name}_pre(&ctx, env, clazz);'
        decl += ind(1) + f'reinterpret_cast<decltype(&{func_name})>({base_name}_orig)('
        decl += ind(2) + f'env, clazz, {m.name_list()}'
        decl += ind(1) + ');'
        decl += ind(1) + f'{base_name}_post(&ctx, env, clazz);'
        decl += ret_stat
        decl += ind(0) + '}'

    decl += ind(0) + f'const JNINativeMethod {base_name}_methods[] = {{'
    for m in methods:
        decl += ind(1) + '{'
        decl += ind(2) + f'"{base_name}",'
        decl += ind(2) + f'"({m.jni()}){jni_ret}",'
        decl += ind(2) + f'(void *) &{base_name}_{m.name}'
        decl += ind(1) + '},'
    decl += ind(0) + '};'
    decl += ind(0) + f'constexpr int {base_name}_methods_num = std::size({base_name}_methods);'
    decl += ind(0)
    return decl

def gen_fork():
    methods = [fork_l, fork_o, fork_p, fork_q_alt, fork_r, fork_samsung_m, fork_samsung_n, fork_samsung_o, fork_samsung_p]
    return gen_definitions(methods, 'nativeForkAndSpecialize')

def gen_spec():
    methods = [spec_q, spec_q_alt, spec_r, spec_samsung_q]
    return gen_definitions(methods, 'nativeSpecializeAppProcess')

def gen_server():
    methods = [server_m, server_samsung_q]
    return gen_definitions(methods, 'nativeForkSystemServer')

with open('jni_hooks.hpp', 'w') as f:
    f.write('// Generated by gen_jni_hooks.py\n')
    f.write(gen_fork())
    f.write(gen_spec())
    f.write(gen_server())
