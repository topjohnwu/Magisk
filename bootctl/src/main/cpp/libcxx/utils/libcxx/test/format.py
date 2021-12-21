#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##

import copy
import errno
import os
import time
import random

import lit.Test        # pylint: disable=import-error
import lit.TestRunner  # pylint: disable=import-error
from lit.TestRunner import ParserKind, IntegratedTestKeywordParser  \
    # pylint: disable=import-error

from libcxx.test.executor import LocalExecutor as LocalExecutor
import libcxx.util


class LibcxxTestFormat(object):
    """
    Custom test format handler for use with the test format use by libc++.

    Tests fall into two categories:
      FOO.pass.cpp - Executable test which should compile, run, and exit with
                     code 0.
      FOO.fail.cpp - Negative test case which is expected to fail compilation.
      FOO.sh.cpp   - A test that uses LIT's ShTest format.
    """

    def __init__(self, cxx, use_verify_for_fail, execute_external,
                 executor, exec_env):
        self.cxx = copy.deepcopy(cxx)
        self.use_verify_for_fail = use_verify_for_fail
        self.execute_external = execute_external
        self.executor = executor
        self.exec_env = dict(exec_env)

    @staticmethod
    def _make_custom_parsers():
        return [
            IntegratedTestKeywordParser('FLAKY_TEST.', ParserKind.TAG,
                                        initial_value=False),
            IntegratedTestKeywordParser('MODULES_DEFINES:', ParserKind.LIST,
                                        initial_value=[])
        ]

    @staticmethod
    def _get_parser(key, parsers):
        for p in parsers:
            if p.keyword == key:
                return p
        assert False and "parser not found"

    # TODO: Move this into lit's FileBasedTest
    def getTestsInDirectory(self, testSuite, path_in_suite,
                            litConfig, localConfig):
        source_path = testSuite.getSourcePath(path_in_suite)
        for filename in os.listdir(source_path):
            # Ignore dot files and excluded tests.
            if filename.startswith('.') or filename in localConfig.excludes:
                continue

            filepath = os.path.join(source_path, filename)
            if not os.path.isdir(filepath):
                if any([filename.endswith(ext)
                        for ext in localConfig.suffixes]):
                    yield lit.Test.Test(testSuite, path_in_suite + (filename,),
                                        localConfig)

    def execute(self, test, lit_config):
        while True:
            try:
                return self._execute(test, lit_config)
            except OSError as oe:
                if oe.errno != errno.ETXTBSY:
                    raise
                time.sleep(0.1)

    def _execute(self, test, lit_config):
        name = test.path_in_suite[-1]
        name_root, name_ext = os.path.splitext(name)
        is_libcxx_test = test.path_in_suite[0] == 'libcxx'
        is_sh_test = name_root.endswith('.sh')
        is_pass_test = name.endswith('.pass.cpp') or name.endswith('.pass.mm')
        is_fail_test = name.endswith('.fail.cpp') or name.endswith('.fail.mm')
        is_objcxx_test = name.endswith('.mm')
        is_objcxx_arc_test = name.endswith('.arc.pass.mm') or \
                             name.endswith('.arc.fail.mm')
        assert is_sh_test or name_ext == '.cpp' or name_ext == '.mm', \
            'non-cpp file must be sh test'

        if test.config.unsupported:
            return (lit.Test.UNSUPPORTED,
                    "A lit.local.cfg marked this unsupported")

        if is_objcxx_test and not \
           'objective-c++' in test.config.available_features:
            return (lit.Test.UNSUPPORTED, "Objective-C++ is not supported")

        parsers = self._make_custom_parsers()
        script = lit.TestRunner.parseIntegratedTestScript(
            test, additional_parsers=parsers, require_script=is_sh_test)
        # Check if a result for the test was returned. If so return that
        # result.
        if isinstance(script, lit.Test.Result):
            return script
        if lit_config.noExecute:
            return lit.Test.Result(lit.Test.PASS)

        # Check that we don't have run lines on tests that don't support them.
        if not is_sh_test and len(script) != 0:
            lit_config.fatal('Unsupported RUN line found in test %s' % name)

        tmpDir, tmpBase = lit.TestRunner.getTempPaths(test)
        substitutions = lit.TestRunner.getDefaultSubstitutions(test, tmpDir,
                                                               tmpBase)
        script = lit.TestRunner.applySubstitutions(script, substitutions)

        test_cxx = copy.deepcopy(self.cxx)
        if is_fail_test:
            test_cxx.useCCache(False)
            test_cxx.useWarnings(False)
        extra_modules_defines = self._get_parser('MODULES_DEFINES:',
                                                 parsers).getValue()
        if '-fmodules' in test.config.available_features:
            test_cxx.compile_flags += [('-D%s' % mdef.strip()) for
                                       mdef in extra_modules_defines]
            test_cxx.addWarningFlagIfSupported('-Wno-macro-redefined')
            # FIXME: libc++ debug tests #define _LIBCPP_ASSERT to override it
            # If we see this we need to build the test against uniquely built
            # modules.
            if is_libcxx_test:
                with open(test.getSourcePath(), 'r') as f:
                    contents = f.read()
                if '#define _LIBCPP_ASSERT' in contents:
                    test_cxx.useModules(False)

        if is_objcxx_test:
            test_cxx.source_lang = 'objective-c++'
            if is_objcxx_arc_test:
                test_cxx.compile_flags += ['-fobjc-arc']
            else:
                test_cxx.compile_flags += ['-fno-objc-arc']
            test_cxx.link_flags += ['-framework', 'Foundation']

        # Dispatch the test based on its suffix.
        if is_sh_test:
            if not isinstance(self.executor, LocalExecutor):
                # We can't run ShTest tests with a executor yet.
                # For now, bail on trying to run them
                return lit.Test.UNSUPPORTED, 'ShTest format not yet supported'
            test.config.environment = dict(self.exec_env)
            return lit.TestRunner._runShTest(test, lit_config,
                                             self.execute_external, script,
                                             tmpBase)
        elif is_fail_test:
            return self._evaluate_fail_test(test, test_cxx, parsers)
        elif is_pass_test:
            return self._evaluate_pass_test(test, tmpBase, lit_config,
                                            test_cxx, parsers)
        else:
            # No other test type is supported
            assert False

    def _clean(self, exec_path):  # pylint: disable=no-self-use
        libcxx.util.cleanFile(exec_path)

    def _evaluate_pass_test(self, test, tmpBase, lit_config,
                            test_cxx, parsers):
        execDir = os.path.dirname(test.getExecPath())
        source_path = test.getSourcePath()
        exec_path = tmpBase + '.exe'
        object_path = tmpBase + '.o'
        # Create the output directory if it does not already exist.
        libcxx.util.mkdir_p(os.path.dirname(tmpBase))
        try:
            # Compile the test
            cmd, out, err, rc = test_cxx.compileLinkTwoSteps(
                source_path, out=exec_path, object_file=object_path,
                cwd=execDir)
            compile_cmd = cmd
            if rc != 0:
                report = libcxx.util.makeReport(cmd, out, err, rc)
                report += "Compilation failed unexpectedly!"
                return lit.Test.Result(lit.Test.FAIL, report)
            # Run the test
            local_cwd = os.path.dirname(source_path)
            env = None
            if self.exec_env:
                env = self.exec_env
            # TODO: Only list actually needed files in file_deps.
            # Right now we just mark all of the .dat files in the same
            # directory as dependencies, but it's likely less than that. We
            # should add a `// FILE-DEP: foo.dat` to each test to track this.
            data_files = [os.path.join(local_cwd, f)
                          for f in os.listdir(local_cwd) if f.endswith('.dat')]
            is_flaky = self._get_parser('FLAKY_TEST.', parsers).getValue()
            max_retry = 3 if is_flaky else 1
            for retry_count in range(max_retry):
                cmd, out, err, rc = self.executor.run(exec_path, [exec_path],
                                                      local_cwd, data_files,
                                                      env)
                report = "Compiled With: '%s'\n" % ' '.join(compile_cmd)
                report += libcxx.util.makeReport(cmd, out, err, rc)
                if rc == 0:
                    res = lit.Test.PASS if retry_count == 0 else lit.Test.FLAKYPASS
                    return lit.Test.Result(res, report)
                elif rc != 0 and retry_count + 1 == max_retry:
                    report += "Compiled test failed unexpectedly!"
                    return lit.Test.Result(lit.Test.FAIL, report)

            assert False # Unreachable
        finally:
            # Note that cleanup of exec_file happens in `_clean()`. If you
            # override this, cleanup is your reponsibility.
            libcxx.util.cleanFile(object_path)
            self._clean(exec_path)

    def _evaluate_fail_test(self, test, test_cxx, parsers):
        source_path = test.getSourcePath()
        # FIXME: lift this detection into LLVM/LIT.
        with open(source_path, 'r') as f:
            contents = f.read()
        verify_tags = ['expected-note', 'expected-remark', 'expected-warning',
                       'expected-error', 'expected-no-diagnostics']
        use_verify = self.use_verify_for_fail and \
                     any([tag in contents for tag in verify_tags])
        # FIXME(EricWF): GCC 5 does not evaluate static assertions that
        # are dependant on a template parameter when '-fsyntax-only' is passed.
        # This is fixed in GCC 6. However for now we only pass "-fsyntax-only"
        # when using Clang.
        if test_cxx.type != 'gcc':
            test_cxx.flags += ['-fsyntax-only']
        if use_verify:
            test_cxx.useVerify()
            test_cxx.useWarnings()
            if '-Wuser-defined-warnings' in test_cxx.warning_flags:
                test_cxx.warning_flags += ['-Wno-error=user-defined-warnings']
        else:
            # We still need to enable certain warnings on .fail.cpp test when
            # -verify isn't enabled. Such as -Werror=unused-result. However,
            # we don't want it enabled too liberally, which might incorrectly
            # allow unrelated failure tests to 'pass'.
            #
            # Therefore, we check if the test was expected to fail because of
            # nodiscard before enabling it
            test_str_list = ['ignoring return value', 'nodiscard', 'NODISCARD']
            if any(test_str in contents for test_str in test_str_list):
                test_cxx.flags += ['-Werror=unused-result']
        cmd, out, err, rc = test_cxx.compile(source_path, out=os.devnull)
        expected_rc = 0 if use_verify else 1
        report = libcxx.util.makeReport(cmd, out, err, rc)
        if rc == expected_rc:
            return lit.Test.Result(lit.Test.PASS, report)
        else:
            report += ('Expected compilation to fail!\n' if not use_verify else
                       'Expected compilation using verify to pass!\n')
            return lit.Test.Result(lit.Test.FAIL, report)
