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

/**
 * @header <#Headline Name#>
 * @abstract <#Abstract#>
 * @discussion
 *     <#Discussion#>
 *
 */
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_FileInputStream.c"

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_File.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(test_parc_FileInputStream)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_FileInputStream)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_FileInputStream)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcFileInputStream_Open);
    LONGBOW_RUN_TEST_CASE(Global, parcFileInputStream_ReadFile);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcFileInputStream_Open)
{
    PARCFile *file = parcFile_Create("test_parc_FileInputStream");
    PARCFileInputStream *stream = parcFileInputStream_Open(file);

    parcFileInputStream_Release(&stream);
    parcFile_Release(&file);
}

LONGBOW_TEST_CASE(Global, parcFileInputStream_ReadFile)
{
    PARCFile *file = parcFile_Create("test_parc_FileInputStream");
    PARCFileInputStream *stream = parcFileInputStream_Open(file);

    PARCBuffer *actual = parcFileInputStream_ReadFile(stream);
    assertNotNull(actual, "Expected non-null result from parcFileInputStream_ReadFile");

    parcBuffer_Flip(actual);

    assertTrue(parcBuffer_HasRemaining(actual), "Expected the buffer to contain data.");

    parcBuffer_Release(&actual);
    parcFileInputStream_Release(&stream);
    parcFile_Release(&file);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_FileInputStream);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
