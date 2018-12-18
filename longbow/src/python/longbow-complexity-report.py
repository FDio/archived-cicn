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
import itertools
sys.path.append("@INSTALL_PYTHON_DIR@")
sys.path.append("@DEPENDENCY_PYTHON_DIR@")
import LongBow
try:
	import hfcca
except ImportError:
  	print "HFCCA not found. You need to download hfcca.py and place it in a location"
	print "where this script (python) can find it."
	print "You can find a compatible version of hfcca at: "
	print "  https://headerfile-free-cyclomatic-complexity-analyzer.googlecode.com/files/hfcca.py"
	print "And place it at: @INSTALL_PYTHON_DIR@"
	print
	print "... however, you should have run the ccnx-post-install script"
	print "    (from the ccnx distribution you got this from)"
	sys.exit(1)

def computeComplexityScore(complexity):
	score = min(100.0 * abs(1.0 - float(complexity - 5) / 50.0), 100.0)
	return score

def csvFunctionResult(file, function):
	score = computeComplexityScore(function.cyclomatic_complexity)
	string = "complexity,%s,%s,%d,%d,%.2f" % (file.filename, function.name, function.start_line, function.cyclomatic_complexity, score)

	LongBow.scorePrinter([90, 80], score, string)
	return function.cyclomatic_complexity

def csvFileComplexity(file):
	score = computeComplexityScore(file.average_CCN)
	string = "complexity,%s,,,%.2f,%.2f" % (file.filename, file.average_CCN, score)
	LongBow.scorePrinter([90, 80], score, string)
	return

def csvFunction(fileInformationList):
	for fileInformation in fileInformationList:
		complexities = map(lambda function: csvFunctionResult(fileInformation, function), fileInformation)
	return

def csvSummary(fileInformationList):
	map(lambda file: csvFileComplexity(file), fileInformationList)
	return


def textFunctionResult(file, function, maxFileNameLength, maxFunctionNameLength):
	score = computeComplexityScore(function.cyclomatic_complexity)
	format = "%-" + str(maxFileNameLength) + "s %-" + str(maxFunctionNameLength) + "s %6d %2d %6.2f"
	string = format % (file.filename, function.name, function.start_line, function.cyclomatic_complexity, score)

	LongBow.scorePrinter([90, 80], score, string)
	return function.cyclomatic_complexity

def textFileComplexity(file, maxFileNameLength):
	score = computeComplexityScore(file.average_CCN)
	string =  ("%-" + str(maxFileNameLength) + "s %6.2f %6.2f") % (file.filename, file.average_CCN, score)
	LongBow.scorePrinter([90, 80], score, string)
	return

def computeMaxFileNameLength(fileInformationList):
	result = 0
	for fileInformation in fileInformationList:
		if len(fileInformation.filename) > result:
			result = len(fileInformation.filename)
	return result

def computeMaxFunctionNameLength(fileInformationList):
	result = 0
	for fileInformation in fileInformationList:
		if len(fileInformation.filename) > result:
			result = len(fileInformation.filename)
	return result

def textFunction(fileInformationList):
		maxFileNameLength = max(map(lambda fileInformation: len(fileInformation.filename), fileInformationList))
		maxFunctionNameLength = max(map(lambda fileInformation: max(map(lambda function: len(function.name), fileInformation)), fileInformationList))

		for fileInformation in fileInformationList:
			complexities = map(lambda function: textFunctionResult(fileInformation, function,  maxFileNameLength, maxFunctionNameLength), fileInformation)
		return

def textSummary(fileInformationList):
		maxFileNameLength = max(map(lambda fileInformation: len(fileInformation.filename), fileInformationList))
		map(lambda file: textFileComplexity(file, maxFileNameLength), fileInformationList)
		return
