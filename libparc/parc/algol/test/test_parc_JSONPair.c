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

#include "../parc_JSONPair.c"
#include <LongBow/unit-test.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.

#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>

#include "../parc_List.h"
#include "../parc_ArrayList.h"
#include "../parc_SafeMemory.h"
#include "../parc_Memory.h"
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_JSONPair)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(JSONPair);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_JSONPair)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_JSONPair)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(JSONPair)
{
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateAcquireRelease);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_BuildString);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_ToString);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_Display);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_Equals);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_Parser);

    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateNULL);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateValue);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateString);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateFromBoolean);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateFromInteger);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateFromFloat);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateFromJSONArray);
    LONGBOW_RUN_TEST_CASE(JSONPair, parcJSONPair_CreateFromJSON);
}

LONGBOW_TEST_FIXTURE_SETUP(JSONPair)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(JSONPair)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateAcquireRelease)
{
    PARCBuffer *name = parcBuffer_WrapCString("name");

    PARCBuffer *stringValue = parcBuffer_WrapCString("foo");
    PARCJSONValue *value = parcJSONValue_CreateFromString(stringValue);
    parcBuffer_Release(&stringValue);

    PARCJSONPair *pair = parcJSONPair_Create(name, value);

    assertTrue(parcBuffer_Equals(name, pair->name),
               "Expected '%s' actual '%s'", parcBuffer_ToString(name), parcBuffer_ToString(pair->name));
    assertTrue(value == pair->value,
               "Expected %p' actual %p", (void *) value, (void *) pair->value);

    PARCJSONPair *reference = parcJSONPair_Acquire(pair);

    assertTrue(reference == pair,
               "Expected parcJSONPair_Acquire to return the same pointer as the original.");

    parcBuffer_Release(&name);
    parcJSONValue_Release(&value);
    parcJSONPair_Release(&reference);
    parcJSONPair_Release(&pair);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_Display)
{
    PARCBuffer *name = parcBuffer_WrapCString("name");
    PARCBuffer *value = parcBuffer_WrapCString("foo");

    PARCJSONValue *jsonValue = parcJSONValue_CreateFromString(value);
    parcBuffer_Release(&value);
    PARCJSONPair *pair = parcJSONPair_Create(name, jsonValue);
    parcBuffer_Release(&name);
    parcJSONValue_Release(&jsonValue);

    parcJSONPair_Display(pair, 0);

    parcJSONPair_Release(&pair);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_BuildString)
{
    PARCBuffer *name = parcBuffer_WrapCString("name");
    PARCBuffer *value = parcBuffer_WrapCString("foo/bar");

    PARCJSONValue *jsonValue = parcJSONValue_CreateFromString(value);
    parcBuffer_Release(&value);
    PARCJSONPair *pair = parcJSONPair_Create(name, jsonValue);
    parcBuffer_Release(&name);
    parcJSONValue_Release(&jsonValue);

    // umcompressed
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcJSONPair_BuildString(pair, composer, false);
    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    char *expected = "\"name\" : \"foo\\/bar\"";
    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    // compressed
    composer = parcBufferComposer_Create();
    parcJSONPair_BuildString(pair, composer, true);
    tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    expected = "\"name\":\"foo/bar\"";
    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    parcJSONPair_Release(&pair);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_ToString)
{
    PARCBuffer *name = parcBuffer_WrapCString("name");
    PARCBuffer *value = parcBuffer_WrapCString("foo");

    PARCJSONValue *jsonValue = parcJSONValue_CreateFromString(value);
    parcBuffer_Release(&value);
    PARCJSONPair *pair = parcJSONPair_Create(name, jsonValue);
    parcBuffer_Release(&name);
    parcJSONValue_Release(&jsonValue);

    char *expected = "\"name\" : \"foo\"";
    char *actual = parcJSONPair_ToString(pair);

    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    parcJSONPair_Release(&pair);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateNULL)
{
    char *name = "MyNull";
    PARCBuffer *expectedName = parcBuffer_AllocateCString(name);

    PARCJSONPair *pair = parcJSONPair_CreateFromNULL(name);

    assertTrue(parcBuffer_Equals(expectedName, parcJSONPair_GetName(pair)),
               "Expected name '%s', got '%s'", name, parcBuffer_ToString(parcJSONPair_GetName(pair)));

    assertTrue(parcJSONValue_IsNull(parcJSONPair_GetValue(pair)),
               "Expected a JSON Null value.");

    parcJSONPair_Release(&pair);
    parcBuffer_Release(&expectedName);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateValue)
{
    char *name = "MyNull";
    PARCBuffer *expectedName = parcBuffer_AllocateCString(name);
    PARCJSONValue *value = parcJSONValue_CreateFromCString("Some Pig");

    PARCJSONPair *pair = parcJSONPair_CreateFromJSONValue(name, value);

    assertTrue(parcBuffer_Equals(expectedName, parcJSONPair_GetName(pair)),
               "Expected name '%s', got '%s'", name, parcBuffer_ToString(parcJSONPair_GetName(pair)));

    assertTrue(parcJSONValue_IsString(parcJSONPair_GetValue(pair)),
               "Expected a String value.");

    assertTrue(parcJSONPair_GetValue(pair) == value, "Expect values to be equal");

    parcJSONValue_Release(&value);
    parcJSONPair_Release(&pair);
    parcBuffer_Release(&expectedName);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateString)
{
    char *name = "MyNull";
    char *value = "value";
    PARCBuffer *expectedName = parcBuffer_AllocateCString(name);
    PARCJSONValue *expectedValue = parcJSONValue_CreateFromCString(value);

    PARCJSONPair *pair = parcJSONPair_CreateFromString(name, value);

    assertTrue(parcBuffer_Equals(expectedName, parcJSONPair_GetName(pair)),
               "Expected name '%s', got '%s'", name, parcBuffer_ToString(parcJSONPair_GetName(pair)));
    assertTrue(parcJSONValue_Equals(expectedValue, parcJSONPair_GetValue(pair)),
               "Expected value '%s', Got '%s'", value, parcBuffer_ToString(parcJSONValue_GetString(parcJSONPair_GetValue(pair))));
    assertTrue(parcJSONValue_IsString(parcJSONPair_GetValue(pair)),
               "Expected a JSON String value.");

    parcJSONPair_Release(&pair);
    parcBuffer_Release(&expectedName);
    parcJSONValue_Release(&expectedValue);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateFromBoolean)
{
    char *name = "MyNull";
    bool value = true;
    PARCBuffer *expectedName = parcBuffer_AllocateCString(name);
    PARCJSONValue *expectedValue = parcJSONValue_CreateFromBoolean(value);

    PARCJSONPair *pair = parcJSONPair_CreateFromBoolean(name, value);

    assertTrue(parcBuffer_Equals(expectedName, parcJSONPair_GetName(pair)),
               "Expected name '%s', got '%s'", name, parcBuffer_ToString(parcJSONPair_GetName(pair)));
    assertTrue(parcJSONValue_Equals(expectedValue, parcJSONPair_GetValue(pair)),
               "Expected value '%d', Got '%d'", value, parcJSONValue_GetBoolean(parcJSONPair_GetValue(pair)));

    assertTrue(parcJSONValue_IsBoolean(parcJSONPair_GetValue(pair)),
               "Expected a JSON Boolean value.");

    parcJSONPair_Release(&pair);
    parcBuffer_Release(&expectedName);
    parcJSONValue_Release(&expectedValue);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateFromInteger)
{
    char *name = "MyNull";
    int value = 31415;
    PARCBuffer *expectedName = parcBuffer_AllocateCString(name);
    PARCJSONValue *expectedValue = parcJSONValue_CreateFromInteger(value);

    PARCJSONPair *pair = parcJSONPair_CreateFromInteger(name, value);

    assertTrue(parcBuffer_Equals(expectedName, parcJSONPair_GetName(pair)),
               "Expected name '%s', got '%s'", name, parcBuffer_ToString(parcJSONPair_GetName(pair)));
    assertTrue(parcJSONValue_Equals(expectedValue, parcJSONPair_GetValue(pair)),
               "Expected value '%d', Got '%" PRIi64 "'", value, parcJSONValue_GetInteger(parcJSONPair_GetValue(pair)));

    assertTrue(parcJSONValue_IsNumber(parcJSONPair_GetValue(pair)),
               "Expected a JSON Integer value.");

    parcJSONPair_Release(&pair);
    parcBuffer_Release(&expectedName);
    parcJSONValue_Release(&expectedValue);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateFromFloat)
{
    char *name = "MyNull";
    double value = 3.1;
    PARCBuffer *expectedName = parcBuffer_AllocateCString(name);
    PARCJSONValue *expectedValue = parcJSONValue_CreateFromFloat(value);

    PARCJSONPair *pair = parcJSONPair_CreateFromDouble(name, value);

    assertTrue(parcBuffer_Equals(expectedName, parcJSONPair_GetName(pair)),
               "Expected name '%s', got '%s'", name, parcBuffer_ToString(parcJSONPair_GetName(pair)));
    assertTrue(parcJSONValue_Equals(expectedValue, parcJSONPair_GetValue(pair)),
               "Expected %g, got %Lg", value, parcJSONValue_GetFloat(parcJSONPair_GetValue(pair)));

    assertTrue(parcJSONValue_IsNumber(parcJSONPair_GetValue(pair)),
               "Expected a JSON number value.");

    parcJSONPair_Release(&pair);
    parcBuffer_Release(&expectedName);
    parcJSONValue_Release(&expectedValue);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateFromJSONArray)
{
    char *name = "MyNull";
    PARCJSONArray *array = parcJSONArray_Create();

    PARCBuffer *expectedName = parcBuffer_AllocateCString(name);
    PARCJSONValue *expectedValue = parcJSONValue_CreateFromJSONArray(array);

    PARCJSONPair *pair = parcJSONPair_CreateFromJSONArray(name, array);
    parcJSONArray_Release(&array);

    assertTrue(parcBuffer_Equals(expectedName, parcJSONPair_GetName(pair)),
               "Expected name '%s', got '%s'", name, parcBuffer_ToString(parcJSONPair_GetName(pair)));
    assertTrue(parcJSONValue_Equals(expectedValue, parcJSONPair_GetValue(pair)),
               "Expected the value to be equal the same array provided");

    assertTrue(parcJSONValue_IsArray(parcJSONPair_GetValue(pair)),
               "Expected a JSON Array value.");

    parcJSONPair_Release(&pair);
    parcBuffer_Release(&expectedName);
    parcJSONValue_Release(&expectedValue);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_CreateFromJSON)
{
    char *name = "MyNull";
    PARCJSON *value = parcJSON_Create();

    PARCBuffer *expectedName = parcBuffer_AllocateCString(name);
    PARCJSONValue *expectedValue = parcJSONValue_CreateFromJSON(value);

    PARCJSONPair *pair = parcJSONPair_CreateFromJSON(name, value);

    assertTrue(parcBuffer_Equals(expectedName, parcJSONPair_GetName(pair)),
               "Expected name '%s', got '%s'", name, parcBuffer_ToString(parcJSONPair_GetName(pair)));
    assertTrue(parcJSONValue_Equals(expectedValue, parcJSONPair_GetValue(pair)),
               "Expected %s", parcJSON_ToString(value));

    assertTrue(parcJSONValue_IsJSON(parcJSONPair_GetValue(pair)),
               "Expected a JSON Object value.");

    parcJSONPair_Release(&pair);
    parcBuffer_Release(&expectedName);
    parcJSONValue_Release(&expectedValue);
    parcJSON_Release(&value);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_Equals)
{
    char *name = "MyNull";
    char *unequalName = "foo";
    int value = 31415;
    int unequalValue = 141;

    PARCJSONPair *pair = parcJSONPair_CreateFromInteger(name, value);
    PARCJSONPair *y = parcJSONPair_CreateFromInteger(name, value);
    PARCJSONPair *z = parcJSONPair_CreateFromInteger(name, value);
    PARCJSONPair *unequal1 = parcJSONPair_CreateFromInteger(name, unequalValue);
    PARCJSONPair *unequal2 = parcJSONPair_CreateFromInteger(unequalName, unequalValue);

    parcObjectTesting_AssertEqualsFunction(parcJSONPair_Equals, pair, y, z, unequal1, unequal2);

    parcJSONPair_Release(&pair);
    parcJSONPair_Release(&y);
    parcJSONPair_Release(&z);
    parcJSONPair_Release(&unequal1);
    parcJSONPair_Release(&unequal2);
}

LONGBOW_TEST_CASE(JSONPair, parcJSONPair_Parser)
{
    PARCBuffer *buffer = parcBuffer_AllocateCString("\"name\" : \"value\"");

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONPair *pair = parcJSONPair_Parser(parser);

    assertTrue(parcBuffer_Position(parcJSONPair_GetName(pair)) == 0, "Expected the JSONPair name buffer to be 'reset'");

    parcJSONPair_Release(&pair);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_JSONPair);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
