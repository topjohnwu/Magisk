import unittest
"""report.py - Utilities for reporting statistics about benchmark results
"""
import os
import re
import copy

from scipy.stats import mannwhitneyu


class BenchmarkColor(object):
    def __init__(self, name, code):
        self.name = name
        self.code = code

    def __repr__(self):
        return '%s%r' % (self.__class__.__name__,
                         (self.name, self.code))

    def __format__(self, format):
        return self.code


# Benchmark Colors Enumeration
BC_NONE = BenchmarkColor('NONE', '')
BC_MAGENTA = BenchmarkColor('MAGENTA', '\033[95m')
BC_CYAN = BenchmarkColor('CYAN', '\033[96m')
BC_OKBLUE = BenchmarkColor('OKBLUE', '\033[94m')
BC_OKGREEN = BenchmarkColor('OKGREEN', '\033[32m')
BC_HEADER = BenchmarkColor('HEADER', '\033[92m')
BC_WARNING = BenchmarkColor('WARNING', '\033[93m')
BC_WHITE = BenchmarkColor('WHITE', '\033[97m')
BC_FAIL = BenchmarkColor('FAIL', '\033[91m')
BC_ENDC = BenchmarkColor('ENDC', '\033[0m')
BC_BOLD = BenchmarkColor('BOLD', '\033[1m')
BC_UNDERLINE = BenchmarkColor('UNDERLINE', '\033[4m')

UTEST_MIN_REPETITIONS = 2
UTEST_OPTIMAL_REPETITIONS = 9  # Lowest reasonable number, More is better.
UTEST_COL_NAME = "_pvalue"


def color_format(use_color, fmt_str, *args, **kwargs):
    """
    Return the result of 'fmt_str.format(*args, **kwargs)' after transforming
    'args' and 'kwargs' according to the value of 'use_color'. If 'use_color'
    is False then all color codes in 'args' and 'kwargs' are replaced with
    the empty string.
    """
    assert use_color is True or use_color is False
    if not use_color:
        args = [arg if not isinstance(arg, BenchmarkColor) else BC_NONE
                for arg in args]
        kwargs = {key: arg if not isinstance(arg, BenchmarkColor) else BC_NONE
                  for key, arg in kwargs.items()}
    return fmt_str.format(*args, **kwargs)


def find_longest_name(benchmark_list):
    """
    Return the length of the longest benchmark name in a given list of
    benchmark JSON objects
    """
    longest_name = 1
    for bc in benchmark_list:
        if len(bc['name']) > longest_name:
            longest_name = len(bc['name'])
    return longest_name


def calculate_change(old_val, new_val):
    """
    Return a float representing the decimal change between old_val and new_val.
    """
    if old_val == 0 and new_val == 0:
        return 0.0
    if old_val == 0:
        return float(new_val - old_val) / (float(old_val + new_val) / 2)
    return float(new_val - old_val) / abs(old_val)


def filter_benchmark(json_orig, family, replacement=""):
    """
    Apply a filter to the json, and only leave the 'family' of benchmarks.
    """
    regex = re.compile(family)
    filtered = {}
    filtered['benchmarks'] = []
    for be in json_orig['benchmarks']:
        if not regex.search(be['name']):
            continue
        filteredbench = copy.deepcopy(be)  # Do NOT modify the old name!
        filteredbench['name'] = regex.sub(replacement, filteredbench['name'])
        filtered['benchmarks'].append(filteredbench)
    return filtered


def get_unique_benchmark_names(json):
    """
    While *keeping* the order, give all the unique 'names' used for benchmarks.
    """
    seen = set()
    uniqued = [x['name'] for x in json['benchmarks']
               if x['name'] not in seen and
               (seen.add(x['name']) or True)]
    return uniqued


def intersect(list1, list2):
    """
    Given two lists, get a new list consisting of the elements only contained
    in *both of the input lists*, while preserving the ordering.
    """
    return [x for x in list1 if x in list2]


def partition_benchmarks(json1, json2):
    """
    While preserving the ordering, find benchmarks with the same names in
    both of the inputs, and group them.
    (i.e. partition/filter into groups with common name)
    """
    json1_unique_names = get_unique_benchmark_names(json1)
    json2_unique_names = get_unique_benchmark_names(json2)
    names = intersect(json1_unique_names, json2_unique_names)
    partitions = []
    for name in names:
        # Pick the time unit from the first entry of the lhs benchmark.
        time_unit = (x['time_unit']
                     for x in json1['benchmarks'] if x['name'] == name).next()
        # Filter by name and time unit.
        lhs = [x for x in json1['benchmarks'] if x['name'] == name and
               x['time_unit'] == time_unit]
        rhs = [x for x in json2['benchmarks'] if x['name'] == name and
               x['time_unit'] == time_unit]
        partitions.append([lhs, rhs])
    return partitions


