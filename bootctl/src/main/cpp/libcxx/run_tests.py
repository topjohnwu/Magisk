#!/usr/bin/env python
#
# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
"""Runs the libc++ tests against the platform libc++."""
from __future__ import print_function

import argparse
import logging
import os
import posixpath
import sys

THIS_DIR = os.path.dirname(os.path.realpath(__file__))
ANDROID_DIR = os.path.realpath(os.path.join(THIS_DIR, '../..'))


def logger():
    """Returns the logger for the module."""
    return logging.getLogger(__name__)


def call(cmd, *args, **kwargs):
    """subprocess.call with logging."""
    import subprocess
    logger().info('call %s', ' '.join(cmd))
    return subprocess.call(cmd, *args, **kwargs)


def check_call(cmd, *args, **kwargs):
    """subprocess.check_call with logging."""
    import subprocess
    logger().info('check_call %s', ' '.join(cmd))
    return subprocess.check_call(cmd, *args, **kwargs)


def check_output(cmd, *args, **kwargs):
    """subprocess.check_output with logging."""
    import subprocess
    logger().info('check_output %s', ' '.join(cmd))
    return subprocess.check_output(cmd, *args, **kwargs)


class ArgParser(argparse.ArgumentParser):
    """Parses command line arguments."""

    def __init__(self):
        super(ArgParser, self).__init__()
        self.add_argument('--bitness', choices=(32, 64), type=int, default=32)
        self.add_argument('--host', action='store_true')


def extract_build_cmds(commands, exe_name):
    """Extracts build command information from `ninja -t commands` output.

    Args:
        commands: String containing the output of `ninja -t commands` for the
            libcxx_test_template.
        exe_name: The basename of the built executable.

    Returns:
        Tuple of (compiler, compiler_flags, linker_flags).
    """
    cc = None
    cflags = None
    ldflags = None
    template_name = 'external/libcxx/libcxx_test_template.cpp'

    for cmd in commands.splitlines():
        cmd_args = cmd.split()
        if cc is None and template_name in cmd_args:
            for i, arg in enumerate(cmd_args):
                if arg == '-o':
                    cmd_args[i + 1] = '%OUT%'
                elif arg == template_name:
                    cmd_args[i] = '%SOURCE%'
                # Drop dependency tracking args since they can cause file
                # not found errors at test time.
                if arg == '-MD':
                    cmd_args[i] = ''
                if arg == '-MF':
                    cmd_args[i] = ''
                    cmd_args[i + 1] = ''
            if cmd_args[0] == 'PWD=/proc/self/cwd':
                cmd_args = cmd_args[1:]
            if cmd_args[0].endswith('gomacc'):
                cmd_args = cmd_args[1:]
            cc = cmd_args[0]
            cflags = cmd_args[1:]
        if ldflags is None:
            is_ld = False
            for i, arg in enumerate(cmd_args):
                # Here we assume that the rspfile contains the path to the
                # object file and nothing else.
                if arg.startswith('@'):
                    cmd_args[i] = '%SOURCE%'
                if arg == '-o' and cmd_args[i + 1].endswith(exe_name):
                    cmd_args[i + 1] = '%OUT%'
                    is_ld = True
            if is_ld:
                ldflags = cmd_args[1:]

    return cc, cflags, ldflags


def get_build_cmds(bitness, host):
    """Use ninja -t commands to find the build commands for an executable."""
    out_dir = os.getenv('OUT_DIR', os.path.join(ANDROID_DIR, 'out'))
    product_out = os.getenv('ANDROID_PRODUCT_OUT')

    if host:
        rel_out_dir = os.path.relpath(
            os.path.join(out_dir, 'soong/host/linux-x86/bin'), ANDROID_DIR)
        target = os.path.join(rel_out_dir, 'libcxx_test_template64')
    else:
        exe_name = 'libcxx_test_template' + str(bitness)
        rel_out_dir = os.path.relpath(product_out, ANDROID_DIR)
        target = os.path.join(rel_out_dir, 'system/bin', exe_name)

    # Generate $OUT_DIR/combined-$TARGET_PRODUCT.ninja and build the
    # template target's dependencies.
    check_call([
        'bash',
        os.path.join(ANDROID_DIR, 'build/soong/soong_ui.bash'), '--make-mode',
        target
    ])

    ninja_path = os.path.join(
        out_dir, 'combined-' + os.getenv('TARGET_PRODUCT') + '.ninja')
    commands = check_output([
        os.path.join(ANDROID_DIR, 'prebuilts/build-tools/linux-x86/bin/ninja'),
        '-C', ANDROID_DIR, '-f', ninja_path, '-t', 'commands', target
    ])

    return extract_build_cmds(commands, os.path.basename(target))


def setup_test_directory():
    """Prepares a device test directory for use by the shell user."""
    stdfs_test_data = os.path.join(
        THIS_DIR, 'test/std/input.output/filesystems/Inputs/static_test_env')
    device_dir = '/data/local/tmp/libcxx'
    dynamic_dir = posixpath.join(device_dir, 'dynamic_test_env')
    check_call(['adb', 'shell', 'rm', '-rf', device_dir])
    check_call(['adb', 'shell', 'mkdir', '-p', device_dir])
    check_call(['adb', 'shell', 'mkdir', '-p', dynamic_dir])
    check_call(['adb', 'push', '--sync', stdfs_test_data, device_dir])
    check_call(['adb', 'shell', 'chown', '-R', 'shell:shell', device_dir])


def main():
    """Program entry point."""
    logging.basicConfig(level=logging.INFO)

    args, lit_args = ArgParser().parse_known_args()
    lit_path = os.path.join(ANDROID_DIR, 'external/llvm/utils/lit/lit.py')
    cc, cflags, ldflags = get_build_cmds(args.bitness, args.host)

    mode_str = 'host' if args.host else 'device'
    android_mode_arg = '--param=android_mode=' + mode_str
    cxx_under_test_arg = '--param=cxx_under_test=' + cc
    cxx_template_arg = '--param=cxx_template=' + ' '.join(cflags)
    link_template_arg = '--param=link_template=' + ' '.join(ldflags)
    site_cfg_path = os.path.join(THIS_DIR, 'test/lit.site.cfg')
    libcxx_site_cfg_arg = '--param=libcxx_site_config=' + site_cfg_path
    libcxxabi_site_cfg_arg = '--param=libcxxabi_site_config=' + site_cfg_path
    default_test_paths = [
        os.path.join(THIS_DIR, 'test'),
        os.path.join(ANDROID_DIR, 'external/libcxxabi/test')
    ]

    have_filter_args = False
    for arg in lit_args:
        # If the argument is a valid path with default_test_paths, it is a test
        # filter.
        real_path = os.path.realpath(arg)
        if not any(real_path.startswith(path) for path in default_test_paths):
            continue
        if not os.path.exists(real_path):
            continue

        have_filter_args = True
        break  # No need to keep scanning.

    if not args.host:
        setup_test_directory()

    lit_args = [
        '-sv', android_mode_arg, cxx_under_test_arg, cxx_template_arg,
        link_template_arg, libcxx_site_cfg_arg, libcxxabi_site_cfg_arg
    ] + lit_args
    cmd = ['python', lit_path] + lit_args
    if not have_filter_args:
        cmd += default_test_paths
    sys.exit(call(cmd))


if __name__ == '__main__':
    main()
