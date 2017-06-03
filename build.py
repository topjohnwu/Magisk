#!/usr/bin/env python3
import sys
import os
import subprocess

def error(str):
	print('\n' + '\033[41m' + str + '\033[0m' + '\n')
	sys.exit(1)

def header(str):
	print('\n' + '\033[44m' + str + '\033[0m' + '\n')

# Environment checks
if not sys.version_info >= (3, 5):
	error('Requires Python >= 3.5')

if 'ANDROID_HOME' not in os.environ:
	error('Please add Android SDK path to ANDROID_HOME environment variable!')

try:
	subprocess.run(['java', '-version'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
except FileNotFoundError:
	error('Please install Java and make sure \'java\' is available in PATH')

# If not Windows, we need gcc to compile
if os.name != 'nt':
	try:
		subprocess.run(['gcc', '-v'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
	except FileNotFoundError:
		error('Please install C compiler and make sure \'gcc\' is available in PATH')

import argparse
import multiprocessing
import zipfile
import datetime
import errno

def silentremove(file):
	try:
	    os.remove(file)
	except OSError as e:
		if e.errno != errno.ENOENT:
			raise

def zip_with_msg(zipfile, source, target):
	if not os.path.exists(source):
		error('{} does not exist! Try build \'binary\' and \'apk\' before zipping!'.format(source))
	print('zip: ' + source + ' -> ' + target)
	zipfile.write(source, target)

def build_all(args):
	build_binary(args)
	build_apk(args)
	zip_main(args)
	zip_uninstaller(args)

def build_binary(args):
	header('* Building Magisk binaries')

	ndk_build = os.path.join(os.environ['ANDROID_HOME'], 'ndk-bundle', 'ndk-build')
	debug_flag = '' if args.release else '-DDEBUG'
	proc = subprocess.run('{} APP_CFLAGS=\"-DMAGISK_VERSION={} -DMAGISK_VER_CODE={} {}\" -j{}'.format(
		ndk_build, args.versionString, args.versionCode, debug_flag, multiprocessing.cpu_count()), shell=True)
	if proc.returncode != 0:
		error('Build Magisk binary failed!')

def build_apk(args):
	header('* Building Magisk Manager')

	os.chdir('MagiskManager')
	if args.release:
		if not os.path.exists(os.path.join('..', 'release_signature.jks')):
			error('Please generate a java keystore and place it in \'release_signature.jks\'')

		proc = subprocess.run('{} assembleRelease'.format(os.path.join('.', 'gradlew')), shell=True)
		if proc.returncode != 0:
			error('Build Magisk Manager failed!')

		unsigned = os.path.join('app', 'build', 'outputs', 'apk', 'app-release-unsigned.apk')
		aligned = os.path.join('app', 'build', 'outputs', 'apk', 'app-release-aligned.apk')
		release = os.path.join('app', 'build', 'outputs', 'apk', 'app-release.apk')

		# Find the latest build tools
		build_tool = sorted(os.listdir(os.path.join(os.environ['ANDROID_HOME'], 'build-tools')))[-1]

		silentremove(aligned)
		silentremove(release)

		proc = subprocess.run([
			os.path.join(os.environ['ANDROID_HOME'], 'build-tools', build_tool, 'zipalign'),
			'-v', '-p', '4', unsigned, aligned])
		if proc.returncode != 0:
			error('Zipalign Magisk Manager failed!')

		proc = subprocess.run('{} sign --ks {} --out {} {}'.format(
			os.path.join(os.environ['ANDROID_HOME'], 'build-tools', build_tool, 'apksigner'),
			os.path.join('..', 'release_signature.jks'),
			release, aligned), shell=True)
		if proc.returncode != 0:
			error('Release sign Magisk Manager failed!')

		silentremove(unsigned)
		silentremove(aligned)
	else:
		proc = subprocess.run('{} assembleDebug'.format(os.path.join('.', 'gradlew')), shell=True)
		if proc.returncode != 0:
			error('Build Magisk Manager failed!')

	# Return to upper directory
	os.chdir('..')

def sign_adjust_zip(unsigned, output):
	header('* Signing / Adjusting Zip')

	# Unsigned->signed
	proc = subprocess.run(['java', '-jar', os.path.join('ziptools', 'signapk.jar'), 
		os.path.join('ziptools', 'test.certificate.x509.pem'), 
		os.path.join('ziptools', 'test.key.pk8'), unsigned, 'tmp_signed.zip'])
	if proc.returncode != 0:
		error('First sign flashable zip failed!')

	if os.name != 'nt' and not os.path.exists(os.path.join('ziptools', 'zipadjust')):
		# Compile zipadjust
		proc = subprocess.run('gcc -o ziptools/zipadjust ziptools/src/*.c -lz', shell=True)
		if proc.returncode != 0:
			error('Build zipadjust failed!')

	# Adjust zip
	proc = subprocess.run([os.path.join('ziptools', 'zipadjust'), 'tmp_signed.zip', 'tmp_adjusted.zip'])
	if proc.returncode != 0:
		error('Adjust flashable zip failed!')

	# Adjusted -> output
	proc = subprocess.run(['java', '-jar', os.path.join('ziptools', 'minsignapk.jar'), 
		os.path.join('ziptools', 'test.certificate.x509.pem'), 
		os.path.join('ziptools', 'test.key.pk8'), 'tmp_adjusted.zip', output])
	if proc.returncode != 0:
		error('Second sign flashable zip failed!')

	# Cleanup
	silentremove(unsigned)
	silentremove('tmp_signed.zip')
	silentremove('tmp_adjusted.zip')

def zip_main(args):
	header('* Packing Flashable Zip')

	with zipfile.ZipFile('tmp_unsigned.zip', 'w', compression=zipfile.ZIP_DEFLATED) as zipf:
		# Compiled Binaries
		for lib_dir, zip_dir in [('arm64-v8a', 'arm64'), ('armeabi-v7a', 'arm'), ('x86', 'x86'), ('x86_64', 'x64')]:
			for binary in ['magisk', 'magiskboot']:
				source = os.path.join('libs', lib_dir, binary)
				target = os.path.join(zip_dir, binary)
				zip_with_msg(zipf, source, target)

		# APK
		source = os.path.join('MagiskManager', 'app', 'build', 'outputs', 'apk', 'app-release.apk' if args.release else 'app-debug.apk')
		target = os.path.join('common', 'magisk.apk')
		zip_with_msg(zipf, source, target)

		# Scripts
		source = os.path.join('scripts', 'flash_script.sh')
		with open(source, 'r') as flash_script:
			# Add version info into flash script
			update_binary = flash_script.read().replace(
				'MAGISK_VERSION_STUB', 'Magisk v{} Installer'.format(args.versionString))
			target = os.path.join('META-INF', 'com', 'google', 'android', 'update-binary')
			print('zip: ' + source + ' -> ' + target)
			zipf.writestr(target, update_binary)
		target = os.path.join('META-INF', 'com', 'google', 'android', 'updater-script')
		print('zip: ' + target)
		zipf.writestr(target, '#MAGISK\n')
		source = os.path.join('scripts', 'init.magisk.rc')
		target = os.path.join('common', 'init.magisk.rc')
		zip_with_msg(zipf, source, target)
		source = os.path.join('scripts', 'boot_patch.sh')
		target = os.path.join('common', 'boot_patch.sh')
		zip_with_msg(zipf, source, target)

		# Prebuilts
		for chromeos in ['futility', 'kernel_data_key.vbprivk', 'kernel.keyblock']:
			source = os.path.join('chromeos', chromeos)
			zip_with_msg(zipf, source, source)

		# End of zipping

	output = 'Magisk-v{}.zip'.format(args.versionString)
	sign_adjust_zip('tmp_unsigned.zip', output)

def zip_uninstaller(args):
	header('* Packing Uninstaller Zip')

	with zipfile.ZipFile('tmp_unsigned.zip', 'w', compression=zipfile.ZIP_DEFLATED) as zipf:
		# Compiled Binaries
		for lib_dir, zip_dir in [('arm64-v8a', 'arm64'), ('armeabi-v7a', 'arm'), ('x86', 'x86'), ('x86_64', 'x64')]:
			source = os.path.join('libs', lib_dir, 'magiskboot')
			target = os.path.join(zip_dir, 'magiskboot')
			zip_with_msg(zipf, source, target)

		source = os.path.join('scripts', 'magisk_uninstaller.sh')
		target = 'magisk_uninstaller.sh'
		zip_with_msg(zipf, source, target)

		source = os.path.join('scripts', 'uninstaller_loader.sh')
		target = os.path.join('META-INF', 'com', 'google', 'android', 'update-binary')
		zip_with_msg(zipf, source, target)

		target = os.path.join('META-INF', 'com', 'google', 'android', 'updater-script')
		print('zip: ' + target)
		zipf.writestr(target, '#MAGISK\n')

	output = 'Magisk-uninstaller-{}.zip'.format(datetime.datetime.now().strftime('%Y%m%d'))
	sign_adjust_zip('tmp_unsigned.zip', output)

def cleanup(args):
	if len(args.target) == 0:
		args.target = ['binary', 'apk', 'zip']

	if 'binary' in args.target:
		header('* Cleaning Magisk binaries')
		subprocess.run(os.path.join(os.environ['ANDROID_HOME'], 'ndk-bundle', 'ndk-build') + ' clean', shell=True)

	if 'apk' in args.target:
		header('* Cleaning Magisk Manager')
		os.chdir('MagiskManager')
		subprocess.run('{} clean'.format(os.path.join('.', 'gradlew')), shell=True)
		os.chdir('..')

	if 'zip' in args.target:
		header('* Cleaning created zip files')
		for f in os.listdir('.'):
			if '.zip' in f:
				print('rm {}'.format(f))
				silentremove(f)

parser = argparse.ArgumentParser(description='Magisk build script')
parser.add_argument('--release', action='store_true', help='compile Magisk for release')
subparsers = parser.add_subparsers(title='actions')

all_parser = subparsers.add_parser('all', help='build everything and create flashable zip with uninstaller')
all_parser.add_argument('versionString')
all_parser.add_argument('versionCode', type=int)
all_parser.set_defaults(func=build_all)

binary_parser = subparsers.add_parser('binary', help='build Magisk binaries')
binary_parser.add_argument('versionString')
binary_parser.add_argument('versionCode', type=int)
binary_parser.set_defaults(func=build_binary)

apk_parser = subparsers.add_parser('apk', help='build Magisk Manager APK')
apk_parser.set_defaults(func=build_apk)

zip_parser = subparsers.add_parser('zip', help='zip and sign Magisk into a flashable zip')
zip_parser.add_argument('versionString')
zip_parser.set_defaults(func=zip_main)

uninstaller_parser = subparsers.add_parser('uninstaller', help='create flashable uninstaller')
uninstaller_parser.set_defaults(func=zip_uninstaller)

clean_parser = subparsers.add_parser('clean', help='clean [target...] Targets: binary apk zip (default: all)')
clean_parser.add_argument('target', nargs='*')
clean_parser.set_defaults(func=cleanup)

if len(sys.argv) == 1:
    parser.print_help()
    sys.exit(1)

args = parser.parse_args()

args.func(args)