def extract_field(partition, field_name):
    # The count of elements may be different. We want *all* of them.
    lhs = [x[field_name] for x in partition[0]]
    rhs = [x[field_name] for x in partition[1]]
    return [lhs, rhs]


def print_utest(partition, utest_alpha, first_col_width, use_color=True):
    timings_time = extract_field(partition, 'real_time')
    timings_cpu = extract_field(partition, 'cpu_time')

    min_rep_cnt = min(len(timings_time[0]),
                      len(timings_time[1]),
                      len(timings_cpu[0]),
                      len(timings_cpu[1]))

    # Does *everything* has at least UTEST_MIN_REPETITIONS repetitions?
    if min_rep_cnt < UTEST_MIN_REPETITIONS:
        return []

    def get_utest_color(pval):
        return BC_FAIL if pval >= utest_alpha else BC_OKGREEN

    time_pvalue = mannwhitneyu(
        timings_time[0], timings_time[1], alternative='two-sided').pvalue
    cpu_pvalue = mannwhitneyu(
        timings_cpu[0], timings_cpu[1], alternative='two-sided').pvalue

    dsc = "U Test, Repetitions: {} vs {}".format(
        len(timings_cpu[0]), len(timings_cpu[1]))
    dsc_color = BC_OKGREEN

    if min_rep_cnt < UTEST_OPTIMAL_REPETITIONS:
        dsc_color = BC_WARNING
        dsc += ". WARNING: Results unreliable! {}+ repetitions recommended.".format(
            UTEST_OPTIMAL_REPETITIONS)

    special_str = "{}{:<{}s}{endc}{}{:16.4f}{endc}{}{:16.4f}{endc}{}      {}"

    last_name = partition[0][0]['name']
    return [color_format(use_color,
                         special_str,
                         BC_HEADER,
                         "{}{}".format(last_name, UTEST_COL_NAME),
                         first_col_width,
                         get_utest_color(time_pvalue), time_pvalue,
                         get_utest_color(cpu_pvalue), cpu_pvalue,
                         dsc_color, dsc,
                         endc=BC_ENDC)]


def generate_difference_report(
        json1,
        json2,
        display_aggregates_only=False,
        utest=False,
        utest_alpha=0.05,
        use_color=True):
    """
    Calculate and report the difference between each test of two benchmarks
    runs specified as 'json1' and 'json2'.
    """
    assert utest is True or utest is False
    first_col_width = find_longest_name(json1['benchmarks'])

    def find_test(name):
        for b in json2['benchmarks']:
            if b['name'] == name:
                return b
        return None

    first_col_width = max(
        first_col_width,
        len('Benchmark'))
    first_col_width += len(UTEST_COL_NAME)
    first_line = "{:<{}s}Time             CPU      Time Old      Time New       CPU Old       CPU New".format(
        'Benchmark', 12 + first_col_width)
    output_strs = [first_line, '-' * len(first_line)]

    partitions = partition_benchmarks(json1, json2)
    for partition in partitions:
        # Careful, we may have different repetition count.
        for i in range(min(len(partition[0]), len(partition[1]))):
            bn = partition[0][i]
            other_bench = partition[1][i]

            # *If* we were asked to only display aggregates,
            # and if it is non-aggregate, then skip it.
            if display_aggregates_only and 'run_type' in bn and 'run_type' in other_bench:
                assert bn['run_type'] == other_bench['run_type']
                if bn['run_type'] != 'aggregate':
                    continue

            fmt_str = "{}{:<{}s}{endc}{}{:+16.4f}{endc}{}{:+16.4f}{endc}{:14.0f}{:14.0f}{endc}{:14.0f}{:14.0f}"

            def get_color(res):
                if res > 0.05:
                    return BC_FAIL
                elif res > -0.07:
                    return BC_WHITE
                else:
                    return BC_CYAN

            tres = calculate_change(bn['real_time'], other_bench['real_time'])
            cpures = calculate_change(bn['cpu_time'], other_bench['cpu_time'])
            output_strs += [color_format(use_color,
                                         fmt_str,
                                         BC_HEADER,
                                         bn['name'],
                                         first_col_width,
                                         get_color(tres),
                                         tres,
                                         get_color(cpures),
                                         cpures,
                                         bn['real_time'],
                                         other_bench['real_time'],
                                         bn['cpu_time'],
                                         other_bench['cpu_time'],
                                         endc=BC_ENDC)]

        # After processing the whole partition, if requested, do the U test.
        if utest:
            output_strs += print_utest(partition,
                                       utest_alpha=utest_alpha,
                                       first_col_width=first_col_width,
                                       use_color=use_color)

    return output_strs


