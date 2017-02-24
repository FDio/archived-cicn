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

#include "../parc_JSONArray.c"

#include <LongBow/unit-test.h>
#include <stdio.h>

#include "../parc_List.h"
#include "../parc_ArrayList.h"
#include "../parc_SafeMemory.h"
#include "../parc_Memory.h"
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_JSONArray)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(parc_JSONArray);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_JSONArray)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_JSONArray)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(parc_JSONArray)
{
    LONGBOW_RUN_TEST_CASE(parc_JSONArray, parcJSONArray_CreateRelease);
    LONGBOW_RUN_TEST_CASE(parc_JSONArray, parcJSONArray_Equals);
    LONGBOW_RUN_TEST_CASE(parc_JSONArray, parcJSONArray_AddValue);
    LONGBOW_RUN_TEST_CASE(parc_JSONArray, parcJSONArray_GetLength);
    LONGBOW_RUN_TEST_CASE(parc_JSONArray, parcJSONArray_GetValue);
    LONGBOW_RUN_TEST_CASE(parc_JSONArray, parcJSONArray_BuildString);
    LONGBOW_RUN_TEST_CASE(parc_JSONArray, parcJSONArray_ToString);
    LONGBOW_RUN_TEST_CASE(parc_JSONArray, parcJSONArray_Display);
}

LONGBOW_TEST_FIXTURE_SETUP(parc_JSONArray)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(parc_JSONArray)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(parc_JSONArray, parcJSONArray_CreateRelease)
{
    PARCJSONArray *expected = parcJSONArray_Create();
    parcJSONArray_AssertValid(expected);
    assertNotNull(expected, "Expected non-null return value from parcJSONArray_Create");

    PARCJSONArray *actual = parcJSONArray_Acquire(expected);
    parcJSONArray_AssertValid(actual);

    parcJSONArray_Release(&actual);
    assertNull(actual, "Expected null value set by parcJSONArray_Release");
    parcJSONArray_AssertValid(expected);

    parcJSONArray_Release(&expected);
    assertNull(expected, "Expected null value set by parcJSONArray_Release");
}

LONGBOW_TEST_CASE(parc_JSONArray, parcJSONArray_Equals)
{
    PARCJSONArray *x = parcJSONArray_Create();
    PARCJSONArray *y = parcJSONArray_Create();
    PARCJSONArray *z = parcJSONArray_Create();

    PARCJSONArray *notEqual1 = parcJSONArray_Create();
    PARCJSONValue *value = parcJSONValue_CreateFromCString("Hello");
    parcJSONArray_AddValue(notEqual1, value);
    parcJSONValue_Release(&value);

    parcObjectTesting_AssertEqualsFunction(parcJSONArray_Equals, x, y, z, notEqual1);

    parcJSONArray_Release(&x);
    parcJSONArray_Release(&y);
    parcJSONArray_Release(&z);
    parcJSONArray_Release(&notEqual1);
}

LONGBOW_TEST_CASE(parc_JSONArray, parcJSONArray_AddValue)
{
    PARCJSONArray *expected = parcJSONArray_Create();
    PARCJSONValue *value = parcJSONValue_CreateFromInteger(10);
    parcJSONArray_AddValue(expected, value);
    parcJSONValue_Release(&value);

    parcJSONArray_Release(&expected);
}

LONGBOW_TEST_CASE(parc_JSONArray, parcJSONArray_GetLength)
{
    PARCJSONArray *expected = parcJSONArray_Create();
    PARCJSONValue *value = parcJSONValue_CreateFromInteger(10);
    parcJSONArray_AddValue(expected, value);
    parcJSONValue_Release(&value);
    assertTrue(parcJSONArray_GetLength(expected) == 1, "Expected a length of 1");

    parcJSONArray_Release(&expected);
}

LONGBOW_TEST_CASE(parc_JSONArray, parcJSONArray_GetValue)
{
    PARCJSONArray *array = parcJSONArray_Create();
    PARCJSONValue *expected = parcJSONValue_CreateFromInteger(10);
    parcJSONArray_AddValue(array, expected);

    PARCJSONValue *actual = parcJSONArray_GetValue(array, 0);

    assertTrue(expected == actual, "Expected different value");

    parcJSONValue_Release(&expected);
    parcJSONArray_Release(&array);
}

LONGBOW_TEST_CASE(parc_JSONArray, parcJSONArray_BuildString)
{
    PARCJSONArray *array = parcJSONArray_Create();
    PARCJSONValue *expected = parcJSONValue_CreateFromInteger(10);
    parcJSONArray_AddValue(array, expected);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcJSONArray_BuildString(array, composer, false);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    parcBufferComposer_Release(&composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    assertTrue(strlen(result) > 0, "Expected non-empty string result");

    parcMemory_Deallocate((void **) &result);

    composer = parcBufferComposer_Create();
    parcJSONArray_BuildString(array, composer, true);
    tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    parcBufferComposer_Release(&composer);
    result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    assertTrue(strlen(result) > 0, "Expected non-empty string result");

    parcMemory_Deallocate((void **) &result);

    parcJSONValue_Release(&expected);
    parcJSONArray_Release(&array);
}

LONGBOW_TEST_CASE(parc_JSONArray, parcJSONArray_ToString)
{
    PARCJSONArray *array = parcJSONArray_Create();
    PARCJSONValue *expected = parcJSONValue_CreateFromInteger(10);
    parcJSONArray_AddValue(array, expected);
    parcJSONValue_Release(&expected);

    const char *string = parcJSONArray_ToString(array);

    parcMemory_Deallocate((void **) &string);

    parcJSONArray_Release(&array);
}

LONGBOW_TEST_CASE(parc_JSONArray, parcJSONArray_Display)
{
    PARCJSONArray *array = parcJSONArray_Create();
    PARCJSONValue *expected = parcJSONValue_CreateFromInteger(10);
    parcJSONArray_AddValue(array, expected);
    parcJSONValue_Release(&expected);

    parcJSONArray_Display(array, 0);

    parcJSONArray_Release(&array);
}

LONGBOW_TEST_FIXTURE(Static)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_JSONArray);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
