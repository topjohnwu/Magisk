#!/usr/bin/env python3
import sys
import os
import subprocess
import argparse
import multiprocessing
import zipfile
import datetime
import errno
import shutil
import lzma
import platform
import urllib.request
import os.path as op
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
archs = ['armeabi-v7a', 'x86']
arch64 = ['arm64-v8a', 'x86_64']
support_targets = ['magisk', 'magiskinit', 'magiskboot', 'magiskpolicy', 'resetprop', 'busybox', 'test']
default_targets = ['magisk', 'magiskinit', 'magiskboot', 'busybox']

ndk_root = op.join(os.environ['ANDROID_SDK_ROOT'], 'ndk')
ndk_path = op.join(ndk_root, 'magisk')
ndk_build = op.join(ndk_path, 'ndk-build')
gradlew = op.join('.', 'gradlew' + ('.bat' if is_windows else ''))

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


def rm_rf(path):
    vprint(f'rm -rf {path}')
    shutil.rmtree(path, ignore_errors=True)


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
    config['prettyName'] = 'false'

    # Load prop files
    if op.exists(args.config):
        config.update(parse_props(args.config))

    for key, value in parse_props('gradle.properties').items():
        if key.startswith('magisk.'):
            config[key[7:]] = value

    config['prettyName'] = config['prettyName'].lower() == 'true'

    try:
        config['versionCode'] = int(config['versionCode'])
    except ValueError:
        error('Config error: "versionCode" is required to be an integer')

    mkdir_p(config['outdir'])
    global STDOUT
    STDOUT = None if args.verbose else subprocess.DEVNULL


def zip_with_msg(zip_file, source, target):
    if not op.exists(source):
        error(f'{source} does not exist! Try build \'binary\' and \'apk\' before zipping!')
    zip_file.write(source, target)
    vprint(f'zip: {source} -> {target}')


def collect_binary():
    for arch in archs + arch64:
        mkdir_p(op.join('native', 'out', arch))
        for bin in support_targets + ['magiskinit64']:
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
                for arch in archs + arch64)
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


def binary_dump(src, out, var_name):
    out.write(f'constexpr unsigned char {var_name}[] = {{')
    for i, c in enumerate(xz(src.read())):
        if i % 16 == 0:
            out.write('\n')
        out.write(f'0x{c:02X},')
    out.write('\n};\n')
    out.flush()


def run_ndk_build(flags):
    os.chdir('native')
    proc = system(f'{ndk_build} {base_flags} {flags} -j{cpu_count}')
    if proc.returncode != 0:
        error('Build binary failed!')
    os.chdir('..')
    collect_binary()


def dump_bin_headers():
    stub = op.join(config['outdir'], 'stub-release.apk')
    if not op.exists(stub):
        error('Build stub APK before building "magiskinit"')
    with open(op.join('native', 'out', 'binaries.h'), 'w') as out:
        with open(stub, 'rb') as src:
            binary_dump(src, out, 'manager_xz')


def build_binary(args):
    # Verify NDK install
    props = parse_props(op.join(ndk_path, 'source.properties'))
    if props['Pkg.Revision'] != config['fullNdkVersion']:
        error('Incorrect NDK. Please install/upgrade NDK with "build.py ndk"')

    if args.target:
        args.target = set(args.target) & set(support_targets)
        if not args.target:
            return
    else:
        args.target = default_targets

    header('* Building binaries: ' + ' '.join(args.target))

    update_flags = False
    flags = op.join('native', 'jni', 'include', 'flags.hpp')
    flags_stat = os.stat(flags)

    if op.exists(args.config):
        if os.stat(args.config).st_mtime_ns > flags_stat.st_mtime_ns:
            update_flags = True

    last_commit = int(cmd_out(['git', 'log', '-1', r'--format=%at', 'HEAD']))
    if last_commit > flags_stat.st_mtime:
        update_flags = True

    if update_flags:
        os.utime(flags)

    # Basic flags
    global base_flags
    base_flags = f'MAGISK_VERSION={config["version"]} MAGISK_VER_CODE={config["versionCode"]}'
    if not args.release:
        base_flags += ' MAGISK_DEBUG=1'

    if 'magisk' in args.target:
        run_ndk_build('B_MAGISK=1 B_64BIT=1')
        clean_elf()

    if 'test' in args.target:
        run_ndk_build('B_TEST=1 B_64BIT=1')

    # 32-bit only targets can be built in one command
    flag = ''

    if 'magiskinit' in args.target:
        dump_bin_headers()
        flag += ' B_INIT=1'

    if 'magiskpolicy' in args.target:
        flag += ' B_POLICY=1'

    if 'resetprop' in args.target:
        flag += ' B_PROP=1'

    if 'magiskboot' in args.target:
        flag += ' B_BOOT=1'

    if 'busybox' in args.target:
        flag += ' B_BB=1'

    if flag:
        run_ndk_build(flag)


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
    return target


def build_app(args):
    header('* Building Magisk Manager')
    build_apk(args, 'app')


def build_stub(args):
    header('* Building Magisk Manager stub')
    build_apk(args, 'stub')


