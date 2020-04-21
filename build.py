#!/usr/bin/env python3
import sys
import os
import subprocess

is_windows = os.name == 'nt'
if is_windows:
    import colorama
    colorama.init()


def error(str):
    print('\n' + '\033[41m' + str + '\033[0m' + '\n')
    sys.exit(1)


def header(str):
    print('\n' + '\033[44m' + str + '\033[0m' + '\n')


def vprint(str):
    if args.verbose:
        print(str)


# Environment checks
if not sys.version_info >= (3, 6):
    error('Requires Python 3.6+')

if 'ANDROID_HOME' not in os.environ:
    error('Please add Android SDK path to ANDROID_HOME environment variable!')

try:
    subprocess.run(['javac', '-version'],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
except FileNotFoundError:
    error('Please install JDK and make sure \'javac\' is available in PATH')

import argparse
import multiprocessing
import zipfile
import datetime
import errno
import shutil
import lzma
import tempfile
import platform
import urllib.request
import os.path as op
from distutils.dir_util import copy_tree

cpu_count = multiprocessing.cpu_count()
archs = ['armeabi-v7a', 'x86']
arch64 = ['arm64-v8a', 'x86_64']
support_targets = ['magisk', 'magiskinit', 'magiskboot', 'magiskpolicy', 'resetprop', 'busybox', 'test']
default_targets = ['magisk', 'magiskinit', 'magiskboot', 'busybox']

ndk_ver = '21'
ndk_ver_full = '21.0.6113669'
build_tools_ver = '29.0.3'

ndk_root = op.join(os.environ['ANDROID_HOME'], 'ndk')
ndk_path = op.join(ndk_root, 'magisk')
ndk_build = op.join(ndk_path, 'ndk-build')
build_tools = op.join(os.environ['ANDROID_HOME'], 'build-tools', build_tools_ver)
gradlew = op.join('.', 'gradlew' + ('.bat' if is_windows else ''))

# Global vars
config = {}
STDOUT = None

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
            props[prop[0].strip(' \t\r\n')] = prop[1].strip(' \t\r\n')
    return props


def load_config(args):
    # Load prop file
    if not op.exists(args.config):
        error(f'Please make sure {args.config} exists')

    # Some default values
    config['outdir'] = 'out'
    config['prettyName'] = 'false'
    config['keyStore'] = 'release-key.jks'

    config.update(parse_props(args.config))

    # Sanitize configs

    config['prettyName'] = config['prettyName'].lower() == 'true'

    if 'version' not in config or 'versionCode' not in config:
        error('Config error: "version" and "versionCode" is required')

    try:
        config['versionCode'] = int(config['versionCode'])
    except ValueError:
        error('Config error: "versionCode" is required to be an integer')

    if args.release and not op.exists(config['keyStore']):
        error(f'Config error: assign "keyStore" to a java keystore')

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
    args.extend(op.join('native', 'out', arch, 'magisk') for arch in archs + arch64)
    execv(args)


def sign_zip(unsigned, output, release):
    if not release:
        mv(unsigned, output)
        return

    signer_name = 'zipsigner-3.0.jar'
    zipsigner = op.join('signing', 'build', 'libs', signer_name)

    if not op.exists(zipsigner):
        header('* Building ' + signer_name)
        proc = execv([gradlew, 'signing:shadowJar'])
        if proc.returncode != 0:
            error(f'Build {signer_name} failed!')

    header('* Signing Zip')

    proc = execv(['java', '-jar', zipsigner, config['keyStore'], config['keyStorePass'],
                  config['keyAlias'], config['keyPass'], unsigned, output])

    if proc.returncode != 0:
        error('Signing zip failed!')


def binary_dump(src, out, var_name):
    out.write(f'constexpr unsigned char {var_name}[] = {{')
    for i, c in enumerate(xz(src.read())):
        if i % 16 == 0:
            out.write('\n')
        out.write(f'0x{c:02X},')
    out.write('\n};\n')
    out.flush()


def gen_update_binary():
    bs = 1024
    update_bin = bytearray(bs)
    file = op.join('native', 'out', 'x86', 'busybox')
    with open(file, 'rb') as f:
        x86_bb = f.read()
    file = op.join('native', 'out', 'armeabi-v7a', 'busybox')
    with open(file, 'rb') as f:
        arm_bb = f.read()
    file = op.join('scripts', 'update_binary.sh')
    with open(file, 'rb') as f:
        script = f.read()
    # Align x86 busybox to bs
    blk_cnt = (len(x86_bb) - 1) // bs + 1
    script = script.replace(b'__X86_CNT__', b'%d' % blk_cnt)
    update_bin[:len(script)] = script
    update_bin.extend(x86_bb)
    # Padding for alignment
    update_bin.extend(b'\0' * (blk_cnt * bs - len(x86_bb)))
    update_bin.extend(arm_bb)
    return update_bin


def run_ndk_build(flags):
    os.chdir('native')
    proc = system(f'{ndk_build} {base_flags} {flags} -j{cpu_count}')
    if proc.returncode != 0:
        error('Build binary failed!')
    os.chdir('..')
    collect_binary()


def dump_bin_headers():
    for arch in archs:
        bin_file = op.join('native', 'out', arch, 'magisk')
        if not op.exists(bin_file):
            error('Build "magisk" before building "magiskinit"')
        with open(op.join('native', 'out', arch, 'binaries_arch.h'), 'w') as out:
            with open(bin_file, 'rb') as src:
                binary_dump(src, out, 'magisk_xz')
    for arch, arch32 in list(zip(arch64, archs)):
        bin_file = op.join('native', 'out', arch, 'magisk')
        with open(op.join('native', 'out', arch32, 'binaries_arch64.h'), 'w') as out:
            with open(bin_file, 'rb') as src:
                binary_dump(src, out, 'magisk_xz')
    stub = op.join(config['outdir'], 'stub-release.apk')
    if not op.exists(stub):
        stub = op.join(config['outdir'], 'stub-debug.apk')
        if not op.exists(stub):
            error('Build stub APK before building "magiskinit"')
    with open(op.join('native', 'out', 'binaries.h'), 'w') as out:
        with open(stub, 'rb') as src:
            binary_dump(src, out, 'manager_xz')


def build_binary(args):
    # Verify NDK install
    props = parse_props(op.join(ndk_path, 'source.properties'))
    if 'Pkg.Revision.orig' not in props or props['Pkg.Revision.orig'] != ndk_ver_full:
        error('Incorrect NDK. Please setup NDK with "build.py ndk"')

    if args.target:
        args.target = set(args.target) & set(support_targets)
        if not args.target:
            return
    else:
        args.target = default_targets

    header('* Building binaries: ' + ' '.join(args.target))

    os.utime(op.join('native', 'jni', 'include', 'flags.h'))

    # Basic flags
    global base_flags
    base_flags = f'MAGISK_VERSION={config["version"]} MAGISK_VER_CODE={config["versionCode"]}'
    if not args.release:
        base_flags += ' MAGISK_DEBUG=1'

    if 'magisk' in args.target:
        run_ndk_build('B_MAGISK=1 B_64BIT=1')
        clean_elf()

    if 'magiskinit' in args.target:
        dump_bin_headers()
        run_ndk_build('B_INIT=1')
        run_ndk_build('B_INIT64=1')

    if 'magiskpolicy' in args.target:
        run_ndk_build('B_POLICY=1')

    if 'resetprop' in args.target:
        run_ndk_build('B_PROP=1')

    if 'magiskboot' in args.target:
        run_ndk_build('B_BOOT=1')

    if 'busybox' in args.target:
        run_ndk_build('B_BB=1')

    if 'test' in args.target:
        run_ndk_build('B_TEST=1 B_64BIT=1')


def build_apk(args, module):
    build_type = 'Release' if args.release else 'Debug'

    proc = execv([gradlew, f'{module}:assemble{build_type}',
                 '-PconfigPath=' + op.abspath(args.config)])
    if proc.returncode != 0:
        error(f'Build {module} failed!')

    build_type = build_type.lower()
    apk = f'{module}-{build_type}.apk'

    source = op.join(module, 'build', 'outputs', 'apk', build_type, apk)
    target = op.join(config['outdir'], apk)

    if args.release:
        zipalign = op.join(build_tools, 'zipalign' + ('.exe' if is_windows else ''))
        aapt2 = op.join(build_tools, 'aapt2' + ('.exe' if is_windows else ''))
        apksigner = op.join(build_tools, 'apksigner' + ('.bat' if is_windows else ''))
        try:
            with tempfile.NamedTemporaryFile(delete=False) as f:
                tmp = f.name

            # AAPT2 optimization
            execv([aapt2, 'optimize', '-o', tmp, '--enable-resource-obfuscation',
                  '--enable-resource-path-shortening', source])

            # Recompress everything just to piss people off
            with zipfile.ZipFile(source, 'w', compression=zipfile.ZIP_DEFLATED) as zout:
                with zipfile.ZipFile(tmp) as zin:
                    for e in zin.namelist():
                        zout.writestr(e, zin.read(e))

            # Zipalign
            execv([zipalign, '-f', '4', source, target])

            # Sign APK
            execv([apksigner, 'sign', '--v1-signer-name', 'CERT',
                  '--ks', config['keyStore'],
                  '--ks-pass', f'pass:{config["keyStorePass"]}',
                  '--ks-key-alias', config['keyAlias'],
                  '--key-pass', f'pass:{config["keyPass"]}', target])
        finally:
            rm(tmp)
            rm(source)
    else:
        mv(source, target)

    header('Output: ' + target)
    return target


def build_app(args):
    header('* Building Magisk Manager')
    source = op.join('scripts', 'util_functions.sh')
    target = op.join('app', 'src', 'main',
                     'res', 'raw', 'util_functions.sh')
    cp(source, target)
    build_apk(args, 'app')


def build_stub(args):
    header('* Building Magisk Manager stub')
    build_apk(args, 'stub')


def zip_main(args):
    header('* Packing Flashable Zip')

    with tempfile.NamedTemporaryFile(delete=False) as f:
        unsigned = f.name

    with zipfile.ZipFile(unsigned, 'w', compression=zipfile.ZIP_DEFLATED, allowZip64=False) as zipf:
        # update-binary
        target = op.join('META-INF', 'com', 'google',
                         'android', 'update-binary')
        vprint('zip: ' + target)
        zipf.writestr(target, gen_update_binary())

        # updater-script
        source = op.join('scripts', 'flash_script.sh')
        target = op.join('META-INF', 'com', 'google',
                         'android', 'updater-script')
        zip_with_msg(zipf, source, target)

        # Binaries
        for lib_dir, zip_dir in [('armeabi-v7a', 'arm'), ('x86', 'x86')]:
            for binary in ['magiskinit', 'magiskinit64', 'magiskboot']:
                source = op.join('native', 'out', lib_dir, binary)
                target = op.join(zip_dir, binary)
                zip_with_msg(zipf, source, target)

        # APK
        source = op.join(
            config['outdir'], 'app-release.apk' if args.release else 'app-debug.apk')
        target = op.join('common', 'magisk.apk')
        zip_with_msg(zipf, source, target)

        # boot_patch.sh
        source = op.join('scripts', 'boot_patch.sh')
        target = op.join('common', 'boot_patch.sh')
        zip_with_msg(zipf, source, target)
        # util_functions.sh
        source = op.join('scripts', 'util_functions.sh')
        with open(source, 'r') as script:
            # Add version info util_functions.sh
            util_func = script.read().replace(
                '#MAGISK_VERSION_STUB',
                f'MAGISK_VER="{config["version"]}"\nMAGISK_VER_CODE={config["versionCode"]}')
            target = op.join('common', 'util_functions.sh')
            vprint(f'zip: {source} -> {target}')
            zipf.writestr(target, util_func)
        # addon.d.sh
        source = op.join('scripts', 'addon.d.sh')
        target = op.join('common', 'addon.d.sh')
        zip_with_msg(zipf, source, target)

        # chromeos
        for tool in ['futility', 'kernel_data_key.vbprivk', 'kernel.keyblock']:
            source = op.join('tools', tool)
            target = op.join('chromeos', tool)
            zip_with_msg(zipf, source, target)

        # End of zipping

    output = op.join(config['outdir'], f'Magisk-v{config["version"]}.zip' if config['prettyName'] else
                     'magisk-release.zip' if args.release else 'magisk-debug.zip')
    sign_zip(unsigned, output, args.release)
    rm(unsigned)
    header('Output: ' + output)


def zip_uninstaller(args):
    header('* Packing Uninstaller Zip')

    with tempfile.NamedTemporaryFile(delete=False) as f:
        unsigned = f.name

    with zipfile.ZipFile(unsigned, 'w', compression=zipfile.ZIP_DEFLATED, allowZip64=False) as zipf:
        # update-binary
        target = op.join('META-INF', 'com', 'google',
                         'android', 'update-binary')
        vprint('zip: ' + target)
        zipf.writestr(target, gen_update_binary())
        # updater-script
        source = op.join('scripts', 'magisk_uninstaller.sh')
        target = op.join('META-INF', 'com', 'google',
                         'android', 'updater-script')
        zip_with_msg(zipf, source, target)

        # Binaries
        for lib_dir, zip_dir in [('armeabi-v7a', 'arm'), ('x86', 'x86')]:
            source = op.join('native', 'out', lib_dir, 'magiskboot')
            target = op.join(zip_dir, 'magiskboot')
            zip_with_msg(zipf, source, target)

        # util_functions.sh
        source = op.join('scripts', 'util_functions.sh')
        with open(source, 'r') as script:
            target = op.join('util_functions.sh')
            vprint(f'zip: {source} -> {target}')
            zipf.writestr(target, script.read())

        # chromeos
        for tool in ['futility', 'kernel_data_key.vbprivk', 'kernel.keyblock']:
            source = op.join('tools', tool)
            target = op.join('chromeos', tool)
            zip_with_msg(zipf, source, target)

        # End of zipping

    datestr = datetime.datetime.now().strftime("%Y%m%d")
    output = op.join(config['outdir'], f'Magisk-uninstaller-{datestr}.zip'
                     if config['prettyName'] else 'magisk-uninstaller.zip')
    sign_zip(unsigned, output, args.release)
    rm(unsigned)
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
            if info.create_system == 3: # ZIP_UNIX_SYSTEM = 3
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
    for arch in ['arm', 'i686']:
      lib_dir = op.join(
        ndk_path, 'toolchains', 'llvm', 'prebuilt', f'{os_name}-x86_64',
        'sysroot', 'usr', 'lib', f'{arch}-linux-androideabi', '16')
      src_dir = op.join('tools', 'ndk-bins', arch)
      # Remove stupid macOS crap
      rm(op.join(src_dir, '.DS_Store'))
      for path in copy_tree(src_dir, lib_dir):
          vprint(f'Replaced {path}')

    # Rewrite source.properties
    src_prop = op.join(ndk_path, 'source.properties')
    props = parse_props(src_prop)
    props['Pkg.Revision.orig'] = props['Pkg.Revision']
    props['Pkg.Revision'] = '0.0.0'
    with open(src_prop, 'w') as p:
        for key, val in props.items():
            print(f'{key} = {val}', file=p)


def build_all(args):
    vars(args)['target'] = []
    build_stub(args)
    build_app(args)
    build_binary(args)
    zip_main(args)
    zip_uninstaller(args)


parser = argparse.ArgumentParser(description='Magisk build script')
parser.add_argument('-r', '--release', action='store_true',
                    help='compile in release mode')
parser.add_argument('-v', '--verbose', action='store_true',
                    help='verbose output')
parser.add_argument('-c', '--config', default='config.prop',
                    help='override config file (default: config.prop)')
subparsers = parser.add_subparsers(title='actions')

all_parser = subparsers.add_parser(
    'all', help='build binaries, apks, zips')
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

zip_parser = subparsers.add_parser(
    'zip', help='zip Magisk into a flashable zip')
zip_parser.set_defaults(func=zip_main)

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
