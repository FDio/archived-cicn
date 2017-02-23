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
import pprint

def sourceFileNameToName(sourceFileName):
    '''
    Given the path to a source file, return the name without any path components or trailing suffix.
    '''
    name = os.path.basename(sourceFileName)
    return name.split(".")[0]

def canonicalizeFunctionName(functionName):
    '''
    Given a function name that contains the initial '_' character,
    strip it and return a canonicalised form of the same name suitable for a source file.
    '''
    if functionName[0] == "_":
        functionName = functionName[1:]
    return functionName

def isReservedName(functionName):
    '''
    Given a canonicalized name, determine if it is a reserved name according to ISO/IEC 9899:2011 and  ANSI Sec. 4.1.2.1,
    identifiers that begin with an underscore and either an uppercase letter or another underscore are always reserved for any use.
    '''
    if functionName[0] == '_' and functionName[1] == '_':
        return True
    elif functionName[0] == '_' and functionName[1].isupper():
        return True

    return False


def getDarwinTestableFunctions(sourceFileName, objectFileName):
    '''
    '''
    command = [ "/usr/bin/nm", "-Um", objectFileName ]

    output = subprocess.check_output(command)
    lines = output.splitlines()

    external = []
    internal = []
    for line in lines:
        fields = line.split(" ")
        if fields[1] == "(__TEXT,__text)":
            functionName = canonicalizeFunctionName(fields[3])

            if isReservedName(functionName):
                print "Ignoring function with a ISO/IEC 9899:2011 and ANSI Sec. 4.1.2.1 reserved name: ", functionName
            else:
                if fields[2] == "external":
                    external.append( ( functionName ) )
                else:
                    internal.append( ( functionName ) )
                    pass
            pass
    pass

    external.sort()
    internal.sort()
    return { "Static": internal, "Global" : external }

def testCases(functionList):
    '''
    '''
    return { "testCases" : functionList }

def testSuite(testCases):
    '''
    A Test Suite is comprised of one or more Test Cases
    '''
    if testCases == None or len(testCases) == 0:
        return None
    return [ testCases ]

def testFixture(testFixtureName, testSuites):
    '''
    A Test Fixture contains an initial setup function, one or more Test Suites, and a final tear-down function.
    '''
    if testSuites == None:
        return None
    return { "name" : testFixtureName, "testSuites" : testSuites }

def testRunner(testRunnerName, files, testFixtures):
    '''
    A Test Runner contains one or more Test Fixtures.
    '''
    testFixtures = [x for x in testFixtures if x is not None]
    return { "name" : testRunnerName, "files" : files, "testFixtures" : testFixtures }

def computeFileNames(argv):
    """ Given an argument list, compute the file names to use for code generation.


    """
    if (argv[1].endswith(".c")):
        return (argv[1], argv[2], sourceFileNameToName(argv[1]) + ".longbow")

    return (argv[1]+".c", argv[1]+".o", sourceFileNameToName(argv[1]) + ".longbow")

if __name__ == '__main__':
    '''
    @(#) longbow-preprocess @VERSION@ @DATE@
	@(#)   All Rights Reserved. Use is subject to license terms.
'''
    if len(sys.argv) <= 1:
        print "Usage: longbow-preprocess (sourceFileName objectFileName) | (fileNamePrefix)"
        print
        print "Generate a plain-text intermediate form for a LongBow test case generated from"
        print "a specified source and object file.  Use longbow-code to produce a LongBow"
        print "test runner based upon the intermediate form."
        sys.exit(1)

    fileNames = computeFileNames(sys.argv)

    sourceFileName = fileNames[0]
    objectFileName = fileNames[1]
    outputFileName = fileNames[2]

    functionDictionary = getDarwinTestableFunctions(sourceFileName, objectFileName)

    testRunnerName = sourceFileNameToName(sourceFileName)

    testFixtures = map(lambda(fixtureType):
                       testFixture(fixtureType, testSuite(testCases(functionDictionary[fixtureType]))), functionDictionary)

    files = { "sourceFile" : sourceFileName, "objectFile" : objectFileName }
    result = testRunner(testRunnerName, files, testFixtures)

    out = open(outputFileName, "w")
    pp = pprint.PrettyPrinter(indent=4, width=132, depth=None, stream=out)
    pp.pprint(result)
    out.close()
    pass
