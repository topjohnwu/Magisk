#!/usr/bin/env python3
import argparse
import errno
import lzma
import multiprocessing
import os
import os.path as op
import platform
import shutil
import stat
import subprocess
import sys
import textwrap
import urllib.request
import zipfile
from distutils.dir_util import copy_tree


def error(str):
    if is_ci:
        print(f'\n ! {str}\n')
    else:
        print(f'\n\033[41m{str}\033[0m\n')
    sys.exit(1)


def header(str):
    if is_ci:
        print(f'\n{str}\n')
    else:
        print(f'\n\033[44m{str}\033[0m\n')


def vprint(str):
    if args.verbose:
        print(str)


is_windows = os.name == 'nt'
is_ci = 'CI' in os.environ and os.environ['CI'] == 'true'

if not is_ci and is_windows:
    import colorama

    colorama.init()

# Environment checks
if not sys.version_info >= (3, 6):
    error('Requires Python 3.6+')

if 'ANDROID_SDK_ROOT' not in os.environ:
    error('Please add Android SDK path to ANDROID_SDK_ROOT environment variable!')

try:
    subprocess.run(['javac', '-version'],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
except FileNotFoundError:
    error('Please install JDK and make sure \'javac\' is available in PATH')

cpu_count = multiprocessing.cpu_count()
archs = ['armeabi-v7a', 'x86', 'arm64-v8a', 'x86_64']
default_targets = ['magisk', 'magiskinit', 'magiskboot', 'busybox']
support_targets = default_targets + ['magiskpolicy', 'resetprop', 'test']

sdk_path = os.environ['ANDROID_SDK_ROOT']
ndk_root = op.join(sdk_path, 'ndk')
ndk_path = op.join(ndk_root, 'magisk')
ndk_build = op.join(ndk_path, 'ndk-build')
gradlew = op.join('.', 'gradlew' + ('.bat' if is_windows else ''))
adb_path = op.join(sdk_path, 'platform-tools', 'adb' + ('.exe' if is_windows else ''))
native_gen_path = op.join('native', 'out', 'generated')

# Global vars
config = {}
STDOUT = None
build_tools = None


def mv(source, target):
    try:
        shutil.move(source, target)
        vprint(f'mv {source} -> {target}')
    except:
        pass


def cp(source, target):
    try:
        shutil.copyfile(source, target)
        vprint(f'cp {source} -> {target}')
    except:
        pass


def rm(file):
    try:
        os.remove(file)
        vprint(f'rm {file}')
    except OSError as e:
        if e.errno != errno.ENOENT:
            raise


def rm_on_error(func, path, _):
    # Remove a read-only file on Windows will get "WindowsError: [Error 5] Access is denied"
    # Clear the "read-only" and retry
    os.chmod(path, stat.S_IWRITE)
    os.unlink(path)


def rm_rf(path):
    vprint(f'rm -rf {path}')
    shutil.rmtree(path, ignore_errors=True, onerror=rm_on_error)


def mkdir(path, mode=0o755):
    try:
        os.mkdir(path, mode)
    except:
        pass


def mkdir_p(path, mode=0o755):
    os.makedirs(path, mode, exist_ok=True)


def execv(cmd):
    return subprocess.run(cmd, stdout=STDOUT)


def system(cmd):
    return subprocess.run(cmd, shell=True, stdout=STDOUT)


def cmd_out(cmd):
    return subprocess.check_output(cmd).strip().decode('utf-8')


def xz(data):
    return lzma.compress(data, preset=9, check=lzma.CHECK_NONE)


def parse_props(file):
    props = {}
    with open(file, 'r') as f:
        for line in [l.strip(' \t\r\n') for l in f]:
            if line.startswith('#') or len(line) == 0:
                continue
            prop = line.split('=')
            if len(prop) != 2:
                continue
            value = prop[1].strip(' \t\r\n')
            if len(value) == 0:
                continue
            props[prop[0].strip(' \t\r\n')] = value
    return props


def load_config(args):
    commit_hash = cmd_out(['git', 'rev-parse', '--short=8', 'HEAD'])

    # Default values
    config['version'] = commit_hash
    config['outdir'] = 'out'

    # Load prop files
    if op.exists(args.config):
        config.update(parse_props(args.config))

    for key, value in parse_props('gradle.properties').items():
        if key.startswith('magisk.'):
            config[key[7:]] = value

    try:
        config['versionCode'] = int(config['versionCode'])
    except ValueError:
        error('Config error: "versionCode" is required to be an integer')

    mkdir_p(config['outdir'])
    global STDOUT
    STDOUT = None if args.verbose else subprocess.DEVNULL


def collect_binary():
    for arch in archs:
        mkdir_p(op.join('native', 'out', arch))
        for bin in support_targets:
            source = op.join('native', 'libs', arch, bin)
            target = op.join('native', 'out', arch, bin)
            mv(source, target)


def clean_elf():
    if is_windows:
        elf_cleaner = op.join('tools', 'elf-cleaner.exe')
    else:
        elf_cleaner = op.join('native', 'out', 'elf-cleaner')
        if not op.exists(elf_cleaner):
            execv(['g++', '-std=c++11', 'tools/termux-elf-cleaner/termux-elf-cleaner.cpp',
                   '-o', elf_cleaner])
    args = [elf_cleaner]
    args.extend(op.join('native', 'out', arch, 'magisk')
                for arch in archs)
    execv(args)


def find_build_tools():
    global build_tools
    if build_tools:
        return build_tools
    build_tools_root = op.join(os.environ['ANDROID_SDK_ROOT'], 'build-tools')
    ls = os.listdir(build_tools_root)
    # Use the latest build tools available
    ls.sort()
    build_tools = op.join(build_tools_root, ls[-1])
    return build_tools


# Unused but keep this code
def sign_zip(unsigned):
    if 'keyStore' not in config:
        return

    msg = '* Signing APK'
    apksigner = op.join(find_build_tools(), 'apksigner' + ('.bat' if is_windows else ''))

    exec_args = [apksigner, 'sign',
                 '--ks', config['keyStore'],
                 '--ks-pass', f'pass:{config["keyStorePass"]}',
                 '--ks-key-alias', config['keyAlias'],
                 '--key-pass', f'pass:{config["keyPass"]}',
                 '--v1-signer-name', 'CERT',
                 '--v4-signing-enabled', 'false']

    if unsigned.endswith('.zip'):
        msg = '* Signing zip'
        exec_args.extend(['--min-sdk-version', '17',
                          '--v2-signing-enabled', 'false',
                          '--v3-signing-enabled', 'false'])

    exec_args.append(unsigned)

    header(msg)
    proc = execv(exec_args)
    if proc.returncode != 0:
        error('Signing failed!')


def binary_dump(src, var_name):
    out_str = f'constexpr unsigned char {var_name}[] = {{'
    for i, c in enumerate(xz(src.read())):
        if i % 16 == 0:
            out_str += '\n'
        out_str += f'0x{c:02X},'
    out_str += '\n};\n'
    return out_str


def run_ndk_build(flags):
    os.chdir('native')
    proc = system(f'{ndk_build} {flags} -j{cpu_count}')
    if proc.returncode != 0:
        error('Build binary failed!')
    os.chdir('..')
    collect_binary()


def write_if_diff(file_name, text):
    do_write = True
    if op.exists(file_name):
        with open(file_name, 'r') as f:
            orig = f.read()
        do_write = orig != text
    if do_write:
        with open(file_name, 'w') as f:
            f.write(text)


def dump_bin_header():
    stub = op.join(config['outdir'], 'stub-release.apk')
    if not op.exists(stub):
        error('Build stub APK before building "magiskinit"')
    mkdir_p(native_gen_path)
    with open(stub, 'rb') as src:
        text = binary_dump(src, 'manager_xz')
        write_if_diff(op.join(native_gen_path, 'binaries.h'), text)


def dump_flag_header():
    flag_txt = textwrap.dedent('''\
        #pragma once
        #define quote(s)            #s
        #define str(s)              quote(s)
        #define MAGISK_FULL_VER     MAGISK_VERSION "(" str(MAGISK_VER_CODE) ")"
        #define NAME_WITH_VER(name) str(name) " " MAGISK_FULL_VER
        ''')
    flag_txt += f'#define MAGISK_VERSION      "{config["version"]}"\n'
    flag_txt += f'#define MAGISK_VER_CODE     {config["versionCode"]}\n'
    flag_txt += f'#define MAGISK_DEBUG        {0 if args.release else 1}\n'

    mkdir_p(native_gen_path)
    write_if_diff(op.join(native_gen_path, 'flags.h'), flag_txt)


def build_binary(args):
    # Verify NDK install
    props = parse_props(op.join(ndk_path, 'source.properties'))
    if props['Pkg.Revision'] != config['fullNdkVersion']:
        error('Incorrect NDK. Please install/upgrade NDK with "build.py ndk"')

    if 'target' not in vars(args):
        vars(args)['target'] = []

    if args.target:
        args.target = set(args.target) & set(support_targets)
        if not args.target:
            return
    else:
        args.target = default_targets

    header('* Building binaries: ' + ' '.join(args.target))

    dump_flag_header()

    flag = ''

    if 'magisk' in args.target:
        flag += ' B_MAGISK=1'

    if 'test' in args.target:
        flag += ' B_TEST=1'

    if flag:
        run_ndk_build(flag + ' B_SHARED=1')

    if 'magisk' in args.target:
        clean_elf()

    if 'magiskinit' in args.target:
        dump_bin_header()
        flag += ' B_INIT=1'

    if 'magiskpolicy' in args.target:
        flag += ' B_POLICY=1'

    if 'resetprop' in args.target:
        flag += ' B_PROP=1'

    if 'magiskboot' in args.target:
        flag += ' B_BOOT=1'

    if flag:
        run_ndk_build(flag)

    if 'busybox' in args.target:
        run_ndk_build('B_BB=1')


def build_apk(args, module):
    build_type = 'Release' if args.release or module == 'stub' else 'Debug'

    proc = execv([gradlew, f'{module}:assemble{build_type}',
                  '-PconfigPath=' + op.abspath(args.config)])
    if proc.returncode != 0:
        error(f'Build {module} failed!')

    build_type = build_type.lower()

    apk = f'{module}-{build_type}.apk'
    source = op.join(module, 'build', 'outputs', 'apk', build_type, apk)
    target = op.join(config['outdir'], apk)
    mv(source, target)
    header('Output: ' + target)


def build_app(args):
    header('* Building the Magisk app')
    build_apk(args, 'app')


def build_stub(args):
    header('* Building the stub app')
    build_apk(args, 'stub')


def cleanup(args):
    support_targets = {'native', 'java'}
    if args.target:
        args.target = set(args.target) & support_targets
    else:
        # If nothing specified, clean everything
        args.target = support_targets

    if 'native' in args.target:
        header('* Cleaning native')
        rm_rf(op.join('native', 'out'))
        rm_rf(op.join('native', 'libs'))
        rm_rf(op.join('native', 'obj'))

    if 'java' in args.target:
        header('* Cleaning java')
        execv([gradlew, 'clean'])


def setup_ndk(args):
    os_name = platform.system().lower()
    ndk_ver = config['ndkVersion']
    url = f'https://dl.google.com/android/repository/android-ndk-r{ndk_ver}-{os_name}.zip'
    ndk_zip = url.split('/')[-1]

    header(f'* Downloading {ndk_zip}')
    with urllib.request.urlopen(url) as response, open(ndk_zip, 'wb') as out_file:
        shutil.copyfileobj(response, out_file)

    header('* Extracting NDK zip')
    rm_rf(ndk_path)
    with zipfile.ZipFile(ndk_zip, 'r') as zf:
        for info in zf.infolist():
            vprint(f'Extracting {info.filename}')
            if info.external_attr >> 28 == 0xA:  # symlink
                src = zf.read(info).decode("utf-8")
                dest = op.join(ndk_root, info.filename)
                os.symlink(src, dest)
                continue
            extracted_path = zf.extract(info, ndk_root)
            if info.create_system == 3:  # ZIP_UNIX_SYSTEM = 3
                unix_attributes = info.external_attr >> 16
            if unix_attributes:
                os.chmod(extracted_path, unix_attributes)
    mv(op.join(ndk_root, f'android-ndk-r{ndk_ver}'), ndk_path)

    header('* Patching static libs')
    for target in ['aarch64-linux-android', 'arm-linux-androideabi',
                   'i686-linux-android', 'x86_64-linux-android']:
        arch = target.split('-')[0]
        lib_dir = op.join(
            ndk_path, 'toolchains', 'llvm', 'prebuilt', f'{os_name}-x86_64',
            'sysroot', 'usr', 'lib', f'{target}', '21')
        if not op.exists(lib_dir):
            continue
        src_dir = op.join('tools', 'ndk-bins', '21', arch)
        rm(op.join(src_dir, '.DS_Store'))
        for path in copy_tree(src_dir, lib_dir):
            vprint(f'Replaced {path}')


def setup_avd(args):
    if not args.skip:
        build_binary(args)
        build_app(args)

    header('* Setting up emulator')

    abi = cmd_out([adb_path, 'shell', 'getprop', 'ro.product.cpu.abi'])
    proc = execv([adb_path, 'push', f'native/out/{abi}/busybox', 'out/app-debug.apk',
           'scripts/avd_magisk.sh', '/data/local/tmp'])
    if proc.returncode != 0:
        error('adb push failed!')

    proc = execv([adb_path, 'shell', 'sh', '/data/local/tmp/avd_magisk.sh'])
    if proc.returncode != 0:
        error('avd_magisk.sh failed!')


def patch_avd_ramdisk(args):
    if not args.skip:
        build_binary(args)
        build_app(args)

    header('* Patching emulator ramdisk.img')

    # Create a backup to prevent accidental overwrites
    backup = args.ramdisk + '.bak'
    if not op.exists(backup):
        cp(args.ramdisk, backup)

    ini = op.join(op.dirname(args.ramdisk), 'advancedFeatures.ini')
    with open(ini, 'r') as f:
        adv_ft = f.read()

    # Need to turn off system as root
    if 'SystemAsRoot = on' in adv_ft:
        # Create a backup
        cp(ini, ini + '.bak')
        adv_ft = adv_ft.replace('SystemAsRoot = on', 'SystemAsRoot = off')
        with open(ini, 'w') as f:
            f.write(adv_ft)

    abi = cmd_out([adb_path, 'shell', 'getprop', 'ro.product.cpu.abi'])
    proc = execv([adb_path, 'push', f'native/out/{abi}/busybox', 'out/app-debug.apk',
           'scripts/avd_patch.sh', '/data/local/tmp'])
    if proc.returncode != 0:
        error('adb push failed!')
    proc = execv([adb_path, 'push', backup, '/data/local/tmp/ramdisk.cpio.tmp'])
    if proc.returncode != 0:
        error('adb push failed!')

    proc = execv([adb_path, 'shell', 'sh', '/data/local/tmp/avd_patch.sh'])
    if proc.returncode != 0:
        error('avd_patch.sh failed!')

    proc = execv([adb_path, 'pull', '/data/local/tmp/ramdisk.cpio.gz', args.ramdisk])
    if proc.returncode != 0:
        error('adb pull failed!')


def build_all(args):
    build_stub(args)
    build_binary(args)
    build_app(args)


parser = argparse.ArgumentParser(description='Magisk build script')
parser.set_defaults(func=lambda x: None)
parser.add_argument('-r', '--release', action='store_true',
                    help='compile in release mode')
parser.add_argument('-v', '--verbose', action='store_true',
                    help='verbose output')
parser.add_argument('-c', '--config', default='config.prop',
                    help='custom config file (default: config.prop)')
subparsers = parser.add_subparsers(title='actions')

all_parser = subparsers.add_parser(
    'all', help='build everything')
all_parser.set_defaults(func=build_all)

binary_parser = subparsers.add_parser('binary', help='build binaries')
binary_parser.add_argument(
    'target', nargs='*', help=f"{', '.join(support_targets)}, \
    or empty for defaults ({', '.join(default_targets)})")
binary_parser.set_defaults(func=build_binary)

app_parser = subparsers.add_parser('app', help='build the Magisk app')
app_parser.set_defaults(func=build_app)

stub_parser = subparsers.add_parser('stub', help='build the stub app')
stub_parser.set_defaults(func=build_stub)

avd_parser = subparsers.add_parser(
    'emulator', help='setup AVD for development')
avd_parser.add_argument('-s', '--skip', action='store_true',
    help='skip building binaries and the app')
avd_parser.set_defaults(func=setup_avd)

avd_patch_parser = subparsers.add_parser(
    'avd_patch', help='patch AVD ramdisk.img')
avd_patch_parser.add_argument('ramdisk', help='path to ramdisk.img')
avd_patch_parser.add_argument('-s', '--skip', action='store_true',
    help='skip building binaries and the app')
avd_patch_parser.set_defaults(func=patch_avd_ramdisk)

clean_parser = subparsers.add_parser('clean', help='cleanup')
clean_parser.add_argument(
    'target', nargs='*', help='native, java, or empty to clean both')
clean_parser.set_defaults(func=cleanup)

ndk_parser = subparsers.add_parser('ndk', help='setup Magisk NDK')
ndk_parser.set_defaults(func=setup_ndk)

if len(sys.argv) == 1:
    parser.print_help()
    sys.exit(1)

args = parser.parse_args()
load_config(args)

# Call corresponding functions
args.func(args)