#
# Recompute the file's average complexity as a floating point number.
def recomputeFileComplexity(fileInformation):
	complexities = map(lambda function: function.cyclomatic_complexity, fileInformation)
	if len(complexities) > 0:
		sum = reduce(lambda sum, complex: sum + complex, complexities)
		fileInformation.average_CCN = float(sum) / len(fileInformation)
	else:
		fileInformation.average_CCN = 0
	return fileInformation.average_CCN

def recomputeFilesComplexity(fileInformationList):
	return map(lambda fileInformation: recomputeFileComplexity(fileInformation), fileInformationList)

def computeAverage(fileInformationList):
	cyclomaticComplexity = map(lambda fileInformation : fileInformation.average_CCN, fileInformationList)
	sum = reduce(lambda sum, x: sum + x, cyclomaticComplexity)
	return float(sum) / float(len(cyclomaticComplexity))

def main(argv):
	desc = '''longbow-complexity-report @VERSION@ @DATE@
	   All Rights Reserved. Use is subject to license terms.

Print the cyclomatic complexity of functions and files.

The option --function displays the file name, function name,
line number of the function, the cyclomatic complexity and a score ranging from 0 to 100.

The default option --summary displays the file name,
the average cyclomatic complexity of all functions in the file and
a score ranging from 0 to 100.

Input is either from a list of files supplied as command line parameters,
or as a list of newline separated file names read from standard input.
Output is a plain text (default) or comma-separated-value (CSV).

Examples:

% longbow-complexity-report *.[ch]

Report conformance of the .c and .h files specified as command line parameters.

% longbow-complexity-report -
Report conformance of the .c and .h files read from standard input, one line per file.

$ longbow-complexity-report parc_JSON.c
parc_JSON.c   2.27 100.00
$
$ echo parc_JSON.c | longbow-complexity-report -o csv -
complexity,parc_JSON.c,,,2.27,100.00
$
'''

	parser = argparse.ArgumentParser(prog='longbow-complexity-report', formatter_class=argparse.RawDescriptionHelpFormatter, description=desc)
	parser.add_argument('-s', '--summary', default=False, action="store_true", help="print the average complexity of each target file.")
	parser.add_argument('-f', '--function', default=False, action="store_true", help="print the complexity of each function in each target file.")
	parser.add_argument('-', '--stdin', default=False, action="store_true", required=False, help="read the list of files from standard input rather than the command line.")
	parser.add_argument('-a', '--average', default=False, action="store_true", required=False, help="display only the simple average of the average complexity of each target file.")
	parser.add_argument('-o', '--output', default="text", action="store", required=False, type=str, help="the output format: \"text\" or \"csv\"")
	parser.add_argument("files", help="Files to check", nargs="*")

	args = parser.parse_args()

	targets = []

	if args.stdin:
		for line in sys.stdin:
			t = line.strip()
			if (len(t) > 0):
				targets.append(t)
	else:
		targets = args.files

	if (len(targets) == 0):
		print >> sys.stderr, "Error: No files to analyze.  See %s -h" % (sys.argv[0])
		sys.exit(1)

	# If nothing was specified, print the summary as a default
	if args.summary == False and args.function == False and args.average == False:
		args.summary = True

	options, arguments = hfcca.createHfccaCommandLineParser().parse_args(args=[argv[0]])
	result = hfcca.analyze(targets, options)

	# Convert from that iterator to a simple list...
	fileInformationList = map(lambda x : x, result)

	recomputeFilesComplexity(fileInformationList)

	if args.function:
		if args.output == "text":
			textFunction(fileInformationList)
		else:
			csvFunction(fileInformationList)

	if args.summary:
		if args.output == "text":
			textSummary(fileInformationList)
		else:
			csvSummary(fileInformationList)

	if args.average:
		print "%.2f" % computeAverage(fileInformationList)

if __name__ == "__main__":
	'''
@(#) longbow-complexity-report @VERSION@ @DATE@
@(#)   All Rights Reserved. Use is subject to license terms.
'''
	main(sys.argv)
