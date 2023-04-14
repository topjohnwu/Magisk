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

    def body(self):
        return ''

class JNIHook(Method):
    def __init__(self, ver, ret, args):
        name = f'{self.base_name()}_{ver}'
        super().__init__(name, ret, args)

    def base_name(self):
        return ''

    def orig_method(self):
        return f'reinterpret_cast<decltype(&{self.name})>({self.base_name()}_orig)'

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
        return 'AppSpecializeArgs_v3 args(uid, gid, gids, runtime_flags, rlimits, mount_external, se_info, nice_name, instruction_set, app_data_dir);'

    def body(self):
        decl = ''
        decl += ind(1) + self.init_args()
        for a in self.args:
            if a.set_arg:
                decl += ind(1) + f'args.{a.name} = &{a.name};'
        decl += ind(1) + 'HookContext ctx(env, &args);'
        decl += ind(1) + f'ctx.{self.base_name()}_pre();'
        decl += ind(1) + self.orig_method() + '('
        decl += ind(2) + f'env, clazz, {self.name_list()}'
        decl += ind(1) + ');'
        decl += ind(1) + f'ctx.{self.base_name()}_post();'
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

spec_samsung_q = SpecApp('samsung_q', [uid, gid, gids, runtime_flags, rlimits, mount_external,
    se_info, Anon(jint), Anon(jint), nice_name, is_child_zygote, instruction_set, app_data_dir])

server_l = ForkServer('l', [uid, gid, gids, runtime_flags, rlimits,
    permitted_capabilities, effective_capabilities])

server_samsung_q = ForkServer('samsung_q', [uid, gid, gids, runtime_flags, Anon(jint), Anon(jint), rlimits,
    permitted_capabilities, effective_capabilities])

hook_map = {}

def gen_jni_def(clz, methods):
    if clz not in hook_map:
        hook_map[clz] = []

    decl = ''
    for m in methods:
        decl += ind(0) + f'[[clang::no_stack_protector]] {m.ret.type.cpp} {m.name}(JNIEnv *env, jclass clazz, {m.cpp()}) {{'
        decl += m.body()
        if m.ret.value:
            decl += ind(1) + f'return {m.ret.value};'
        decl += ind(0) + '}'

    decl += ind(0) + f'const JNINativeMethod {m.base_name()}_methods[] = {{'
    for m in methods:
        decl += ind(1) + '{'
        decl += ind(2) + f'"{m.base_name()}",'
        decl += ind(2) + f'"{m.jni()}",'
        decl += ind(2) + f'(void *) &{m.name}'
        decl += ind(1) + '},'
    decl += ind(0) + '};'
    decl = ind(0) + f'void *{m.base_name()}_orig = nullptr;' + decl
    decl += ind(0) + f'constexpr int {m.base_name()}_methods_num = std::size({m.base_name()}_methods);'
    decl += ind(0)

    hook_map[clz].append(m.base_name())

    return decl

def gen_jni_hook():
    decl = ''
    decl += ind(0) + 'static JNINativeMethod *hookAndSaveJNIMethods(const char *className, const JNINativeMethod *methods, int numMethods) {'
    decl += ind(1) + 'JNINativeMethod *newMethods = nullptr;'
    decl += ind(1) + 'int clz_id = -1;'
    decl += ind(1) + 'int hook_cnt = 0;'
    decl += ind(1) + 'do {'

    for index, (clz, methods) in enumerate(hook_map.items()):
        decl += ind(2) + f'if (className == "{clz}"sv) {{'
        decl += ind(3) + f'clz_id = {index};'
        decl += ind(3) + f'hook_cnt = {len(methods)};'
        decl += ind(3) + 'break;'
        decl += ind(2) + '}'

    decl += ind(1) + '} while (false);'

    decl += ind(1) + 'if (hook_cnt) {'
    decl += ind(2) + 'newMethods = new JNINativeMethod[numMethods];'
    decl += ind(2) + 'memcpy(newMethods, methods, sizeof(JNINativeMethod) * numMethods);'
    decl += ind(1) + '}'

    decl += ind(1) + 'auto &class_map = (*jni_method_map)[className];'
    decl += ind(1) + 'for (int i = 0; i < numMethods; ++i) {'

    for index, methods in enumerate(hook_map.values()):
        decl += ind(2) + f'if (hook_cnt && clz_id == {index}) {{'
        for m in methods:
            decl += ind(3) + f'HOOK_JNI({m})'
        decl += ind(2) + '}'

    decl += ind(2) + 'class_map[methods[i].name][methods[i].signature] = methods[i].fnPtr;'
    decl += ind(1) + '}'

    decl += ind(1) + 'return newMethods;'
    decl += ind(0) + '}'
    return decl

with open('jni_hooks.hpp', 'w') as f:
    f.write('// Generated by gen_jni_hooks.py\n')
    f.write('\nnamespace {\n')

    zygote = 'com/android/internal/os/Zygote'

    methods = [fas_l, fas_o, fas_p, fas_q_alt, fas_r, fas_samsung_m, fas_samsung_n, fas_samsung_o, fas_samsung_p]
    f.write(gen_jni_def(zygote, methods))

    methods = [spec_q, spec_q_alt, spec_r, spec_samsung_q]
    f.write(gen_jni_def(zygote, methods))

    methods = [server_l, server_samsung_q]
    f.write(gen_jni_def(zygote, methods))

    f.write('\n} // namespace\n')

    f.write(gen_jni_hook())
    f.write('\n')
