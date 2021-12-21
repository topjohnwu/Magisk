import os
import re
import sys

import libcxx.test.config
import libcxx.android.compiler
import libcxx.android.test.format


class Configuration(libcxx.test.config.Configuration):
    def __init__(self, lit_config, config):
        super(Configuration, self).__init__(lit_config, config)
        self.exec_env = {}

    @property
    def is_host(self):
        """Returns True if configured to run host tests."""
        return self.lit_config.params.get('android_mode') == 'host'

    def configure(self):
        self.configure_src_root()
        self.configure_obj_root()

        self.configure_cxx()
        self.configure_triple()
        self.configure_features()
        if self.is_host:
            self.configure_filesystem_host()
        else:
            self.configure_filesystem_device()

    def print_config_info(self):
        self.lit_config.note(
            'Using compiler: {}'.format(self.cxx.path))
        self.lit_config.note(
            'Using compile template: {}'.format(self.cxx.cxx_template))
        self.lit_config.note(
            'Using link template: {}'.format(self.cxx.link_template))
        self.lit_config.note('Using available_features: %s' %
                             list(self.config.available_features))

    def configure_obj_root(self):
        if self.is_host:
            self.libcxx_obj_root = os.getenv('ANDROID_HOST_OUT')
        else:
            self.libcxx_obj_root = os.getenv('ANDROID_PRODUCT_OUT')

    def configure_cxx(self):
        cxx_under_test = self.lit_config.params.get('cxx_under_test')
        cxx_template = self.lit_config.params.get('cxx_template')
        link_template = self.lit_config.params.get('link_template')

        self.cxx = libcxx.android.compiler.AndroidCXXCompiler(
            cxx_under_test, cxx_template, link_template)

    def configure_triple(self):
        # The libcxxabi test suite needs this but it doesn't actually
        # use it for anything important.
        self.config.host_triple = ''

        self.config.target_triple = self.cxx.get_triple()

    def configure_filesystem_host(self):
        # TODO: We shouldn't be writing to the source directory for the host.
        static_env = os.path.join(self.libcxx_src_root, 'test', 'std',
                                  'input.output', 'filesystems', 'Inputs',
                                  'static_test_env')
        static_env = os.path.realpath(static_env)
        assert os.path.isdir(static_env)
        self.cxx.extra_cflags.append(
            '-DLIBCXX_FILESYSTEM_STATIC_TEST_ROOT="{}"'.format(static_env))

        dynamic_env = os.path.join(self.config.test_exec_root, 'filesystem',
                                   'Output', 'dynamic_env')
        dynamic_env = os.path.realpath(dynamic_env)
        if not os.path.isdir(dynamic_env):
            os.makedirs(dynamic_env)
        self.cxx.extra_cflags.append(
            '-DLIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT="{}"'.format(dynamic_env))
        self.exec_env['LIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT'] = dynamic_env

        dynamic_helper = os.path.join(self.libcxx_src_root, 'test', 'support',
                                      'filesystem_dynamic_test_helper.py')
        assert os.path.isfile(dynamic_helper)
        self.cxx.extra_cflags.append(
            '-DLIBCXX_FILESYSTEM_DYNAMIC_TEST_HELPER="{} {}"'.format(
                sys.executable, dynamic_helper))

    def configure_filesystem_device(self):
        static_env = '/data/local/tmp/libcxx/static_test_env'
        self.cxx.extra_cflags.append(
            '-DLIBCXX_FILESYSTEM_STATIC_TEST_ROOT="{}"'.format(static_env))

        dynamic_env = '/data/local/tmp/libcxx/dynamic_test_env'
        self.cxx.extra_cflags.append(
            '-DLIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT="{}"'.format(dynamic_env))
        self.exec_env['LIBCXX_FILESYSTEM_DYNAMIC_TEST_ROOT'] = dynamic_env

        dynamic_helper = ('/data/nativetest/filesystem_dynamic_test_helper.py/'
                          'filesystem_dynamic_test_helper.py')
        self.cxx.extra_cflags.append(
            '-DLIBCXX_FILESYSTEM_DYNAMIC_TEST_HELPER="{}"'.format(
                dynamic_helper))

    def configure_features(self):
        self.config.available_features.add('long_tests')
        self.config.available_features.add('c++experimental')
        self.config.available_features.add('c++fs')
        self.config.available_features.add('c++filesystem')
        self.config.available_features.add('fcoroutines-ts')
        std_pattern = re.compile(r'-std=(c\+\+\d[0-9x-z])')
        match = std_pattern.search(self.cxx.cxx_template)
        if match:
            self.config.available_features.add(match.group(1))

    def get_test_format(self):
        mode = self.lit_config.params.get('android_mode', 'device')
        if mode == 'device':
            return libcxx.android.test.format.TestFormat(
                self.cxx,
                self.libcxx_src_root,
                self.libcxx_obj_root,
                getattr(self.config, 'device_dir', '/data/local/tmp/'),
                getattr(self.config, 'timeout', '60'),
                self.exec_env)
        elif mode == 'host':
            return libcxx.android.test.format.HostTestFormat(
                self.cxx,
                self.libcxx_src_root,
                self.libcxx_obj_root,
                getattr(self.config, 'timeout', '60'),
                self.exec_env)
        else:
            raise RuntimeError('Invalid android_mode: {}'.format(mode))
