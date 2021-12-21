import os

import libcxx.test.config
import libcxx.android.test.format


class Configuration(libcxx.test.config.Configuration):
    def __init__(self, lit_config, config):
        super(Configuration, self).__init__(lit_config, config)
        self.cxx_under_test = None
        self.cxx_template = None
        self.link_template = None

    def configure(self):
        self.configure_cxx()
        self.configure_triple()
        self.configure_src_root()
        self.configure_obj_root()
        self.configure_cxx_library_root()
        self.configure_compile_flags()
        self.configure_link_flags()
        self.configure_features()

    def configure_link_flags(self):
        self.link_flags.append('-nodefaultlibs')

        # Configure libc++ library paths.
        self.link_flags.append('-L' + self.cxx_library_root)

        # Add libc_ndk's output path to the library search paths.
        libdir = '{}/obj/STATIC_LIBRARIES/libc_ndk_intermediates'.format(
            os.getenv('ANDROID_PRODUCT_OUT'))
        self.link_flags.append('-L' + libdir)

        self.link_flags.append('-lc++_ndk')
        self.link_flags.append('-lc_ndk')
        self.link_flags.append('-lc')

    def configure_features(self):
        self.config.available_features.add('long_tests')

    def get_test_format(self):
        cxx_template = ' '.join(
            self.compile_flags + ['-c', '-o', '%OUT%', '%SOURCE%'])
        link_template = ' '.join(
            ['-o', '%OUT%', '%SOURCE%'] + self.compile_flags + self.link_flags)
        tmp_dir = getattr(self.config, 'device_dir', '/data/local/tmp/')

        return libcxx.android.test.format.TestFormat(
            self.cxx,
            self.libcxx_src_root,
            self.obj_root,
            cxx_template,
            link_template,
            tmp_dir,
            getattr(self.config, 'timeout', '300'),
            exec_env={'LD_LIBRARY_PATH': tmp_dir})
