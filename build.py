#!/usr/bin/env python3
import sys
import os
import subprocess

if os.name == 'nt':
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
    subprocess.run(['java', '-version'],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
except FileNotFoundError:
    error('Please install JDK and make sure \'java\' is available in PATH')

import argparse
import multiprocessing
import zipfile
import datetime
import errno
import shutil
import lzma
import tempfile

if 'ANDROID_NDK_HOME' in os.environ:
    ndk_build = os.path.join(os.environ['ANDROID_NDK_HOME'], 'ndk-build')
else:
    ndk_build = os.path.join(
        os.environ['ANDROID_HOME'], 'ndk-bundle', 'ndk-build')

cpu_count = multiprocessing.cpu_count()
gradlew = os.path.join('.', 'gradlew.bat' if os.name == 'nt' else 'gradlew')
archs = ['armeabi-v7a', 'x86']
arch64 = ['arm64-v8a', 'x86_64']
keystore = 'release-key.jks'
config = {}


def mv(source, target):
    try:
        shutil.move(source, target)
    except:
        pass


def cp(source, target):
    try:
        shutil.copyfile(source, target)
        vprint(f'cp: {source} -> {target}')
    except:
        pass


def rm(file):
    try:
        os.remove(file)
    except OSError as e:
        if e.errno != errno.ENOENT:
            raise


def mkdir(path, mode=0o777):
    try:
        os.mkdir(path, mode)
    except:
        pass


def mkdir_p(path, mode=0o777):
    os.makedirs(path, mode, exist_ok=True)


def zip_with_msg(zip_file, source, target):
    if not os.path.exists(source):
        error(f'{source} does not exist! Try build \'binary\' and \'apk\' before zipping!')
    zip_file.write(source, target)
    vprint(f'zip: {source} -> {target}')


def collect_binary():
    for arch in archs + arch64:
        mkdir_p(os.path.join('native', 'out', arch))
        for bin in ['magisk', 'magiskinit', 'magiskinit64', 'magiskboot', 'busybox', 'test']:
            source = os.path.join('native', 'libs', arch, bin)
            target = os.path.join('native', 'out', arch, bin)
            mv(source, target)


def execv(cmd, redirect=None):
    return subprocess.run(cmd, stdout=redirect if redirect != None else STDOUT)


def system(cmd, redirect=None):
    return subprocess.run(cmd, shell=True, stdout=redirect if redirect != None else STDOUT)


def xz(data):
    return lzma.compress(data, preset=9, check=lzma.CHECK_NONE)


def sign_zip(unsigned, output, release):
    signer_name = 'zipsigner-3.0.jar'
    zipsigner = os.path.join('signing', 'build', 'libs', signer_name)

    if not os.path.exists(zipsigner):
        header('* Building ' + signer_name)
        proc = execv([gradlew, 'signing:shadowJar'])
        if proc.returncode != 0:
            error(f'Build {signer_name} failed!')

    header('* Signing Zip')

    if release:
        proc = execv(['java', '-jar', zipsigner, keystore, config['keyStorePass'],
                      config['keyAlias'], config['keyPass'], unsigned, output])
    else:
        proc = execv(['java', '-jar', zipsigner, unsigned, output])

    if proc.returncode != 0:
        error('Signing zip failed!')


def binary_dump(src, out, var_name):
    out.write(f'const static unsigned char {var_name}[] = {{')
    for i, c in enumerate(xz(src.read())):
        if i % 16 == 0:
            out.write('\n')
        out.write(f'0x{c:02X},')
    out.write('\n};\n')
    out.flush()


def gen_update_binary():
    bs = 1024
    update_bin = bytearray(bs)
    file = os.path.join('native', 'out', 'x86', 'busybox')
    with open(file, 'rb') as f:
        x86_bb = f.read()
    file = os.path.join('native', 'out', 'armeabi-v7a', 'busybox')
    with open(file, 'rb') as f:
        arm_bb = f.read()
    file = os.path.join('scripts', 'update_binary.sh')
    with open(file, 'rb') as f:
        script = f.read()
    # Align x86 busybox to bs
    blkCnt = (len(x86_bb) - 1) // bs + 1
    script = script.replace(b'__X86_CNT__', b'%d' % blkCnt)
    update_bin[:len(script)] = script
    update_bin.extend(x86_bb)
    # Padding for alignment
    update_bin.extend(b'\0' * (blkCnt * bs - len(x86_bb)))
    update_bin.extend(arm_bb)
    return update_bin


def run_ndk_build(flags):
    proc = system(f'{ndk_build} -C native {base_flags} {flags} -j{cpu_count}')
    if proc.returncode != 0:
        error('Build binary failed!')
    collect_binary()


def build_binary(args):
    support_targets = {'magisk', 'magiskinit', 'magiskboot', 'busybox', 'test'}
    if args.target:
        args.target = set(args.target) & support_targets
        if not args.target:
            return
    else:
        # If nothing specified, build everything
        args.target = ['magisk', 'magiskinit', 'magiskboot', 'busybox']

    header('* Building binaries: ' + ' '.join(args.target))

    os.utime(os.path.join('native', 'jni', 'include', 'flags.h'))

    # Basic flags
    global base_flags
    base_flags = f'MAGISK_VERSION={config["version"]} MAGISK_VER_CODE={config["versionCode"]}'
    if not args.release:
        base_flags += ' MAGISK_DEBUG=1'

    if 'magisk' in args.target:
        run_ndk_build('B_MAGISK=1 B_64BIT=1')
        # Dump the binary to header
        for arch in archs:
            bin_file = os.path.join('native', 'out', arch, 'magisk')
            with open(os.path.join('native', 'out', arch, 'binaries_arch.h'), 'w') as out:
                with open(bin_file, 'rb') as src:
                    binary_dump(src, out, 'magisk_xz')
        for arch, arch32 in list(zip(arch64, archs)):
            bin_file = os.path.join('native', 'out', arch, 'magisk')
            with open(os.path.join('native', 'out', arch32, 'binaries_arch64.h'), 'w') as out:
                with open(bin_file, 'rb') as src:
                    binary_dump(src, out, 'magisk_xz')

    if 'busybox' in args.target:
        run_ndk_build('B_BB=1')

    if 'magiskinit' in args.target:
        if not os.path.exists(os.path.join('native', 'out', 'x86', 'binaries_arch.h')):
            error('Build "magisk" before building "magiskinit"')
        if not os.path.exists(os.path.join('native', 'out', 'binaries.h')):
            error('Build stub APK before building "magiskinit"')
        run_ndk_build('B_INIT=1')
        run_ndk_build('B_INIT64=1')

    if 'magiskboot' in args.target:
        run_ndk_build('B_BOOT=1')

    if 'test' in args.target:
        run_ndk_build('B_TEST=1 B_64BIT=1')


def build_apk(args, module):
    build_type = 'Release' if args.release else 'Debug'

    proc = execv([gradlew, f'{module}:assemble{build_type}',
                 '-PconfigPath=' + os.path.abspath(args.config)])
    if proc.returncode != 0:
        error('Build Magisk Manager failed!')

    build_type = build_type.lower()
    apk = f'{module}-{build_type}.apk'

    source = os.path.join(module, 'build', 'outputs', 'apk', build_type, apk)
    target = os.path.join(config['outdir'], apk)
    mv(source, target)
    header('Output: ' + target)
    return target


def build_app(args):
    header('* Building Magisk Manager')
    source = os.path.join('scripts', 'util_functions.sh')
    target = os.path.join('app', 'src', 'main',
                          'res', 'raw', 'util_functions.sh')
    cp(source, target)
    build_apk(args, 'app')


def build_stub(args):
    header('* Building Magisk Manager stub')
    stub = build_apk(args, 'stub')
    # Dump the stub APK to header
    mkdir(os.path.join('native', 'out'))
    with open(os.path.join('native', 'out', 'binaries.h'), 'w') as out:
        with open(stub, 'rb') as src:
            binary_dump(src, out, 'manager_xz')


def build_snet(args):
    header('* Building snet extension')
    proc = execv([gradlew, 'snet:assembleRelease'])
    if proc.returncode != 0:
        error('Build snet extention failed!')
    source = os.path.join('snet', 'build', 'outputs', 'apk',
                          'release', 'snet-release-unsigned.apk')
    target = os.path.join(config['outdir'], 'snet.apk')
    # Re-compress the whole APK for smaller size
    with zipfile.ZipFile(target, 'w', compression=zipfile.ZIP_DEFLATED, allowZip64=False) as zout:
        with zipfile.ZipFile(source) as zin:
            for item in zin.infolist():
                zout.writestr(item.filename, zin.read(item))
    rm(source)
    header('Output: ' + target)


def zip_main(args):
    header('* Packing Flashable Zip')

    unsigned = tempfile.mkstemp()[1]

    with zipfile.ZipFile(unsigned, 'w', compression=zipfile.ZIP_DEFLATED, allowZip64=False) as zipf:
        # META-INF
        # update-binary
        target = os.path.join('META-INF', 'com', 'google',
                              'android', 'update-binary')
        vprint('zip: ' + target)
        zipf.writestr(target, gen_update_binary())
        # updater-script
        source = os.path.join('scripts', 'flash_script.sh')
        target = os.path.join('META-INF', 'com', 'google',
                              'android', 'updater-script')
        zip_with_msg(zipf, source, target)

        # Binaries
        for lib_dir, zip_dir in [('armeabi-v7a', 'arm'), ('x86', 'x86')]:
            for binary in ['magiskinit', 'magiskinit64', 'magiskboot']:
                source = os.path.join('native', 'out', lib_dir, binary)
                target = os.path.join(zip_dir, binary)
                zip_with_msg(zipf, source, target)

        # APK
        source = os.path.join(
            config['outdir'], 'app-release.apk' if args.release else 'app-debug.apk')
        target = os.path.join('common', 'magisk.apk')
        zip_with_msg(zipf, source, target)

        # Scripts
        # boot_patch.sh
        source = os.path.join('scripts', 'boot_patch.sh')
        target = os.path.join('common', 'boot_patch.sh')
        zip_with_msg(zipf, source, target)
        # util_functions.sh
        source = os.path.join('scripts', 'util_functions.sh')
        with open(source, 'r') as script:
            # Add version info util_functions.sh
            util_func = script.read().replace(
                '#MAGISK_VERSION_STUB',
                f'MAGISK_VER="{config["version"]}"\nMAGISK_VER_CODE={config["versionCode"]}')
            target = os.path.join('common', 'util_functions.sh')
            vprint(f'zip: {source} -> {target}')
            zipf.writestr(target, util_func)
        # addon.d.sh
        source = os.path.join('scripts', 'addon.d.sh')
        target = os.path.join('common', 'addon.d.sh')
        zip_with_msg(zipf, source, target)

        # Prebuilts
        for chromeos in ['futility', 'kernel_data_key.vbprivk', 'kernel.keyblock']:
            source = os.path.join('chromeos', chromeos)
            zip_with_msg(zipf, source, source)

        # End of zipping

    output = os.path.join(config['outdir'], f'Magisk-v{config["version"]}.zip' if config['prettyName'] else
                          'magisk-release.zip' if args.release else 'magisk-debug.zip')
    sign_zip(unsigned, output, args.release)
    header('Output: ' + output)


def zip_uninstaller(args):
    header('* Packing Uninstaller Zip')

    unsigned = tempfile.mkstemp()[1]

    with zipfile.ZipFile(unsigned, 'w', compression=zipfile.ZIP_DEFLATED, allowZip64=False) as zipf:
        # META-INF
        # update-binary
        target = os.path.join('META-INF', 'com', 'google',
                              'android', 'update-binary')
        vprint('zip: ' + target)
        zipf.writestr(target, gen_update_binary())
        # updater-script
        source = os.path.join('scripts', 'magisk_uninstaller.sh')
        target = os.path.join('META-INF', 'com', 'google',
                              'android', 'updater-script')
        zip_with_msg(zipf, source, target)

        # Binaries
        for lib_dir, zip_dir in [('armeabi-v7a', 'arm'), ('x86', 'x86')]:
            source = os.path.join('native', 'out', lib_dir, 'magiskboot')
            target = os.path.join(zip_dir, 'magiskboot')
            zip_with_msg(zipf, source, target)

        # Scripts
        # util_functions.sh
        source = os.path.join('scripts', 'util_functions.sh')
        with open(source, 'r') as script:
            target = os.path.join('util_functions.sh')
            vprint(f'zip: {source} -> {target}')
            zipf.writestr(target, script.read())

        # Prebuilts
        for chromeos in ['futility', 'kernel_data_key.vbprivk', 'kernel.keyblock']:
            source = os.path.join('chromeos', chromeos)
            zip_with_msg(zipf, source, source)

        # End of zipping

    output = os.path.join(config['outdir'], f'Magisk-uninstaller-{datetime.datetime.now().strftime("%Y%m%d")}.zip'
                          if config['prettyName'] else 'magisk-uninstaller.zip')
    sign_zip(unsigned, output, args.release)
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
        system(ndk_build + ' -C native B_MAGISK=1 B_INIT=1 B_BOOT=1 B_BB=1 clean')
        shutil.rmtree(os.path.join('native', 'out'), ignore_errors=True)

    if 'java' in args.target:
        header('* Cleaning java')
        execv([gradlew, 'clean'])


def build_all(args):
    vars(args)['target'] = []
    build_stub(args)
    build_app(args)
    build_binary(args)
    zip_main(args)
    zip_uninstaller(args)
    build_snet(args)


parser = argparse.ArgumentParser(description='Magisk build script')
parser.add_argument('-r', '--release', action='store_true',
                    help='compile Magisk for release')
parser.add_argument('-v', '--verbose', action='store_true',
                    help='verbose output')
parser.add_argument('-c', '--config', default='config.prop',
                    help='config file location')
subparsers = parser.add_subparsers(title='actions')

all_parser = subparsers.add_parser(
    'all', help='build everything (binaries/apks/zips)')
all_parser.set_defaults(func=build_all)

binary_parser = subparsers.add_parser('binary', help='build binaries')
binary_parser.add_argument(
    'target', nargs='*', help='Support: magisk, magiskinit, magiskboot, busybox. Leave empty to build all.')
binary_parser.set_defaults(func=build_binary)

apk_parser = subparsers.add_parser('apk', help='build Magisk Manager APK')
apk_parser.set_defaults(func=build_app)

stub_parser = subparsers.add_parser(
    'stub', help='build stub Magisk Manager APK')
stub_parser.set_defaults(func=build_stub)

snet_parser = subparsers.add_parser(
    'snet', help='build snet extention for Magisk Manager')
snet_parser.set_defaults(func=build_snet)

zip_parser = subparsers.add_parser(
    'zip', help='zip Magisk into a flashable zip')
zip_parser.set_defaults(func=zip_main)

un_parser = subparsers.add_parser(
    'uninstaller', help='create flashable uninstaller')
un_parser.set_defaults(func=zip_uninstaller)

clean_parser = subparsers.add_parser('clean', help='cleanup.')
clean_parser.add_argument(
    'target', nargs='*', help='Support: native, java. Leave empty to clean all.')
clean_parser.set_defaults(func=cleanup)

if len(sys.argv) == 1:
    parser.print_help()
    sys.exit(1)

args = parser.parse_args()

# Some default values
config['outdir'] = 'out'
config['prettyName'] = 'false'

with open(args.config, 'r') as f:
    for line in [l.strip(' \t\r\n') for l in f]:
        if line.startswith('#') or len(line) == 0:
            continue
        prop = line.split('=')
        config[prop[0].strip(' \t\r\n')] = prop[1].strip(' \t\r\n')

if 'version' not in config or 'versionCode' not in config:
    error('"version" and "versionCode" is required in "config.prop"')

try:
    config['versionCode'] = int(config['versionCode'])
except ValueError:
    error('"versionCode" is required to be an integer')

config['prettyName'] = config['prettyName'].lower() == 'true'

mkdir_p(config['outdir'])

if args.release and not os.path.exists(keystore):
    error(f'Please generate a java keystore and place it in "{keystore}"')
STDOUT = None if args.verbose else subprocess.DEVNULL
args.func(args)
