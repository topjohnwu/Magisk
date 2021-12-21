#===----------------------------------------------------------------------===//
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===//

import importlib
import locale
import os
import platform
import re
import subprocess
import sys

from libcxx.util import executeCommand

class DefaultTargetInfo(object):
    def __init__(self, full_config):
        self.full_config = full_config

    def platform(self):
        return sys.platform.lower().strip()

    def add_locale_features(self, features):
        self.full_config.lit_config.warning(
            "No locales entry for target_system: %s" % self.platform())

    def add_cxx_compile_flags(self, flags): pass
    def add_cxx_link_flags(self, flags): pass
    def configure_env(self, env): pass
    def allow_cxxabi_link(self): return True
    def add_sanitizer_features(self, sanitizer_type, features): pass
    def use_lit_shell_default(self): return False


def test_locale(loc):
    assert loc is not None
    default_locale = locale.setlocale(locale.LC_ALL)
    try:
        locale.setlocale(locale.LC_ALL, loc)
        return True
    except locale.Error:
        return False
    finally:
        locale.setlocale(locale.LC_ALL, default_locale)


def add_common_locales(features, lit_config, is_windows=False):
    # A list of locales needed by the test-suite.
    # The list uses the canonical name for the locale used in the test-suite
    # TODO: On Linux ISO8859 *may* needs to hyphenated.
    locales = [
        ('en_US.UTF-8', 'English_United States.1252'),
        ('fr_FR.UTF-8', 'French_France.1252'),
        ('ru_RU.UTF-8', 'Russian_Russia.1251'),
        ('zh_CN.UTF-8', 'Chinese_China.936'),
        ('fr_CA.ISO8859-1', 'French_Canada.1252'),
        ('cs_CZ.ISO8859-2', 'Czech_Czech Republic.1250')
    ]
    for loc_id, windows_loc_name in locales:
        loc_name = windows_loc_name if is_windows else loc_id
        if test_locale(loc_name):
            features.add('locale.{0}'.format(loc_id))
        else:
            lit_config.warning('The locale {0} is not supported by '
                               'your platform. Some tests will be '
                               'unsupported.'.format(loc_name))


class DarwinLocalTI(DefaultTargetInfo):
    def __init__(self, full_config):
        super(DarwinLocalTI, self).__init__(full_config)

    def is_host_macosx(self):
        name = subprocess.check_output(['sw_vers', '-productName']).strip()
        return name == "Mac OS X"

    def get_macosx_version(self):
        assert self.is_host_macosx()
        version = subprocess.check_output(
            ['sw_vers', '-productVersion']).strip()
        version = re.sub(r'([0-9]+\.[0-9]+)(\..*)?', r'\1', version)
        return version

    def get_sdk_version(self, name):
        assert self.is_host_macosx()
        cmd = ['xcrun', '--sdk', name, '--show-sdk-path']
        try:
            out = subprocess.check_output(cmd).strip()
        except OSError:
            pass

        if not out:
            self.full_config.lit_config.fatal(
                    "cannot infer sdk version with: %r" % cmd)

        return re.sub(r'.*/[^0-9]+([0-9.]+)\.sdk', r'\1', out)

    def get_platform(self):
        platform = self.full_config.get_lit_conf('platform')
        if platform:
            platform = re.sub(r'([^0-9]+)([0-9\.]*)', r'\1-\2', platform)
            name, version = tuple(platform.split('-', 1))
        else:
            name = 'macosx'
            version = None

        if version:
            return (False, name, version)

        # Infer the version, either from the SDK or the system itself.  For
        # macosx, ignore the SDK version; what matters is what's at
        # /usr/lib/libc++.dylib.
        if name == 'macosx':
            version = self.get_macosx_version()
        else:
            version = self.get_sdk_version(name)
        return (True, name, version)

    def add_locale_features(self, features):
        add_common_locales(features, self.full_config.lit_config)

    def add_cxx_compile_flags(self, flags):
        if self.full_config.use_deployment:
            _, name, _ = self.full_config.config.deployment
            cmd = ['xcrun', '--sdk', name, '--show-sdk-path']
        else:
            cmd = ['xcrun', '--show-sdk-path']
        out, err, exit_code = executeCommand(cmd)
        if exit_code != 0:
            self.full_config.lit_config.warning("Could not determine macOS SDK path! stderr was " + err)
        if exit_code == 0 and out:
            sdk_path = out.strip()
            self.full_config.lit_config.note('using SDKROOT: %r' % sdk_path)
            assert isinstance(sdk_path, str)
            flags += ["-isysroot", sdk_path]

    def add_cxx_link_flags(self, flags):
        flags += ['-lSystem']

    def configure_env(self, env):
        library_paths = []
        # Configure the library path for libc++
        if self.full_config.cxx_runtime_root:
            library_paths += [self.full_config.cxx_runtime_root]
        elif self.full_config.use_system_cxx_lib:
            if (os.path.isdir(str(self.full_config.use_system_cxx_lib))):
                library_paths += [self.full_config.use_system_cxx_lib]

        # Configure the abi library path
        if self.full_config.abi_library_root:
            library_paths += [self.full_config.abi_library_root]
        if library_paths:
            env['DYLD_LIBRARY_PATH'] = ':'.join(library_paths)

    def allow_cxxabi_link(self):
        # FIXME: PR27405
        # libc++ *should* export all of the symbols found in libc++abi on OS X.
        # For this reason LibcxxConfiguration will not link libc++abi in OS X.
        # However __cxa_throw_bad_new_array_length doesn't get exported into
        # libc++ yet so we still need to explicitly link libc++abi when testing
        # libc++abi
        # See PR22654.
        if(self.full_config.get_lit_conf('name', '') == 'libc++abi'):
            return True
        # Don't link libc++abi explicitly on OS X because the symbols
        # should be available in libc++ directly.
        return False