###############################################################################
# Unit tests


class TestGetUniqueBenchmarkNames(unittest.TestCase):
    def load_results(self):
        import json
        testInputs = os.path.join(
            os.path.dirname(
                os.path.realpath(__file__)),
            'Inputs')
        testOutput = os.path.join(testInputs, 'test3_run0.json')
        with open(testOutput, 'r') as f:
            json = json.load(f)
        return json

    def test_basic(self):
        expect_lines = [
            'BM_One',
            'BM_Two',
            'short',  # These two are not sorted
            'medium',  # These two are not sorted
        ]
        json = self.load_results()
        output_lines = get_unique_benchmark_names(json)
        print("\n")
        print("\n".join(output_lines))
        self.assertEqual(len(output_lines), len(expect_lines))
        for i in range(0, len(output_lines)):
            self.assertEqual(expect_lines[i], output_lines[i])


class TestReportDifference(unittest.TestCase):
    def load_results(self):
        import json
        testInputs = os.path.join(
            os.path.dirname(
                os.path.realpath(__file__)),
            'Inputs')
        testOutput1 = os.path.join(testInputs, 'test1_run1.json')
        testOutput2 = os.path.join(testInputs, 'test1_run2.json')
        with open(testOutput1, 'r') as f:
            json1 = json.load(f)
        with open(testOutput2, 'r') as f:
            json2 = json.load(f)
        return json1, json2

    def test_basic(self):
        expect_lines = [
            ['BM_SameTimes', '+0.0000', '+0.0000', '10', '10', '10', '10'],
            ['BM_2xFaster', '-0.5000', '-0.5000', '50', '25', '50', '25'],
            ['BM_2xSlower', '+1.0000', '+1.0000', '50', '100', '50', '100'],
            ['BM_1PercentFaster', '-0.0100', '-0.0100', '100', '99', '100', '99'],
            ['BM_1PercentSlower', '+0.0100', '+0.0100', '100', '101', '100', '101'],
            ['BM_10PercentFaster', '-0.1000', '-0.1000', '100', '90', '100', '90'],
            ['BM_10PercentSlower', '+0.1000', '+0.1000', '100', '110', '100', '110'],
            ['BM_100xSlower', '+99.0000', '+99.0000',
                '100', '10000', '100', '10000'],
            ['BM_100xFaster', '-0.9900', '-0.9900',
                '10000', '100', '10000', '100'],
            ['BM_10PercentCPUToTime', '+0.1000',
                '-0.1000', '100', '110', '100', '90'],
            ['BM_ThirdFaster', '-0.3333', '-0.3334', '100', '67', '100', '67'],
            ['BM_BadTimeUnit', '-0.9000', '+0.2000', '0', '0', '0', '1'],
        ]
        json1, json2 = self.load_results()
        output_lines_with_header = generate_difference_report(
            json1, json2, use_color=False)
        output_lines = output_lines_with_header[2:]
        print("\n")
        print("\n".join(output_lines_with_header))
        self.assertEqual(len(output_lines), len(expect_lines))
        for i in range(0, len(output_lines)):
            parts = [x for x in output_lines[i].split(' ') if x]
            self.assertEqual(len(parts), 7)
            self.assertEqual(expect_lines[i], parts)


class TestReportDifferenceBetweenFamilies(unittest.TestCase):
    def load_result(self):
        import json
        testInputs = os.path.join(
            os.path.dirname(
                os.path.realpath(__file__)),
            'Inputs')
        testOutput = os.path.join(testInputs, 'test2_run.json')
        with open(testOutput, 'r') as f:
            json = json.load(f)
        return json

    def test_basic(self):
        expect_lines = [
            ['.', '-0.5000', '-0.5000', '10', '5', '10', '5'],
            ['./4', '-0.5000', '-0.5000', '40', '20', '40', '20'],
            ['Prefix/.', '-0.5000', '-0.5000', '20', '10', '20', '10'],
            ['Prefix/./3', '-0.5000', '-0.5000', '30', '15', '30', '15'],
        ]
        json = self.load_result()
        json1 = filter_benchmark(json, "BM_Z.ro", ".")
        json2 = filter_benchmark(json, "BM_O.e", ".")
        output_lines_with_header = generate_difference_report(
            json1, json2, use_color=False)
        output_lines = output_lines_with_header[2:]
        print("\n")
        print("\n".join(output_lines_with_header))
        self.assertEqual(len(output_lines), len(expect_lines))
        for i in range(0, len(output_lines)):
            parts = [x for x in output_lines[i].split(' ') if x]
            self.assertEqual(len(parts), 7)
            self.assertEqual(expect_lines[i], parts)


