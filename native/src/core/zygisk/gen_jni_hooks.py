#!/usr/bin/env python3

primitives = ['jint', 'jboolean', 'jlong']

class JType:
    def __init__(self, cpp, jni):
        self.cpp = cpp
        self.jni = jni


class JArray(JType):
    def __init__(self, type):
        if type.cpp in primitives:
            name = type.cpp + 'Array'
        else:
            name = 'jobjectArray'
        super().__init__(name, '[' + type.jni)


class Argument:
    def __init__(self, name, type, set_arg = False):
        self.name = name
        self.type = type
        self.set_arg = set_arg

    def cpp(self):
        return f'{self.type.cpp} {self.name}'

# Args we don't care, give it an auto generated name
class Anon(Argument):
    cnt = 0
    def __init__(self, type):
        super().__init__(f'_{Anon.cnt}', type)
        Anon.cnt += 1

class Return:
    def __init__(self, value, type):
        self.value = value
        self.type = type

class Method:
    def __init__(self, name, ret, args):
        self.name = name
        self.ret = ret
        self.args = args

    def cpp(self):
        return ', '.join(map(lambda x: x.cpp(), self.args))

    def name_list(self):
        return ', '.join(map(lambda x: x.name, self.args))

    def jni(self):
        args = ''.join(map(lambda x: x.type.jni, self.args))
        return f'({args}){self.ret.type.jni}'

    def body(self, name, i):
        return ''

class JNIHook(Method):
    def __init__(self, ver, ret, args):
        name = f'{self.base_name()}_{ver}'
        super().__init__(name, ret, args)

    def base_name(self):
        return ''

    def orig_method(self, name, i):
        return f'reinterpret_cast<{self.ret.type.cpp}(*)(JNIEnv *env, jclass clazz, {self.cpp()})>(g_hook->{name}_methods[{i}].fnPtr)'

def ind(i):
    return '\n' + '    ' * i

# Common types
jint = JType('jint', 'I')
jintArray = JArray(jint)
jstring = JType('jstring', 'Ljava/lang/String;')
jboolean = JType('jboolean', 'Z')
jlong = JType('jlong', 'J')
void = JType('void', 'V')

class ForkAndSpec(JNIHook):
    def __init__(self, ver, args):
        super().__init__(ver, Return('ctx.pid', jint), args)

    def base_name(self):
        return 'nativeForkAndSpecialize'

    def init_args(self):
        return 'AppSpecializeArgs_v5 args(uid, gid, gids, runtime_flags, rlimits, mount_external, se_info, nice_name, instruction_set, app_data_dir);'

    def body(self, name, i):
        decl = ''
        decl += ind(3) + self.init_args()
        for a in self.args:
            if a.set_arg:
                decl += ind(3) + f'args.{a.name} = &{a.name};'
        decl += ind(3) + 'ZygiskContext ctx(env, &args);'
        decl += ind(3) + f'ctx.{self.base_name()}_pre();'
        decl += ind(3) + self.orig_method(name, i) + '('
        decl += ind(4) + f'env, clazz, {self.name_list()}'
        decl += ind(3) + ');'
        decl += ind(3) + f'ctx.{self.base_name()}_post();'
        return decl

class SpecApp(ForkAndSpec):
    def __init__(self, ver, args):
        super().__init__(ver, args)
        self.ret = Return('', void)

    def base_name(self):
        return 'nativeSpecializeAppProcess'

class ForkServer(ForkAndSpec):
    def base_name(self):
        return 'nativeForkSystemServer'

    def init_args(self):
        return 'ServerSpecializeArgs_v1 args(uid, gid, gids, runtime_flags, permitted_capabilities, effective_capabilities);'

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
fds_to_ignore = Argument('fds_to_ignore', jintArray, True)

# p
is_child_zygote = Argument('is_child_zygote', jboolean, True)

# q_alt
is_top_app = Argument('is_top_app', jboolean, True)

# r
pkg_data_info_list = Argument('pkg_data_info_list', JArray(jstring), True)
whitelisted_data_info_list = Argument('whitelisted_data_info_list', JArray(jstring), True)
mount_data_dirs = Argument('mount_data_dirs', jboolean, True)
mount_storage_dirs = Argument('mount_storage_dirs', jboolean, True)