class FreeBSDLocalTI(DefaultTargetInfo):
    def __init__(self, full_config):
        super(FreeBSDLocalTI, self).__init__(full_config)

    def add_locale_features(self, features):
        add_common_locales(features, self.full_config.lit_config)

    def add_cxx_link_flags(self, flags):
        flags += ['-lc', '-lm', '-lpthread', '-lgcc_s', '-lcxxrt']


class NetBSDLocalTI(DefaultTargetInfo):
    def __init__(self, full_config):
        super(NetBSDLocalTI, self).__init__(full_config)

    def add_locale_features(self, features):
        add_common_locales(features, self.full_config.lit_config)

    def add_cxx_link_flags(self, flags):
        flags += ['-lc', '-lm', '-lpthread', '-lgcc_s', '-lc++abi',
                  '-lunwind']


class LinuxLocalTI(DefaultTargetInfo):
    def __init__(self, full_config):
        super(LinuxLocalTI, self).__init__(full_config)

    def platform(self):
        return 'linux'

    def platform_name(self):
        name, _, _ = platform.linux_distribution()
        # Some distros have spaces, e.g. 'SUSE Linux Enterprise Server'
        # lit features can't have spaces
        name = name.lower().strip().replace(' ', '-')
        return name # Permitted to be None

    def platform_ver(self):
        _, ver, _ = platform.linux_distribution()
        ver = ver.lower().strip().replace(' ', '-')
        return ver # Permitted to be None.

    def add_locale_features(self, features):
        add_common_locales(features, self.full_config.lit_config)
        # Some linux distributions have different locale data than others.
        # Insert the distributions name and name-version into the available
        # features to allow tests to XFAIL on them.
        name = self.platform_name()
        ver = self.platform_ver()
        if name:
            features.add(name)
        if name and ver:
            features.add('%s-%s' % (name, ver))

    def add_cxx_compile_flags(self, flags):
        flags += ['-D__STDC_FORMAT_MACROS',
                  '-D__STDC_LIMIT_MACROS',
                  '-D__STDC_CONSTANT_MACROS']

    def add_cxx_link_flags(self, flags):
        enable_threads = ('libcpp-has-no-threads' not in
                          self.full_config.config.available_features)
        llvm_unwinder = self.full_config.get_lit_bool('llvm_unwinder', False)
        shared_libcxx = self.full_config.get_lit_bool('enable_shared', True)
        # FIXME: Remove the need to link -lrt in all the tests, and instead
        # limit it only to the filesystem tests. This ensures we don't cause an
        # implicit dependency on librt except when filesystem is needed.
        enable_fs = self.full_config.get_lit_bool('enable_filesystem',
                                                  default=False)
        flags += ['-lm']
        if not llvm_unwinder:
            flags += ['-lgcc_s', '-lgcc']
        if enable_threads:
            flags += ['-lpthread']
            if not shared_libcxx or enable_fs:
              flags += ['-lrt']
        flags += ['-lc']
        if llvm_unwinder:
            flags += ['-lunwind', '-ldl']
        else:
            flags += ['-lgcc_s']
        compiler_rt = self.full_config.get_lit_bool('compiler_rt', False)
        if not compiler_rt:
            flags += ['-lgcc']
        use_libatomic = self.full_config.get_lit_bool('use_libatomic', False)
        if use_libatomic:
            flags += ['-latomic']
        san = self.full_config.get_lit_conf('use_sanitizer', '').strip()
        if san:
            # The libraries and their order are taken from the
            # linkSanitizerRuntimeDeps function in
            # clang/lib/Driver/Tools.cpp
            flags += ['-lpthread', '-lrt', '-lm', '-ldl']


class WindowsLocalTI(DefaultTargetInfo):
    def __init__(self, full_config):
        super(WindowsLocalTI, self).__init__(full_config)

    def add_locale_features(self, features):
        add_common_locales(features, self.full_config.lit_config,
                           is_windows=True)

    def use_lit_shell_default(self):
        # Default to the internal shell on Windows, as bash on Windows is
        # usually very slow.
        return True


def make_target_info(full_config):
    default = "libcxx.test.target_info.LocalTI"
    info_str = full_config.get_lit_conf('target_info', default)
    if info_str != default:
        mod_path, _, info = info_str.rpartition('.')
        mod = importlib.import_module(mod_path)
        target_info = getattr(mod, info)(full_config)
        full_config.lit_config.note("inferred target_info as: %r" % info_str)
        return target_info
    target_system = platform.system()
    if target_system == 'Darwin':  return DarwinLocalTI(full_config)
    if target_system == 'FreeBSD': return FreeBSDLocalTI(full_config)
    if target_system == 'NetBSD':  return NetBSDLocalTI(full_config)
    if target_system == 'Linux':   return LinuxLocalTI(full_config)
    if target_system == 'Windows': return WindowsLocalTI(full_config)
    return DefaultTargetInfo(full_config)