class TestReportDifferenceWithUTest(unittest.TestCase):
    def load_results(self):
        import json
        testInputs = os.path.join(
            os.path.dirname(
                os.path.realpath(__file__)),
            'Inputs')
        testOutput1 = os.path.join(testInputs, 'test3_run0.json')
        testOutput2 = os.path.join(testInputs, 'test3_run1.json')
        with open(testOutput1, 'r') as f:
            json1 = json.load(f)
        with open(testOutput2, 'r') as f:
            json2 = json.load(f)
        return json1, json2

    def test_utest(self):
        expect_lines = []
        expect_lines = [
            ['BM_One', '-0.1000', '+0.1000', '10', '9', '100', '110'],
            ['BM_Two', '+0.1111', '-0.0111', '9', '10', '90', '89'],
            ['BM_Two', '-0.1250', '-0.1628', '8', '7', '86', '72'],
            ['BM_Two_pvalue',
             '0.6985',
             '0.6985',
             'U',
             'Test,',
             'Repetitions:',
             '2',
             'vs',
             '2.',
             'WARNING:',
             'Results',
             'unreliable!',
             '9+',
             'repetitions',
             'recommended.'],
            ['short', '-0.1250', '-0.0625', '8', '7', '80', '75'],
            ['short', '-0.4325', '-0.1351', '8', '5', '77', '67'],
            ['short_pvalue',
             '0.7671',
             '0.1489',
             'U',
             'Test,',
             'Repetitions:',
             '2',
             'vs',
             '3.',
             'WARNING:',
             'Results',
             'unreliable!',
             '9+',
             'repetitions',
             'recommended.'],
            ['medium', '-0.3750', '-0.3375', '8', '5', '80', '53'],
        ]
        json1, json2 = self.load_results()
        output_lines_with_header = generate_difference_report(
            json1, json2, utest=True, utest_alpha=0.05, use_color=False)
        output_lines = output_lines_with_header[2:]
        print("\n")
        print("\n".join(output_lines_with_header))
        self.assertEqual(len(output_lines), len(expect_lines))
        for i in range(0, len(output_lines)):
            parts = [x for x in output_lines[i].split(' ') if x]
            self.assertEqual(expect_lines[i], parts)


class TestReportDifferenceWithUTestWhileDisplayingAggregatesOnly(
        unittest.TestCase):
    def load_results(self):
        import json
        testInputs = os.path.join(
            os.path.dirname(
                os.path.realpath(__file__)),
            'Inputs')
        testOutput1 = os.path.join(testInputs, 'test3_run0.json')
        testOutput2 = os.path.join(testInputs, 'test3_run1.json')
        with open(testOutput1, 'r') as f:
            json1 = json.load(f)
        with open(testOutput2, 'r') as f:
            json2 = json.load(f)
        return json1, json2

    def test_utest(self):
        expect_lines = []
        expect_lines = [
            ['BM_One', '-0.1000', '+0.1000', '10', '9', '100', '110'],
            ['BM_Two', '+0.1111', '-0.0111', '9', '10', '90', '89'],
            ['BM_Two', '-0.1250', '-0.1628', '8', '7', '86', '72'],
            ['BM_Two_pvalue',
             '0.6985',
             '0.6985',
             'U',
             'Test,',
             'Repetitions:',
             '2',
             'vs',
             '2.',
             'WARNING:',
             'Results',
             'unreliable!',
             '9+',
             'repetitions',
             'recommended.'],
            ['short', '-0.1250', '-0.0625', '8', '7', '80', '75'],
            ['short', '-0.4325', '-0.1351', '8', '5', '77', '67'],
            ['short_pvalue',
             '0.7671',
             '0.1489',
             'U',
             'Test,',
             'Repetitions:',
             '2',
             'vs',
             '3.',
             'WARNING:',
             'Results',
             'unreliable!',
             '9+',
             'repetitions',
             'recommended.'],
        ]
        json1, json2 = self.load_results()
        output_lines_with_header = generate_difference_report(
            json1, json2, display_aggregates_only=True,
            utest=True, utest_alpha=0.05, use_color=False)
        output_lines = output_lines_with_header[2:]
        print("\n")
        print("\n".join(output_lines_with_header))
        self.assertEqual(len(output_lines), len(expect_lines))
        for i in range(0, len(output_lines)):
            parts = [x for x in output_lines[i].split(' ') if x]
            self.assertEqual(expect_lines[i], parts)


if __name__ == '__main__':
    unittest.main()

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
# kate: tab-width: 4; replace-tabs on; indent-width 4; tab-indents: off;
# kate: indent-mode python; remove-trailing-spaces modified;