def build_snet(args):
    if not op.exists(op.join('snet', 'src', 'main', 'java', 'com', 'topjohnwu', 'snet')):
        error('snet sources have to be bind mounted on top of the stub folder')
    header('* Building snet extension')
    proc = execv([gradlew, 'stub:assembleRelease'])
    if proc.returncode != 0:
        error('Build snet extention failed!')
    source = op.join('stub', 'build', 'outputs', 'apk',
                     'release', 'stub-release.apk')
    target = op.join(config['outdir'], 'snet.jar')
    # Extract classes.dex
    with zipfile.ZipFile(target, 'w', compression=zipfile.ZIP_DEFLATED, allowZip64=False) as zout:
        with zipfile.ZipFile(source) as zin:
            zout.writestr('classes.dex', zin.read('classes.dex'))
    rm(source)
    header('Output: ' + target)


def zip_uninstaller(args):
    header('* Packing Uninstaller Zip')

    datestr = f'{datetime.datetime.now():%Y%m%d}'
    name = f'Magisk-uninstaller-{datestr}.zip' if config['prettyName'] else 'magisk-uninstaller.zip'
    output = op.join(config['outdir'], name)

    with zipfile.ZipFile(output, 'w', compression=zipfile.ZIP_DEFLATED, allowZip64=False) as zipf:
        # update-binary
        source = op.join('scripts', 'update_binary.sh')
        target = op.join('META-INF', 'com', 'google',
                         'android', 'update-binary')
        zip_with_msg(zipf, source, target)

        # updater-script
        source = op.join('scripts', 'magisk_uninstaller.sh')
        target = op.join('META-INF', 'com', 'google',
                         'android', 'updater-script')
        zip_with_msg(zipf, source, target)

        # Binaries
        for exe in ('busybox', 'magiskboot'):
            for arch in archs:
                source = op.join('native', 'out', arch, exe)
                target = op.join('lib', arch, f'lib{exe}.so')
                zip_with_msg(zipf, source, target)

        # util_functions.sh
        source = op.join('scripts', 'util_functions.sh')
        with open(source, 'r') as script:
            # Add version info util_functions.sh
            util_func = script.read().replace(
                '#MAGISK_VERSION_STUB',
                f'MAGISK_VER="{config["version"]}"\n'
                f'MAGISK_VER_CODE={config["versionCode"]}')
            target = op.join('assets', 'util_functions.sh')
            vprint(f'zip: {source} -> {target}')
            zipf.writestr(target, util_func)

        # chromeos
        for tool in ['futility', 'kernel_data_key.vbprivk', 'kernel.keyblock']:
            if tool == 'futility':
                source = op.join('tools', tool)
            else:
                source = op.join('tools', 'keys', tool)
            target = op.join('assets', 'chromeos', tool)
            zip_with_msg(zipf, source, target)

        # End of zipping

    sign_zip(output)
    header('Output: ' + output)


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
    url = f'https://dl.google.com/android/repository/android-ndk-r{ndk_ver}-{os_name}-x86_64.zip'
    ndk_zip = url.split('/')[-1]

    header(f'* Downloading {ndk_zip}')
    with urllib.request.urlopen(url) as response, open(ndk_zip, 'wb') as out_file:
        shutil.copyfileobj(response, out_file)

    header('* Extracting NDK zip')
    rm_rf(ndk_path)
    with zipfile.ZipFile(ndk_zip, 'r') as zf:
        for info in zf.infolist():
            extracted_path = zf.extract(info, ndk_root)
            vprint(f'Extracting {info.filename}')
            if info.create_system == 3:  # ZIP_UNIX_SYSTEM = 3
                unix_attributes = info.external_attr >> 16
            if unix_attributes:
                os.chmod(extracted_path, unix_attributes)
    mv(op.join(ndk_root, f'android-ndk-r{ndk_ver}'), ndk_path)

    header('* Removing unnecessary files')
    for dirname, subdirs, _ in os.walk(op.join(ndk_path, 'platforms')):
        for plats in subdirs:
            pp = op.join(dirname, plats)
            rm_rf(pp)
            mkdir(pp)
        subdirs.clear()
    rm_rf(op.join(ndk_path, 'sysroot'))

    header('* Replacing API-16 static libs')
    for target in ['arm-linux-androideabi', 'i686-linux-android']:
        arch = target.split('-')[0]
        lib_dir = op.join(
            ndk_path, 'toolchains', 'llvm', 'prebuilt', f'{os_name}-x86_64',
            'sysroot', 'usr', 'lib', f'{target}', '16')
        src_dir = op.join('tools', 'ndk-bins', arch)
        # Remove stupid macOS crap
        rm(op.join(src_dir, '.DS_Store'))
        for path in copy_tree(src_dir, lib_dir):
            vprint(f'Replaced {path}')


def build_all(args):
    vars(args)['target'] = []
    build_stub(args)
    build_binary(args)
    build_app(args)
    zip_uninstaller(args)


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

app_parser = subparsers.add_parser('app', help='build Magisk Manager')
app_parser.set_defaults(func=build_app)

stub_parser = subparsers.add_parser(
    'stub', help='build stub Magisk Manager')
stub_parser.set_defaults(func=build_stub)

# Need to bind mount snet sources on top of stub folder
# Note: source code for the snet extension is *NOT* public
snet_parser = subparsers.add_parser(
    'snet', help='build snet extension')
snet_parser.set_defaults(func=build_snet)

un_parser = subparsers.add_parser(
    'uninstaller', help='create flashable uninstaller')
un_parser.set_defaults(func=zip_uninstaller)

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
