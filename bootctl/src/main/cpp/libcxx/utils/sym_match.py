#!/usr/bin/env python
#===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===##

"""
sym_match - Match all symbols in a list against a list of regexes.
"""
from argparse import ArgumentParser
import sys
from libcxx.sym_check import util, match, extract


def main():
    parser = ArgumentParser(
        description='Extract a list of symbols from a shared library.')
    parser.add_argument(
        '--blacklist', dest='blacklist',
        type=str, action='store', default=None)
    parser.add_argument(
        'symbol_list', metavar='symbol_list', type=str,
        help='The file containing the old symbol list')
    parser.add_argument(
        'regexes', metavar='regexes', default=[], nargs='*',
        help='The file containing the new symbol list or a library')
    args = parser.parse_args()

    if not args.regexes and args.blacklist is None:
        sys.stderr.write('Either a regex or a blacklist must be specified.\n')
        sys.exit(1)
    if args.blacklist:
        search_list = util.read_blacklist(args.blacklist)
    else:
        search_list = args.regexes

    symbol_list = util.extract_or_load(args.symbol_list)

    matching_count, report = match.find_and_report_matching(
        symbol_list, search_list)
    sys.stdout.write(report)
    if matching_count != 0:
        print('%d matching symbols found...' % matching_count)


if __name__ == '__main__':
    main()
