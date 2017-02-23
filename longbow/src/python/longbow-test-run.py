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
import os
import sys
import re
import pprint
import subprocess
import argparse
import json

class TokenParser:
    def __init__(self, tokens=[]):
        self.index = 0
        self.tokens = tokens

    def nextToken(self):
        result = self.tokens[self.index]
        self.index = self.index + 1
        return result

    def previousToken(self):
        self.index = self.index - 1
        result = self.tokens[self.index - 1]
        return result

    def expectedToken(self, expected):
        token = self.nextToken()
        if token == expected:
            return True
        self.index = self.index - 1
        print "expectedToken(%s) is not the actual %s" % (expected, token)
        return False

    def end(self):
        if self.index == len(self.tokens):
            return True
        return False

class LongBowCodeCoverage:
    def __init__(self):
        return

    def run(self, executableFile):
        lines = subprocess.check_output([ "gcov", "-f", executableFile ])
        token =  map(lambda x : x.strip("'"), re.split("[ :\n]+", lines))
        return self.parse(token)

    def parseFunction(self, parser):
        functionName = parser.nextToken()
        parser.expectedToken("Lines")
        parser.expectedToken("executed")
        coverage = parser.nextToken()
        return { "function" : functionName, "coverage" : coverage }
   
    def parseFile(self, parser):
        fileName = parser.nextToken()
        parser.expectedToken("Lines")
        parser.expectedToken("executed")
        coverage = parser.nextToken()
        return { "file" : fileName, "coverage" : coverage }
   
    def parse(self, tokens):
        parser = TokenParser(tokens)
        functions = [ ]
       
        while not parser.end():
            token = parser.nextToken()
            if (token == "Function"):
                function = self.parseFunction(parser)
                functions.append(function)
            elif (token == "File"):
                file = self.parseFile(parser)
            pass

        self.detailCoverage = { "file" : file, "functions" : functions }
        return self.detailCoverage

    def getCoverage(self):
        result["file"]["coverage"]

    def getDetailCoverage(self):
        return self.detailCoverage


class LongBowTestRun:
    def __init__(self, options=[]):
        self.options = options
        self.mainFileName = None
        self.exitStatus = 0
        return
   
    def setOptions(self, options=[]):
        self.options = options
        return
   
    def getMainFileName(self):
        return self.mainFileName
   
    def run(self, testRunner):
        self.mainFileName = testRunner
        self.exitStatus = 0

        try:
            try:
                os.remove(testRunner + ".gcda")
            except:
                pass
            lines = subprocess.check_output([ testRunner ])
            lines = re.split("[ :]+", lines)
            self.exitStatus = 0
        except subprocess.CalledProcessError, e:
            self.exitStatus = e.returncode
       
        return self.exitStatus

    def report(self, detailedOutput=False, jsonOutput=False):
        result = ""
        if self.exitStatus == 0:
            coverage = LongBowCodeCoverage()
            result = coverage.run(testRunner.getMainFileName())
   
            if detailedOutput:
                if jsonOutput:
                    result = json.dumps(result, sort_keys=False, indent=4, separators=(',', ': '))
                else:
                    pp = str(result)
                    pass
            else:
                if jsonOutput:
                    result = json.dumps(result["file"], sort_keys=False, indent=4, separators=(',', ': '))
                else:
                    result = "PASS " + result["file"]["file"] + " " + result["file"]["coverage"]
        else:
            result = "FAIL " + args.testRunner
            pass

        return result


if __name__ == '__main__':
    testRunners = []
    if len(sys.argv) < 2:
        print "Usage: longbow-test-run.py testExecutable"
        print "Run a LongBow test"
        sys.exit(1)
   
    parser = argparse.ArgumentParser(description='Run a LongBow Test')
    parser.add_argument("--json", help="Produce JSON output instead of a Python dictionary.",  action="store_true")
    parser.add_argument("--detailed", help="Produce detailed output.",  action="store_true")
    parser.add_argument("testRunner", help="The name of the test executable.", nargs='+')
    args = parser.parse_args()

    testRunner = LongBowTestRun([ "--run-nonforked" ])

    for test in args.testRunner:
        exitStatus = testRunner.run(test)
        print testRunner.report(args.detailed, args.json)

