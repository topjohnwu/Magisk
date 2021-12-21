#!/usr/bin/env python

import unittest
"""
compare.py - versatile benchmark output compare tool
"""

import argparse
from argparse import ArgumentParser
import sys
import gbench
from gbench import util, report
from gbench.util import *


def check_inputs(in1, in2, flags):
    """
    Perform checking on the user provided inputs and diagnose any abnormalities
    """
    in1_kind, in1_err = classify_input_file(in1)
    in2_kind, in2_err = classify_input_file(in2)
    output_file = find_benchmark_flag('--benchmark_out=', flags)
    output_type = find_benchmark_flag('--benchmark_out_format=', flags)
    if in1_kind == IT_Executable and in2_kind == IT_Executable and output_file:
        print(("WARNING: '--benchmark_out=%s' will be passed to both "
               "benchmarks causing it to be overwritten") % output_file)
    if in1_kind == IT_JSON and in2_kind == IT_JSON and len(flags) > 0:
        print("WARNING: passing optional flags has no effect since both "
              "inputs are JSON")
    if output_type is not None and output_type != 'json':
        print(("ERROR: passing '--benchmark_out_format=%s' to 'compare.py`"
               " is not supported.") % output_type)
        sys.exit(1)


def create_parser():
    parser = ArgumentParser(
        description='versatile benchmark output compare tool')

    parser.add_argument(
        '-a',
        '--display_aggregates_only',
        dest='display_aggregates_only',
        action="store_true",
        help="If there are repetitions, by default, we display everything - the"
             " actual runs, and the aggregates computed. Sometimes, it is "
             "desirable to only view the aggregates. E.g. when there are a lot "
             "of repetitions. Do note that only the display is affected. "
             "Internally, all the actual runs are still used, e.g. for U test.")

    utest = parser.add_argument_group()
    utest.add_argument(
        '--no-utest',
        dest='utest',
        default=True,
        action="store_false",
        help="The tool can do a two-tailed Mann-Whitney U test with the null hypothesis that it is equally likely that a randomly selected value from one sample will be less than or greater than a randomly selected value from a second sample.\nWARNING: requires **LARGE** (no less than {}) number of repetitions to be meaningful!\nThe test is being done by default, if at least {} repetitions were done.\nThis option can disable the U Test.".format(report.UTEST_OPTIMAL_REPETITIONS, report.UTEST_MIN_REPETITIONS))
    alpha_default = 0.05
    utest.add_argument(
        "--alpha",
        dest='utest_alpha',
        default=alpha_default,
        type=float,
        help=("significance level alpha. if the calculated p-value is below this value, then the result is said to be statistically significant and the null hypothesis is rejected.\n(default: %0.4f)") %
        alpha_default)

    subparsers = parser.add_subparsers(
        help='This tool has multiple modes of operation:',
        dest='mode')

    parser_a = subparsers.add_parser(
        'benchmarks',
        help='The most simple use-case, compare all the output of these two benchmarks')
    baseline = parser_a.add_argument_group(
        'baseline', 'The benchmark baseline')
    baseline.add_argument(
        'test_baseline',
        metavar='test_baseline',
        type=argparse.FileType('r'),
        nargs=1,
        help='A benchmark executable or JSON output file')
    contender = parser_a.add_argument_group(
        'contender', 'The benchmark that will be compared against the baseline')
    contender.add_argument(
        'test_contender',
        metavar='test_contender',
        type=argparse.FileType('r'),
        nargs=1,
        help='A benchmark executable or JSON output file')
    parser_a.add_argument(
        'benchmark_options',
        metavar='benchmark_options',
        nargs=argparse.REMAINDER,
        help='Arguments to pass when running benchmark executables')

    parser_b = subparsers.add_parser(
        'filters', help='Compare filter one with the filter two of benchmark')
    baseline = parser_b.add_argument_group(
        'baseline', 'The benchmark baseline')
    baseline.add_argument(
        'test',
        metavar='test',
        type=argparse.FileType('r'),
        nargs=1,
        help='A benchmark executable or JSON output file')
    baseline.add_argument(
        'filter_baseline',
        metavar='filter_baseline',
        type=str,
        nargs=1,
        help='The first filter, that will be used as baseline')
    contender = parser_b.add_argument_group(
        'contender', 'The benchmark that will be compared against the baseline')
    contender.add_argument(
        'filter_contender',
        metavar='filter_contender',
        type=str,
        nargs=1,
        help='The second filter, that will be compared against the baseline')
    parser_b.add_argument(
        'benchmark_options',
        metavar='benchmark_options',
        nargs=argparse.REMAINDER,
        help='Arguments to pass when running benchmark executables')

    parser_c = subparsers.add_parser(
        'benchmarksfiltered',
        help='Compare filter one of first benchmark with filter two of the second benchmark')
    baseline = parser_c.add_argument_group(
        'baseline', 'The benchmark baseline')
    baseline.add_argument(
        'test_baseline',
        metavar='test_baseline',
        type=argparse.FileType('r'),
        nargs=1,
        help='A benchmark executable or JSON output file')
    baseline.add_argument(
        'filter_baseline',
        metavar='filter_baseline',
        type=str,
        nargs=1,
        help='The first filter, that will be used as baseline')
    contender = parser_c.add_argument_group(
        'contender', 'The benchmark that will be compared against the baseline')
    contender.add_argument(
        'test_contender',
        metavar='test_contender',
        type=argparse.FileType('r'),
        nargs=1,
        help='The second benchmark executable or JSON output file, that will be compared against the baseline')
    contender.add_argument(
        'filter_contender',
        metavar='filter_contender',
        type=str,
        nargs=1,
        help='The second filter, that will be compared against the baseline')
    parser_c.add_argument(
        'benchmark_options',
        metavar='benchmark_options',
        nargs=argparse.REMAINDER,
        help='Arguments to pass when running benchmark executables')

    return parser