# u
mount_sysprop_overrides = Argument('mount_sysprop_overrides', jboolean, True)

# server
permitted_capabilities = Argument('permitted_capabilities', jlong)
effective_capabilities = Argument('effective_capabilities', jlong)

# Method definitions
fas_l = ForkAndSpec('l', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, nice_name, fds_to_close, instruction_set, app_data_dir])

fas_o = ForkAndSpec('o', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, nice_name, fds_to_close, fds_to_ignore, instruction_set, app_data_dir])

fas_p = ForkAndSpec('p', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir])

fas_q_alt = ForkAndSpec('q_alt', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir, is_top_app])

fas_r = ForkAndSpec('r', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir, is_top_app,
    pkg_data_info_list, whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs])

fas_u = ForkAndSpec('u', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, fds_to_close, fds_to_ignore, is_child_zygote, instruction_set, app_data_dir, is_top_app,
    pkg_data_info_list, whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs, mount_sysprop_overrides])

fas_samsung_m = ForkAndSpec('samsung_m', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, Anon(jint), Anon(jint), nice_name, fds_to_close, instruction_set, app_data_dir])

fas_samsung_n = ForkAndSpec('samsung_n', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, Anon(jint), Anon(jint), nice_name, fds_to_close, instruction_set, app_data_dir, Anon(jint)])

fas_samsung_o = ForkAndSpec('samsung_o', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, Anon(jint), Anon(jint), nice_name, fds_to_close, fds_to_ignore, instruction_set, app_data_dir])

fas_samsung_p = ForkAndSpec('samsung_p', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, Anon(jint), Anon(jint), nice_name, fds_to_close, fds_to_ignore, is_child_zygote,
    instruction_set, app_data_dir])

spec_q = SpecApp('q', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, is_child_zygote, instruction_set, app_data_dir])

spec_q_alt = SpecApp('q_alt', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info,
    nice_name, is_child_zygote, instruction_set, app_data_dir, is_top_app])

spec_r = SpecApp('r', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info, nice_name,
    is_child_zygote, instruction_set, app_data_dir, is_top_app, pkg_data_info_list,
    whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs])

spec_u = SpecApp('u', [uid, gid, gids, runtime_flags, rlimits, mount_external, se_info, nice_name,
    is_child_zygote, instruction_set, app_data_dir, is_top_app, pkg_data_info_list,
    whitelisted_data_info_list, mount_data_dirs, mount_storage_dirs, mount_sysprop_overrides])

spec_samsung_q = SpecApp('samsung_q', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, Anon(jint), Anon(jint), nice_name, is_child_zygote, instruction_set, app_data_dir])

server_l = ForkServer('l', [uid, gid, gids, runtime_flags, rlimits,
    permitted_capabilities, effective_capabilities])

server_samsung_q = ForkServer('samsung_q', [uid, gid, gids, runtime_flags, Anon(jint), Anon(jint), rlimits,
    permitted_capabilities, effective_capabilities])

def gen_jni_def(name, methods):
    decl = ''
    decl += ind(0) + f'std::array<JNINativeMethod, {len(methods)}> {name}_methods = {{{{'
    for i, m in enumerate(methods):
        decl += ind(1) + '{'
        decl += ind(2) + f'"{m.base_name()}",'
        decl += ind(2) + f'"{m.jni()}",'
        decl += ind(2) + f'(void *) +[] [[clang::no_stack_protector]] (JNIEnv *env, jclass clazz, {m.cpp()}) static -> {m.ret.type.cpp} {{'
        decl += m.body(name, i)
        if m.ret.value:
            decl += ind(3) + f'return {m.ret.value};'
        decl += ind(2) + '}'
        decl += ind(1) + '},'
    decl += ind(0) + '}};'
    decl += ind(0)

    return decl

with open('jni_hooks.hpp', 'w') as f:
    f.write('// Generated by gen_jni_hooks.py\n')

    f.write(gen_jni_def('zygote', [
        fas_l, fas_o, fas_p, fas_q_alt, fas_r, fas_u, fas_samsung_m, fas_samsung_n, fas_samsung_o,
        fas_samsung_p, spec_q, spec_q_alt, spec_r, spec_u, spec_samsung_q, server_l, server_samsung_q]))

    f.write('\n')
