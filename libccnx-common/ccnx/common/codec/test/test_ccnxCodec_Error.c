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
 */
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnxCodec_Error.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(tlv_Errors)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(tlv_Errors)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(tlv_Errors)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecError_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecError_GetByteOffset);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecError_GetErrorCode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecError_GetLine);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecError_GetFunction);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecError_GetErrorMessage);
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

LONGBOW_TEST_CASE(Global, ccnxCodecError_Create_Destroy)
{
    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_NO_ERROR, "apple", 10, 100);
    ccnxCodecError_Release(&error);
    assertTrue(parcMemory_Outstanding() == 0, "memory imbalance after create/destroy");
}

LONGBOW_TEST_CASE(Global, ccnxCodecError_GetByteOffset)
{
    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_NO_ERROR, "apple", 10, 100);
    assertTrue(ccnxCodecError_GetByteOffset(error) == 100,
               "Wrong offset, expected %u got %zu",
               100,
               ccnxCodecError_GetByteOffset(error));
    ccnxCodecError_Release(&error);
}

LONGBOW_TEST_CASE(Global, ccnxCodecError_GetErrorCode)
{
    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_NO_ERROR, "apple", 10, 100);
    assertTrue(ccnxCodecError_GetErrorCode(error) == TLV_ERR_NO_ERROR,
               "Wrong error code, expected %d got %d",
               TLV_ERR_NO_ERROR,
               ccnxCodecError_GetErrorCode(error));
    ccnxCodecError_Release(&error);
}

LONGBOW_TEST_CASE(Global, ccnxCodecError_GetLine)
{
    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_NO_ERROR, "apple", 10, 100);
    assertTrue(ccnxCodecError_GetLine(error) == 10,
               "Wrong line number, expected %d got %d",
               10,
               ccnxCodecError_GetLine(error));
    ccnxCodecError_Release(&error);
}

LONGBOW_TEST_CASE(Global, ccnxCodecError_GetFunction)
{
    char *apple = "apple";

    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_NO_ERROR, apple, 10, 100);
    assertTrue(ccnxCodecError_GetFunction(error) == apple,
               "Wrong function string, expected %p got %p",
               apple,
               ccnxCodecError_GetFunction(error));
    ccnxCodecError_Release(&error);
}

LONGBOW_TEST_CASE(Global, ccnxCodecError_GetErrorMessage)
{
    char *apple = "apple";
    const char *truth = ccnxCodecErrors_ErrorMessage(TLV_ERR_NO_ERROR);

    CCNxCodecError *error = ccnxCodecError_Create(TLV_ERR_NO_ERROR, apple, 10, 100);
    assertTrue(ccnxCodecError_GetErrorMessage(error) == truth,
               "Wrong function string, expected %p got %p",
               truth,
               ccnxCodecError_GetErrorMessage(error));
    ccnxCodecError_Release(&error);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(tlv_Errors);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