def main():
    # Parse the command line flags
    parser = create_parser()
    args, unknown_args = parser.parse_known_args()
    if args.mode is None:
        parser.print_help()
        exit(1)
    assert not unknown_args
    benchmark_options = args.benchmark_options

    if args.mode == 'benchmarks':
        test_baseline = args.test_baseline[0].name
        test_contender = args.test_contender[0].name
        filter_baseline = ''
        filter_contender = ''

        # NOTE: if test_baseline == test_contender, you are analyzing the stdev

        description = 'Comparing %s to %s' % (test_baseline, test_contender)
    elif args.mode == 'filters':
        test_baseline = args.test[0].name
        test_contender = args.test[0].name
        filter_baseline = args.filter_baseline[0]
        filter_contender = args.filter_contender[0]

        # NOTE: if filter_baseline == filter_contender, you are analyzing the
        # stdev

        description = 'Comparing %s to %s (from %s)' % (
            filter_baseline, filter_contender, args.test[0].name)
    elif args.mode == 'benchmarksfiltered':
        test_baseline = args.test_baseline[0].name
        test_contender = args.test_contender[0].name
        filter_baseline = args.filter_baseline[0]
        filter_contender = args.filter_contender[0]

        # NOTE: if test_baseline == test_contender and
        # filter_baseline == filter_contender, you are analyzing the stdev

        description = 'Comparing %s (from %s) to %s (from %s)' % (
            filter_baseline, test_baseline, filter_contender, test_contender)
    else:
        # should never happen
        print("Unrecognized mode of operation: '%s'" % args.mode)
        parser.print_help()
        exit(1)

    check_inputs(test_baseline, test_contender, benchmark_options)

    if args.display_aggregates_only:
        benchmark_options += ['--benchmark_display_aggregates_only=true']

    options_baseline = []
    options_contender = []

    if filter_baseline and filter_contender:
        options_baseline = ['--benchmark_filter=%s' % filter_baseline]
        options_contender = ['--benchmark_filter=%s' % filter_contender]

    # Run the benchmarks and report the results
    json1 = json1_orig = gbench.util.run_or_load_benchmark(
        test_baseline, benchmark_options + options_baseline)
    json2 = json2_orig = gbench.util.run_or_load_benchmark(
        test_contender, benchmark_options + options_contender)

    # Now, filter the benchmarks so that the difference report can work
    if filter_baseline and filter_contender:
        replacement = '[%s vs. %s]' % (filter_baseline, filter_contender)
        json1 = gbench.report.filter_benchmark(
            json1_orig, filter_baseline, replacement)
        json2 = gbench.report.filter_benchmark(
            json2_orig, filter_contender, replacement)

    # Diff and output
    output_lines = gbench.report.generate_difference_report(
        json1, json2, args.display_aggregates_only,
        args.utest, args.utest_alpha)
    print(description)
    for ln in output_lines:
        print(ln)


