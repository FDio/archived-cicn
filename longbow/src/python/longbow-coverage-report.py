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
import argparse

sys.path.append("@INSTALL_PYTHON_DIR@")
sys.path.append("@DEPENDENCY_PYTHON_DIR@")
sys.path.append("../site-packages/longbow/")
import CoverageReport


if __name__ == '__main__':
    '''
@(#) longbow-coverage-report @VERSION@ @DATE@
@(#)   All Rights Reserved. Use is subject to license terms.
'''
    description = '''
longbow-coverage-report @VERSION@ @DATE@
All Rights Reserved. Use is subject to license terms.

Report on the code coverage of tests.

The source files or executables to analyse are supplied as command line parameters,
or as a list of newline separated file names read from standard input.

Output is plain-text (default --output text) or a CSV file (--output csv)
reporting the results.

Results are:
  An average of all files specified (--average)
  A one line summary of all files specified (--summary)
  A listing of the original source file, colorized showing tested and non-tested lines.
    '''
    parser = argparse.ArgumentParser(prog='longbow-coverage-report',
                                     formatter_class=argparse.RawDescriptionHelpFormatter,
                                     description=description)
    parser.add_argument('-', '--stdin', default=False, action="store_true", required=False,
                        help="Read the list of files from standard input.")
    parser.add_argument('-s', '--summary', default=False, action="store_true", required=False,
                        help="Display the score for each file (excluding test source files).")
    parser.add_argument('-a', '--average', default=False, action="store_true", required=False,
                        help="Display the average score for all C source files (excluding test source files).")
    parser.add_argument('-o', '--output', default="text", action="store", required=False, type=str,
                        help="Set the output format: \"text\" or \"csv\"")
    parser.add_argument('-v', '--visual', default=False, action="store_true", required=False,
                        help="Colorize the original source code showing coverage")
    parser.add_argument('-x', '--explain', default=False, action="store_true", required=False,
                        help="Display information about the collection of coverage information (guru mode).")
    parser.add_argument('-d', '--distribution', default="[95, 90]", action="store", required=False, type=str,
                        help="A list containing the score distributions for pretty-printing. Default [95, 90]")
    parser.add_argument('-T', '--includeTestSources', default=False, action="store_true", required=False,
                        help="Include analysis of the test sources. Default False")
    parser.add_argument('-t', '--testDir', default="", action="store", required=False, type=str,
                        help="Directory hint for locating test files.")

    parser.add_argument("files", help="Files to check", nargs="*")

    args = parser.parse_args()

    if not args.summary and not args.average and not args.visual and not args.explain:
        args.summary = True

    fileNames = []

    if args.stdin:
        for line in sys.stdin:
            t = line.strip()
            if len(t) > 0:
                fileNames.append(t)
    else:
        fileNames = args.files

    CoverageReport.commandLineMain(args, fileNames, args.testDir)
