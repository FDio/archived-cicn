#! /usr/bin/env python
# Copyright (c) 2017 Cisco and/or its affiliates.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#

import sys
import os
import subprocess
import difflib
import csv
import argparse
sys.path.append("@INSTALL_PYTHON_DIR@")
import LongBow
from pprint import pprint

def textOutputFile(file, maximumFileNameLength):
    format = "%-" + str(maximumFileNameLength) + "s %6d"
    print format % (file[0], file[1])
    return

def textSummary(files):
    maximumFileNameLength = max(map(lambda file: len(file[0]), files))
    map(lambda file: textOutputFile(file, maximumFileNameLength), files)
    return

def csvOutputFile(file):
    format = "size,%s,%d"
    print format % (file[0], file[1])
    return

def csvSummary(files):
    map(lambda file: csvOutputFile(file), files)
    return

def textTotal(files):
    total = sum(map(lambda file: file[1], files))
    print total
    return

def csvTotal(files):
    total = sum(map(lambda file: file[1], files))
    print total
    return

def main():
    desc = '''
Report on number of lines of one or more C source or header files.

Input is either from a list of files supplied as command line parameters,
or as a list of newline separated file names read from standard input.
Output is a plain text (default) or a CSV file reporting
the file name and the total number of lines in the file.

Usage:

% longbow-size-report *.[ch]

Report the number of lines in .c and .h files specified as command line parameters.

% longbow-size-report -
Read the lists of files from standard input, one file per line.

$ longbow-size-report parc_JSON.c
parc_JSON.c    239
$
$
$ echo parc_JSON.c | longbow-size-report -o csv -
parc_JSON.c,239
$
'''

    parser = argparse.ArgumentParser(prog='longbow-size-report', formatter_class=argparse.RawDescriptionHelpFormatter, description=desc)
    parser.add_argument('-', '--stdin', default=False, action="store_true", required=False, help="read the list of files from standard input.")
    parser.add_argument('-s', '--summary', default=False, action="store_true", required=False, help="display the number of lines for each file")
    parser.add_argument('-t', '--total', default=False, action="store_true", required=False, help="display the total number of lines for all files")
    parser.add_argument('-o', '--output', default="text", action="store", required=False, type=str, help="the output format: \"text\" or \"csv\"")

    parser.add_argument("files", help="Files to check", nargs="*")

    args = parser.parse_args()

    if args.summary == False and args.total == False:
        args.summary = True

    targets = []

    if args.stdin:
        for line in sys.stdin:
            t = line.strip()
            if len(t) > 0:
                targets.append(t)
    else:
        targets = args.files

    if len(targets) == 0:
        parser.print_usage()
        sys.exit(1)

    files = map(lambda fileName: [ fileName, LongBow.countLines(fileName)], targets)
    total = sum(map(lambda element: element[1], files))

    if args.summary:
        if args.output == "text":
            textSummary(files)
        else:
            csvSummary(files)

    if args.total:
        if args.output == "text":
            textTotal(files)
        else:
            csvTotal(files)

if __name__ == '__main__':
    '''
    @(#) longbow-size-report @VERSION@ @DATE@
    @(#)   All Rights Reserved. Use is subject to license terms.
    '''
    main()
