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

#include <config.h>
#include <LongBow/unit-test.h>
#include <fcntl.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_SafeMemory.h"
#include "../parc_FileOutputStream.c"

#define PATH_SEGMENT "A"

LONGBOW_TEST_RUNNER(parc_FileOutputStream)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(AcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_FileOutputStream)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_FileOutputStream)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(AcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcFileOutputStream_Create);
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcFileOutputStream_Release);
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcFileOutputStream_AcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(AcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(AcquireRelease)
{
    unlink("/tmp/test_parc_FileOutputStream");
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(AcquireRelease, parcFileOutputStream_Create)
{
    PARCFileOutputStream *stream = parcFileOutputStream_Create(open("/tmp/test_parc_FileOutputStream", O_CREAT | O_WRONLY | O_TRUNC, 0600));
    assertNotNull(stream, "Expected a non-null pointer");

    parcFileOutputStream_Release(&stream);
    assertNull(stream, "Expected parcFileOutputStream_Release to null the pointer");
    unlink("/tmp/test_parc_FileOutputStream");
}

LONGBOW_TEST_CASE(AcquireRelease, parcFileOutputStream_Release)
{
    PARCFileOutputStream *stream = parcFileOutputStream_Create(open("/tmp/test_parc_FileOutputStream", O_CREAT | O_WRONLY | O_TRUNC, 0600));
    assertNotNull(stream, "Expected a non-null pointer");

    parcFileOutputStream_Release(&stream);
    assertNull(stream, "Expected parcFileOutputStream_Release to null the pointer");
    unlink("/tmp/test_parc_FileOutputStream");
}

LONGBOW_TEST_CASE(AcquireRelease, parcFileOutputStream_AcquireRelease)
{
    PARCFileOutputStream *stream = parcFileOutputStream_Create(open("/tmp/test_parc_FileOutputStream", O_CREAT | O_WRONLY | O_TRUNC, 0600));
    assertNotNull(stream, "Expected a non-null pointer");

    PARCFileOutputStream *reference = parcFileOutputStream_Acquire(stream);
    assertTrue(stream == reference, "Expected the reference to be equal to the original.");

    parcFileOutputStream_Release(&stream);
    assertNull(stream, "Expected parcFileOutputStream_Release to null the pointer");

    parcFileOutputStream_Release(&reference);
    unlink("/tmp/test_parc_FileOutputStream");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcFileOutputStream_Write);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    unlink("/tmp/test_parc_FileOutputStream");

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcFileOutputStream_Write)
{
    PARCFileOutputStream *stream =
        parcFileOutputStream_Create(open("/tmp/test_parc_FileOutputStream", O_CREAT | O_WRONLY | O_TRUNC, 0600));

    PARCBuffer *buffer = parcBuffer_Allocate(16 * 1024 * 1024);

    parcFileOutputStream_Write(stream, buffer);

    assertFalse(parcBuffer_HasRemaining(buffer), "Expected the buffer to be emtpy");

    parcBuffer_Release(&buffer);

    parcFileOutputStream_Release(&stream);
    unlink("/tmp/test_parc_FileOutputStream");
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
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_FileOutputStream);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
