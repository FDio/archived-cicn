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
import VocabularyReport
import hfcca

def main(argv):
	desc = '''
Print the vocabulary (number of tokens) of functions and files.

The option --function displays the file name, function name,
line number of the function, the number of tokens

The default option --summary displays the file name, the average vocabulary
of all functions in the file and a score ranging from 0 to 100.

Usage:
$ longbow-vocabulary-report parc_JSON.c
parc_JSON.c  51.00 100.00
$
$ echo parc_JSON.c | longbow-vocabulary-report --function -o csv -
vocabulary,parc_JSON.c,parcJSON_Create,49,50,100.00
...
$

'''

	parser = argparse.ArgumentParser(prog='longbow-vocabulary-report', formatter_class=argparse.RawDescriptionHelpFormatter, description=desc)
	parser.add_argument('-s', '--summary', default=False, action="store_true", help="print the average vocabulary of each target file.")
	parser.add_argument('-f', '--function', default=False, action="store_true", help="print the vocabulary of each function in each target file.")
	parser.add_argument('-', '--stdin', default=False, action="store_true", required=False, help="read the list of files from standard input rather than the command line.")
	parser.add_argument('-a', '--average', default=False, action="store_true", required=False, help="display only the simple average of the average vocabulary of each target file.")
	parser.add_argument('-o', '--output', default="text", action="store", required=False, type=str, help="the output format: \"text\" or \"csv\"")
	parser.add_argument("files", help="Files to check", nargs="*")

	args = parser.parse_args()

        VocabularyReport.commandLineMain(args, hfcca)


if __name__ == "__main__":
	'''
@(#) longbow-vocabulary-report @VERSION@ @DATE@
@(#)   All Rights Reserved. Use is subject to license terms.
	'''
	main(sys.argv)