class TestParser(unittest.TestCase):
    def setUp(self):
        self.parser = create_parser()
        testInputs = os.path.join(
            os.path.dirname(
                os.path.realpath(__file__)),
            'gbench',
            'Inputs')
        self.testInput0 = os.path.join(testInputs, 'test1_run1.json')
        self.testInput1 = os.path.join(testInputs, 'test1_run2.json')

    def test_benchmarks_basic(self):
        parsed = self.parser.parse_args(
            ['benchmarks', self.testInput0, self.testInput1])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'benchmarks')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertFalse(parsed.benchmark_options)

    def test_benchmarks_basic_without_utest(self):
        parsed = self.parser.parse_args(
            ['--no-utest', 'benchmarks', self.testInput0, self.testInput1])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertFalse(parsed.utest)
        self.assertEqual(parsed.utest_alpha, 0.05)
        self.assertEqual(parsed.mode, 'benchmarks')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertFalse(parsed.benchmark_options)

    def test_benchmarks_basic_display_aggregates_only(self):
        parsed = self.parser.parse_args(
            ['-a', 'benchmarks', self.testInput0, self.testInput1])
        self.assertTrue(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'benchmarks')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertFalse(parsed.benchmark_options)

    def test_benchmarks_basic_with_utest_alpha(self):
        parsed = self.parser.parse_args(
            ['--alpha=0.314', 'benchmarks', self.testInput0, self.testInput1])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.utest_alpha, 0.314)
        self.assertEqual(parsed.mode, 'benchmarks')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertFalse(parsed.benchmark_options)

    def test_benchmarks_basic_without_utest_with_utest_alpha(self):
        parsed = self.parser.parse_args(
            ['--no-utest', '--alpha=0.314', 'benchmarks', self.testInput0, self.testInput1])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertFalse(parsed.utest)
        self.assertEqual(parsed.utest_alpha, 0.314)
        self.assertEqual(parsed.mode, 'benchmarks')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertFalse(parsed.benchmark_options)

    def test_benchmarks_with_remainder(self):
        parsed = self.parser.parse_args(
            ['benchmarks', self.testInput0, self.testInput1, 'd'])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'benchmarks')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertEqual(parsed.benchmark_options, ['d'])

    def test_benchmarks_with_remainder_after_doubleminus(self):
        parsed = self.parser.parse_args(
            ['benchmarks', self.testInput0, self.testInput1, '--', 'e'])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'benchmarks')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertEqual(parsed.benchmark_options, ['e'])

    def test_filters_basic(self):
        parsed = self.parser.parse_args(
            ['filters', self.testInput0, 'c', 'd'])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'filters')
        self.assertEqual(parsed.test[0].name, self.testInput0)
        self.assertEqual(parsed.filter_baseline[0], 'c')
        self.assertEqual(parsed.filter_contender[0], 'd')
        self.assertFalse(parsed.benchmark_options)

    def test_filters_with_remainder(self):
        parsed = self.parser.parse_args(
            ['filters', self.testInput0, 'c', 'd', 'e'])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'filters')
        self.assertEqual(parsed.test[0].name, self.testInput0)
        self.assertEqual(parsed.filter_baseline[0], 'c')
        self.assertEqual(parsed.filter_contender[0], 'd')
        self.assertEqual(parsed.benchmark_options, ['e'])

    def test_filters_with_remainder_after_doubleminus(self):
        parsed = self.parser.parse_args(
            ['filters', self.testInput0, 'c', 'd', '--', 'f'])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'filters')
        self.assertEqual(parsed.test[0].name, self.testInput0)
        self.assertEqual(parsed.filter_baseline[0], 'c')
        self.assertEqual(parsed.filter_contender[0], 'd')
        self.assertEqual(parsed.benchmark_options, ['f'])

    def test_benchmarksfiltered_basic(self):
        parsed = self.parser.parse_args(
            ['benchmarksfiltered', self.testInput0, 'c', self.testInput1, 'e'])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'benchmarksfiltered')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.filter_baseline[0], 'c')
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertEqual(parsed.filter_contender[0], 'e')
        self.assertFalse(parsed.benchmark_options)

    def test_benchmarksfiltered_with_remainder(self):
        parsed = self.parser.parse_args(
            ['benchmarksfiltered', self.testInput0, 'c', self.testInput1, 'e', 'f'])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'benchmarksfiltered')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.filter_baseline[0], 'c')
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertEqual(parsed.filter_contender[0], 'e')
        self.assertEqual(parsed.benchmark_options[0], 'f')

    def test_benchmarksfiltered_with_remainder_after_doubleminus(self):
        parsed = self.parser.parse_args(
            ['benchmarksfiltered', self.testInput0, 'c', self.testInput1, 'e', '--', 'g'])
        self.assertFalse(parsed.display_aggregates_only)
        self.assertTrue(parsed.utest)
        self.assertEqual(parsed.mode, 'benchmarksfiltered')
        self.assertEqual(parsed.test_baseline[0].name, self.testInput0)
        self.assertEqual(parsed.filter_baseline[0], 'c')
        self.assertEqual(parsed.test_contender[0].name, self.testInput1)
        self.assertEqual(parsed.filter_contender[0], 'e')
        self.assertEqual(parsed.benchmark_options[0], 'g')


if __name__ == '__main__':
    # unittest.main()
    main()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
# kate: tab-width: 4; replace-tabs on; indent-width 4; tab-indents: off;
# kate: indent-mode python; remove-trailing-spaces modified;
