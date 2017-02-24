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
// This permits internal static functions to be visible to this Test Framework.
#include "../metis_StreamBuffer.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(metis_StreamBuffer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_StreamBuffer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_StreamBuffer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisStreamBuffer_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisStreamBuffer_DisableCallbacks);
    LONGBOW_RUN_TEST_CASE(Global, metisStreamBuffer_EnableCallbacks);
    LONGBOW_RUN_TEST_CASE(Global, metisStreamBuffer_Flush);
    LONGBOW_RUN_TEST_CASE(Global, metisStreamBuffer_FlushCheckpoint);
    LONGBOW_RUN_TEST_CASE(Global, metisStreamBuffer_FlushFinished);
    LONGBOW_RUN_TEST_CASE(Global, metisStreamBuffer_SetCallbacks);
    LONGBOW_RUN_TEST_CASE(Global, metisStreamBuffer_SetWatermark);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisStreamBuffer_Destroy)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisStreamBuffer_DisableCallbacks)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisStreamBuffer_EnableCallbacks)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisStreamBuffer_Flush)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisStreamBuffer_FlushCheckpoint)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisStreamBuffer_FlushFinished)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisStreamBuffer_SetCallbacks)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisStreamBuffer_SetWatermark)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_StreamBuffer);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
