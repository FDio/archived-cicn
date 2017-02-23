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
import StyleReport

if __name__ == '__main__':
    '''
@(#) longbow-code @VERSION@ @DATE@
	@(#)   All Rights Reserved. Use is subject to license terms.
'''
    desc = '''
Report on style conformance for one or more C source or header files.

Input is either from a list of files supplied as command line parameters,
or as a list of newline separated file names read from standard input.
Reports are:
--summary A one line per file report of the file name, number of lines, number of non-compliant lines, and a score.
--average A single line output of the average of all scores.
--total A single line of output of the percentage of all compliant lines to the total number of lines in all files.
--visual A visual representation of the style check.

For each of these reports, the output format is specified by:
--output text  Display text on standard output
--output csv Display a list of comma-separated values on standard output.
--output gui Use a graphical user interface if possible.

The visual report displays either a colorized, line by line output of
the differences between the original source file it's exemplar (-o text),
or displays a file-merge application for interactive use ()-o gui)

Example:

% longbow-style-report *.[ch]

Report conformance of the .c and .h files specified as command line parameters.

% longbow-style-report -
Report conformance of the .c and .h files read from standard input, one line per file.

$ longbow-style-report parc_JSON.c
parc_JSON.c   239     0 100.00$
$
$ echo parc_JSON.c | longbow-style-report -
parc_JSON.c,239,0,100.00
$
'''

    parser = argparse.ArgumentParser(prog='longbow-style-report', formatter_class=argparse.RawDescriptionHelpFormatter, description=desc)
    parser.add_argument('-', '--stdin', default=False, action="store_true", required=False, help="read the list of files from standard input only.")
    parser.add_argument('-s', '--summary', default=False, action="store_true", required=False, help="Display the score for each file.")
    parser.add_argument('-a', '--average', default=False, action="store_true", required=False, help="Display the simple average of all scores.")
    parser.add_argument('-t', '--total', default=False, action="store_true", required=False, help="Display the percentage of all compliant lines to the total number of lines in all files.")
    parser.add_argument('-d', '--distribution', default="[95, 90]", action="store", required=False, type=str, help="a list containing the score distributions for pretty-printing. Default '[95, 90]' (green >= 95, yellow >= 90, red < 90).")
    parser.add_argument('-o', '--output', default="text", action="store", required=False, type=str, help="the output format: 'text', 'csv', or 'gui'.")
    parser.add_argument('-v', '--visual', default=False, action="store_true", required=False, help="Display a visual representation of the style check.")
    parser.add_argument('-k', '--key', default="name", action="store", required=False, type=str, help="The sort key: Type '--key help' for the list.")
    parser.add_argument('-e', '--exclude', default="", action="store", required=False, type=str, help="Exclude a comma separated set of one or more of: 'red', 'yellow', 'green'.")

    parser.add_argument("files", help="Files to check", nargs="*")

    args = parser.parse_args()

    if args.summary == False and args.average == False and args.total == False and args.visual == False:
        args.summary = True

    targets = []

    if args.stdin:
        for line in sys.stdin:
            t = line.strip()
            if len(t) > 0:
                targets.append(t)
    else:
        targets = args.files

    UNCRUSTIFY = "@UNCRUSTIFY_BIN@"
    UNCRUSTIFY_CONFIG = "@UNCRUSTIFY_CONFIG@"

    StyleReport.commandLineMain(args, targets, UNCRUSTIFY, UNCRUSTIFY_CONFIG)
