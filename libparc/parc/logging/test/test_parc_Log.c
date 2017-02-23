/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** *
 */
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Log.c"

#include <parc/logging/parc_LogLevel.h>
#include <parc/logging/parc_LogReporterFile.h>

#include <parc/algol/parc_FileOutputStream.h>
#include <parc/algol/parc_SafeMemory.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

LONGBOW_TEST_RUNNER(test_parc_Log)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateDestroy);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_Log)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_Log)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

uint32_t CreationInitialMemoryOutstanding = 0;

LONGBOW_TEST_FIXTURE(CreateDestroy)
{
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcLog_Create);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, parcLog_Create_DefaultValues);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateDestroy)
{
    CreationInitialMemoryOutstanding = parcMemory_Outstanding();

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateDestroy)
{
    if (parcMemory_Outstanding() != CreationInitialMemoryOutstanding) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("'%s' leaks memory by %u\n",
               longBowTestCase_GetName(testCase), parcMemory_Outstanding() - CreationInitialMemoryOutstanding);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateDestroy, parcLog_Create)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(output);
    parcOutputStream_Release(&output);

    PARCLog *log = parcLog_Create("localhost", "test_parc_Log", NULL, reporter);
    parcLogReporter_Release(&reporter);

    assertTrue(parcLogLevel_Equals(parcLog_GetLevel(log), PARCLogLevel_Off), "Expected initial log level to be OFF");

    parcLog_Release(&log);
}

LONGBOW_TEST_CASE(CreateDestroy, parcLog_Create_DefaultValues)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(output);
    parcOutputStream_Release(&output);

    PARCLog *log = parcLog_Create(NULL, NULL, NULL, reporter);
    parcLogReporter_Release(&reporter);

    assertTrue(parcLogLevel_Equals(parcLog_GetLevel(log), PARCLogLevel_Off), "Expected initial log level to be OFF");

    parcLog_Release(&log);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Emergency);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Warning);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Alert);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Critical);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Error);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Notice);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Debug);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Info);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Message);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_IsLoggable_True);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_IsLoggable_False);

    LONGBOW_RUN_TEST_CASE(Global, parcLog_Emergency_WrongLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Warning_WrongLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Alert_WrongLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Critical_WrongLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Error_WrongLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Notice_WrongLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Debug_WrongLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLog_Info_WrongLevel);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    CreationInitialMemoryOutstanding = parcMemory_Outstanding();

    {
        PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
        PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
        parcFileOutputStream_Release(&fileOutput);

        PARCLogReporter *reporter = parcLogReporterFile_Create(output);
        parcOutputStream_Release(&output);

        PARCLog *log = parcLog_Create("localhost", "test_parc_Log", NULL, reporter);
        parcLogReporter_Release(&reporter);

        parcLog_SetLevel(log, PARCLogLevel_All);

        longBowTestCase_SetClipBoardData(testCase, log);
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    {
        PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
        parcLog_Release(&log);
    }

    if (parcMemory_Outstanding() != CreationInitialMemoryOutstanding) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("'%s' leaks memory by %u\n",
               longBowTestCase_GetName(testCase), parcMemory_Outstanding() - CreationInitialMemoryOutstanding);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcLog_IsLoggable_True)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_IsLoggable(log, PARCLogLevel_Alert), "Expected parcLog_IsLoggable to be true.");
}

LONGBOW_TEST_CASE(Global, parcLog_IsLoggable_False)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    parcLog_SetLevel(log, PARCLogLevel_Off);

    assertFalse(parcLog_IsLoggable(log, PARCLogLevel_Alert), "Expected parcLog_IsLoggable to be true.");
}

LONGBOW_TEST_CASE(Global, parcLog_Info)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_Info(log, "This is a info message"),
               "Expected message to be logged successfully");
}

LONGBOW_TEST_CASE(Global, parcLog_Warning)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_Warning(log, "This is a warning message"),
               "Expected message to be logged successfully");
}

LONGBOW_TEST_CASE(Global, parcLog_Emergency)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_Emergency(log, "This is an emergency message"),
               "Expected message to be logged successfully");
}

LONGBOW_TEST_CASE(Global, parcLog_Alert)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_Alert(log, "This is a alert message"),
               "Expected message to be logged successfully");
}

LONGBOW_TEST_CASE(Global, parcLog_Critical)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_Critical(log, "This is a critical message"),
               "Expected message to be logged successfully");
}

LONGBOW_TEST_CASE(Global, parcLog_Notice)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_Notice(log, "This is a notice message"),
               "Expected message to be logged successfully");
}

LONGBOW_TEST_CASE(Global, parcLog_Error)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_Error(log, "This is a error message"),
               "Expected message to be logged successfully");
}

LONGBOW_TEST_CASE(Global, parcLog_Debug)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);

    assertTrue(parcLog_Debug(log, "This is a debug message"),
               "Expected message to be logged successfully");
}

LONGBOW_TEST_CASE(Global, parcLog_Warning_WrongLevel)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Off);

    assertFalse(parcLog_Warning(log, "This is a warning message"),
                "Expected message to not be logged");
}

LONGBOW_TEST_CASE(Global, parcLog_Emergency_WrongLevel)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Off);

    // Even if the log level is set to off, you cannot block an emergency message.
    assertTrue(parcLog_Emergency(log, "This is an emergency message"),
               "Expected message to not be logged");
}

LONGBOW_TEST_CASE(Global, parcLog_Alert_WrongLevel)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Off);

    assertFalse(parcLog_Alert(log, "This is a finest message"),
                "Expected message to not be logged");
}

LONGBOW_TEST_CASE(Global, parcLog_Critical_WrongLevel)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Off);

    assertFalse(parcLog_Critical(log, "This is a finer message"),
                "Expected message to not be logged");
}

LONGBOW_TEST_CASE(Global, parcLog_Notice_WrongLevel)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Off);

    assertFalse(parcLog_Notice(log, "This is a fine message"),
                "Expected message to not be logged");
}

LONGBOW_TEST_CASE(Global, parcLog_Debug_WrongLevel)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Off);

    assertFalse(parcLog_Debug(log, "This is a debug message"),
                "Expected message to not be logged");
}

LONGBOW_TEST_CASE(Global, parcLog_Error_WrongLevel)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Off);

    assertFalse(parcLog_Error(log, "This is a debug message"),
                "Expected message to not be logged");
}

LONGBOW_TEST_CASE(Global, parcLog_Info_WrongLevel)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Off);

    assertFalse(parcLog_Info(log, "This is a debug message"),
                "Expected message to not be logged");
}

LONGBOW_TEST_CASE(Global, parcLog_Message)
{
    PARCLog *log = (PARCLog *) longBowTestCase_GetClipBoardData(testCase);
    parcLog_SetLevel(log, PARCLogLevel_Alert);

    assertTrue(parcLog_Message(log, PARCLogLevel_Alert, 0, "This is an alert message"),
               "Expected message to be logged");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_Log);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
