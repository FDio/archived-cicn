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


// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Runner.
#include "../parc_LogReporter.c"

#include <LongBow/unit-test.h>

#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_FileOutputStream.h>
#include <parc/logging/parc_LogReporterFile.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_LogReporter)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_LogReporter)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_LogReporter)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcLogReporter_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcLogReporter_Create_NULLObject);
    LONGBOW_RUN_TEST_CASE(Global, parcLogReporter_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parcLogReporter_GetPrivateObject);
    LONGBOW_RUN_TEST_CASE(Global, parcLogReporter_Report);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcLogReporter_AcquireRelease)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *out = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(out);

    parcObjectTesting_AssertAcquireReleaseContract(parcLogReporter_Acquire, reporter);

    parcOutputStream_Release(&out);

    assertNull(out, "Expected null value.");

    parcLogReporter_Release(&reporter);
}

LONGBOW_TEST_CASE(Global, parcLogReporter_Create)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *out = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(out);

    parcOutputStream_Release(&out);

    assertNull(out, "Expected null value.");

    parcLogReporter_Release(&reporter);
}

void
testReporter(PARCLogReporter *reporter, const PARCLogEntry *entry)
{
}

LONGBOW_TEST_CASE(Global, parcLogReporter_Create_NULLObject)
{
    PARCLogReporter *reporter = parcLogReporter_Create(NULL, NULL, testReporter, NULL);

    parcLogReporter_Release(&reporter);
}

LONGBOW_TEST_CASE(Global, parcLogReporter_GetPrivateObject)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *out = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(out);
    parcOutputStream_Release(&out);

    void *instance = parcLogReporter_GetPrivateObject(reporter);
    assertNotNull(instance, "Expected the instance pointer to be non-NULL");

    parcLogReporter_Release(&reporter);
}

LONGBOW_TEST_CASE(Global, parcLogReporter_Report)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *out = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(out);
    parcOutputStream_Release(&out);

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 1234, timeStamp, payload);

    parcLogReporter_Report(reporter, entry);
    parcLogEntry_Release(&entry);
    parcBuffer_Release(&payload);

    assertNull(out, "Expected null value.");

    parcLogReporter_Release(&reporter);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_LogReporter);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
