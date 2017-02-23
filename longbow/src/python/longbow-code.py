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
'''
This programme takes a previously generated LongBow production (see longbow-preprocess.py) as input
and generates corresponding C code as a template for a complete test runner for that production.
'''

def sourceFileNameToShortName(sourceFileName):
    '''
    Given a path to a source file, return the name without any path components or suffix after the first '.' (inclusive).
    '''
    name = os.path.basename(sourceFileName)
    return name.split(".")[0]

def canonicalizeFunctionName(functionName):
    '''
    Given a function name that contains the initial '_' character,
    strip it and return a canonicalised form of the same name suitable for invoking from a C source file.

    This used to translate the typical function name mangling by the C compiler,
    where foo() becomes _foo in the object file symbol table.
    '''
    if functionName[0] == "_":
        functionName = functionName[1:]
    return functionName

class LongBowTestGenerator:
    def __init__(self, output=sys.stdout):
        self.output = output
        return

    def generateCode(self, testProduction):
        testRunnerName = testProduction["name"]
        sourceFileName = testProduction["files"]["sourceFile"]
        objectFileName = testProduction["files"]["objectFile"]

        self.filePrologue()
        self.testRunnerPrologue(sourceFileName, objectFileName, testRunnerName, testProduction["testFixtures"])

        for testFixture in testProduction["testFixtures"]:
            fixtures = self.generateTestFixture(testProduction, testFixture)
            pass

        self.testRunnerEpilogue(sourceFileName, objectFileName, testRunnerName, testProduction["testFixtures"])
        return

    def filePrologue(self):
        self.output.write("/*\n")
        self.output.write(" *\n")
        self.output.write(" */\n")
        self.output.write("\n")
        return

    def testRunnerPrologue(self, sourceFileName, objectFileName, testRunnerName, testFixtures):
        self.output.write("// Include the file(s) containing the functions to be tested.\n")
        self.output.write("// This permits internal static functions to be visible to this Test Runner.\n")
        self.output.write("#include \"%s\"\n" % (sourceFileName))
        self.output.write("\n")
        self.output.write("#include <LongBow/unit-test.h>\n")
        self.output.write("\n")
        self.output.write("LONGBOW_TEST_RUNNER(%s)\n" % (testRunnerName))
        self.output.write("{\n")
        self.output.write("    // The following Test Fixtures will run their corresponding Test Cases.\n")
        self.output.write("    // Test Fixtures are run in the order specified here, but every test must be idempotent.\n")
        self.output.write("    // Never rely on the execution order of tests or share state between them.\n")
        for testFixture in testFixtures:
            self.output.write("    LONGBOW_RUN_TEST_FIXTURE(%s);\n" % (testFixture["name"]))
            pass
        self.output.write("}\n")
        self.output.write("\n" )
        self.output.write("// The Test Runner calls this function once before any Test Fixtures are run.\n")
        self.output.write("LONGBOW_TEST_RUNNER_SETUP(%s)\n" % (testRunnerName))
        self.output.write("{\n")
        self.output.write("    return LONGBOW_STATUS_SUCCEEDED;\n")
        self.output.write("}\n")
        self.output.write("\n")
        self.output.write("// The Test Runner calls this function once after all the Test Fixtures are run.\n")
        self.output.write("LONGBOW_TEST_RUNNER_TEARDOWN(%s)\n" % (testRunnerName))
        self.output.write("{\n")
        self.output.write("    return LONGBOW_STATUS_SUCCEEDED;\n")
        self.output.write("}\n")
        self.output.write("\n")
        return

    def testRunnerEpilogue(self, sourceFileName, objectFileName, testRunnerName, testFixtures):
        self.output.write("int\n")
        self.output.write("main(int argc, char *argv[])\n")
        self.output.write("{\n")
        self.output.write("    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(%s);\n" % (testRunnerName))
        self.output.write("    int exitStatus = longBowMain(argc, argv, testRunner, NULL);\n");
        self.output.write("    longBowTestRunner_Destroy(&testRunner);\n");
        self.output.write("    exit(exitStatus);\n");
        self.output.write("}\n")
        return

    def generateTestFixture(self, testProduction, testFixture):
        testFixtureName = testFixture["name"]

        sourceFileName = testProduction["files"]["sourceFile"]
        objectFileName = testProduction["files"]["objectFile"]

        self.testFixturePrologue(sourceFileName, objectFileName, testFixtureName, testFixture["testSuites"])

        for testSuite in testFixture["testSuites"]:
            self.generateTestSuite(testProduction, testFixture, testSuite)
            pass

        self.testFixtureEpilogue(testProduction, testFixture, testSuite)
        return [ testFixtureName ]

    def testFixturePrologue(self, sourceFileName, objectFileName, testFixtureName, testSuites):
        self.output.write("LONGBOW_TEST_FIXTURE(%s)\n" % (testFixtureName))
        self.output.write("{\n")
        for testSuite in testSuites:
            for testCase in testSuite["testCases"]:
                self.output.write("    LONGBOW_RUN_TEST_CASE(%s, %s);\n" % (testFixtureName, testCase))
                pass
            pass
        self.output.write("}\n")
        self.output.write("\n")
        self.output.write("LONGBOW_TEST_FIXTURE_SETUP(%s)\n" % (testFixtureName))
        self.output.write("{\n")
        self.output.write("    return LONGBOW_STATUS_SUCCEEDED;\n")
        self.output.write("}\n")
        self.output.write("\n")
        self.output.write( "LONGBOW_TEST_FIXTURE_TEARDOWN(%s)\n" % (testFixtureName))
        self.output.write("{\n")
        self.output.write("    return LONGBOW_STATUS_SUCCEEDED;\n")
        self.output.write("}\n")
        self.output.write("\n")
        return

    def testFixtureEpilogue(self, testProduction, testFixture, testSuite):
        return

    def generateTestSuite(self, testProduction, testFixture, testSuite):
        for testCase in testSuite["testCases"]:
            self.generateTestCase(testProduction, testFixture, testCase)
        return

    def generateTestCase(self, testProduction, testFixture, testCase):
        self.output.write("LONGBOW_TEST_CASE(%s, %s)\n" % (testFixture["name"], testCase))
        self.output.write("{\n")
        self.output.write("    testUnimplemented(\"\");\n")
        self.output.write("}\n")
        self.output.write("\n")
        return

def getProductionSchema(fileName):
    '''
    Get the "production" schema produced by the preprocessor.
    '''
    f = open(fileName, "r")
    text = f.read()
    f.close()
    return eval(text)

def canonicalOutputFileName(production):
    outputFileName = "test_" + sourceFileNameToShortName(production["files"]["sourceFile"]) + ".c"
    return outputFileName

def canonicalOutput(outputFileName):
    if outputFileName == None:
        return sys.stdout
    open(outputFileName)

if __name__ == '__main__':
    '''
@(#) longbow-code @VERSION@ @DATE@
@(#)   All Rights Reserved. Use is subject to license terms.
    '''
    outputFileName = None

    if len(sys.argv) != 2:
        print "Usage: longbow-code file.longbow"
        sys.exit(1)

    production = getProductionSchema(sys.argv[1])

    if outputFileName == None:
        outputFileName = canonicalOutputFileName(production)

    if os.path.exists(outputFileName):
        print "Refusing to overwrite the existing '%s'." % (outputFileName)
        sys.exit(1)

    outputFile = open(outputFileName, 'w')

    generator = LongBowTestGenerator(outputFile)
    generator.generateCode(production)
    pass
