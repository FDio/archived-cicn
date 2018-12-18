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

#include "../parc_JSON.c"
#include "../parc_JSONPair.c"

#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>

#include <LongBow/unit-test.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.


#include "../parc_List.h"
#include "../parc_ArrayList.h"
#include "../parc_SafeMemory.h"
#include "../parc_Memory.h"
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_JSON)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(JSON);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_JSON)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_JSON)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(JSON)
{
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_CreateRelease);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_Equals);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_HashCode);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_Copy);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_Add);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_GetMembers);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_GetPairByName);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_GetValueByName);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_GetPairByIndex);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_GetValueByIndex);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_BuildString);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_ToString);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_ToCompactString);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_GetByPath);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_GetByPath_BadArrayIndex);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_GetByPath_DeadEndPath);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_ParseString);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_ParseBuffer_WithExcess);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_Display);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_AddString);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_AddObject);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_AddInteger);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_AddBoolean);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_AddArray);
    LONGBOW_RUN_TEST_CASE(JSON, parcJSON_AddValue);
}

typedef struct {
    PARCJSON *json;
    char *expected;
    char *compactExpected;
} TestData;

LONGBOW_TEST_FIXTURE_SETUP(JSON)
{
    TestData *data = parcMemory_Allocate(sizeof(TestData));

    char *temp = "{ \"string\" : \"foo\\/bar\", \"null\" : null, \"true\" : true, \"false\" : false, \"integer\" : 31415, \"float\" : 3.141500, \"json\" : { \"string\" : \"foo\\/bar\" }, \"array\" : [ null, false, true, 31415, \"string\", [ null, false, true, 31415, \"string\" ], {  } ] }";
    data->expected = parcMemory_StringDuplicate(temp, strlen(temp));

    temp = "{\"string\":\"foo/bar\",\"null\":null,\"true\":true,\"false\":false,\"integer\":31415,\"float\":3.141500,\"json\":{\"string\":\"foo/bar\"},\"array\":[null,false,true,31415,\"string\",[null,false,true,31415,\"string\"],{}]}";
    data->compactExpected = parcMemory_StringDuplicate(temp, strlen(temp));

    data->json = parcJSON_ParseString(temp);

    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(JSON)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    parcJSON_Release(&data->json);
    parcMemory_Deallocate(&data->expected);
    parcMemory_Deallocate(&data->compactExpected);

    parcMemory_Deallocate(&data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_CASE(JSON, parcJSON_CreateRelease)
{
    PARCJSON *json = parcJSON_Create();

    parcJSON_Release(&json);
    assertNull(json, "Expected the NULL pointer side-effect of Release.");
}

LONGBOW_TEST_CASE(JSON, parcJSON_Copy)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCJSON *copy = parcJSON_Copy(data->json);

    assertTrue(parcJSON_Equals(data->json, copy), "Expect copy to equal original");

    parcJSON_Release(&copy);
}

LONGBOW_TEST_CASE(JSON, parcJSON_HashCode)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCHashCode expected = parcHashCode_Hash((uint8_t *) data->compactExpected, strlen(data->compactExpected));

    PARCHashCode hashCode = parcJSON_HashCode(data->json);

    assertTrue(hashCode == expected, "Expect correct hash code");
}

LONGBOW_TEST_CASE(JSON, parcJSON_Add)
{
    PARCJSON *json = parcJSON_Create();
    {
        PARCBuffer *string = parcBuffer_WrapCString("string");
        PARCJSONValue *stringValue = parcJSONValue_CreateFromString(string);
        PARCBuffer *stringName = parcBuffer_WrapCString("string");
        PARCJSONPair *pair = parcJSONPair_Create(stringName, stringValue);
        parcJSON_AddPair(json, pair);
        parcJSONPair_Release(&pair);
        parcBuffer_Release(&stringName);
        parcJSONValue_Release(&stringValue);
        parcBuffer_Release(&string);
    }
    {
        PARCBuffer *name = parcBuffer_WrapCString("null");
        PARCJSONValue *value = parcJSONValue_CreateFromNULL();
        PARCJSONPair *pair = parcJSONPair_Create(name, value);
        parcJSON_AddPair(json, pair);
        parcJSONPair_Release(&pair);
        parcBuffer_Release(&name);
        parcJSONValue_Release(&value);
    }
    {
        PARCBuffer *name = parcBuffer_WrapCString("true");
        PARCJSONValue *value = parcJSONValue_CreateFromBoolean(true);
        PARCJSONPair *pair = parcJSONPair_Create(name, value);
        parcJSON_AddPair(json, pair);
        parcJSONPair_Release(&pair);
        parcBuffer_Release(&name);
        parcJSONValue_Release(&value);
    }
    {
        PARCBuffer *name = parcBuffer_WrapCString("false");
        PARCJSONValue *value = parcJSONValue_CreateFromBoolean(false);
        PARCJSONPair *pair = parcJSONPair_Create(name, value);
        parcJSON_AddPair(json, pair);
        parcJSONPair_Release(&pair);
        parcBuffer_Release(&name);
        parcJSONValue_Release(&value);
    }
    {
        PARCBuffer *name = parcBuffer_WrapCString("integer");
        PARCJSONValue *value = parcJSONValue_CreateFromInteger(31415);
        PARCJSONPair *pair = parcJSONPair_Create(name, value);
        parcJSON_AddPair(json, pair);
        parcJSONPair_Release(&pair);
        parcBuffer_Release(&name);
        parcJSONValue_Release(&value);
    }
    {
        PARCBuffer *name = parcBuffer_WrapCString("float");
        PARCJSONValue *value = parcJSONValue_CreateFromFloat(3.1415);
        PARCJSONPair *pair = parcJSONPair_Create(name, value);
        parcJSON_AddPair(json, pair);
        parcJSONPair_Release(&pair);
        parcBuffer_Release(&name);
        parcJSONValue_Release(&value);
    }

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSON, parcJSON_GetMembers)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    char *s = parcJSON_ToString(data->json);
    parcMemory_Deallocate((void **) &s);

    PARCList *members = parcJSON_GetMembers(data->json);
    assertTrue(parcList_Size(members) == 8, "Expected 8, actual %zd", parcList_Size(members));
}

LONGBOW_TEST_CASE(JSON, parcJSON_GetPairByName)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int expected = 31415;
    const PARCJSONPair *pair = parcJSON_GetPairByName(data->json, "integer");

    PARCBuffer *name = parcJSONPair_GetName(pair);
    PARCJSONValue *value = parcJSONPair_GetValue(pair);

    int64_t actual = parcJSONValue_GetInteger(value);

    PARCBuffer *expectedName = parcBuffer_WrapCString("integer");

    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'integer', actual '%s'", (char *) parcBuffer_ToString(name));

    assertTrue(expected == actual, "Expected %d, actual %" PRIi64 "", expected, actual);

    parcBuffer_Release(&expectedName);
}

LONGBOW_TEST_CASE(JSON, parcJSON_GetValueByName)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    int expected = 31415;
    const PARCJSONValue *value = parcJSON_GetValueByName(data->json, "integer");

    int64_t actual = parcJSONValue_GetInteger(value);

    PARCBuffer *expectedName = parcBuffer_WrapCString("integer");

    assertTrue(expected == actual, "Expected %d, actual %" PRIi64 "", expected, actual);

    parcBuffer_Release(&expectedName);
}

LONGBOW_TEST_CASE(JSON, parcJSON_GetPairByIndex)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCJSONPair *pair = parcJSON_GetPairByIndex(data->json, 0);
    PARCBuffer *name = parcJSONPair_GetName(pair);
    PARCBuffer *expectedName = parcBuffer_WrapCString("string");
    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'string', actual '%s'", (char *) parcBuffer_ToString(name));
    parcBuffer_Release(&expectedName);

    pair = parcJSON_GetPairByIndex(data->json, 1);
    name = parcJSONPair_GetName(pair);
    expectedName = parcBuffer_WrapCString("null");
    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'null', actual '%s'", (char *) parcBuffer_ToString(name));
    parcBuffer_Release(&expectedName);

    pair = parcJSON_GetPairByIndex(data->json, 2);
    name = parcJSONPair_GetName(pair);
    expectedName = parcBuffer_WrapCString("true");
    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'true', actual '%s'", (char *) parcBuffer_ToString(name));
    parcBuffer_Release(&expectedName);

    pair = parcJSON_GetPairByIndex(data->json, 3);
    name = parcJSONPair_GetName(pair);
    expectedName = parcBuffer_WrapCString("false");
    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'false', actual '%s'", (char *) parcBuffer_ToString(name));
    parcBuffer_Release(&expectedName);

    pair = parcJSON_GetPairByIndex(data->json, 4);
    name = parcJSONPair_GetName(pair);
    expectedName = parcBuffer_WrapCString("integer");
    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'integer', actual '%s'", (char *) parcBuffer_ToString(name));
    parcBuffer_Release(&expectedName);

    pair = parcJSON_GetPairByIndex(data->json, 5);
    name = parcJSONPair_GetName(pair);
    expectedName = parcBuffer_WrapCString("float");
    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'float', actual '%s'", (char *) parcBuffer_ToString(name));
    parcBuffer_Release(&expectedName);

    pair = parcJSON_GetPairByIndex(data->json, 6);
    name = parcJSONPair_GetName(pair);
    expectedName = parcBuffer_WrapCString("json");
    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'json', actual '%s'", (char *) parcBuffer_ToString(name));
    parcBuffer_Release(&expectedName);

    pair = parcJSON_GetPairByIndex(data->json, 7);
    name = parcJSONPair_GetName(pair);
    expectedName = parcBuffer_WrapCString("array");
    assertTrue(parcBuffer_Equals(expectedName, name),
               "Expected 'array', actual '%s'", (char *) parcBuffer_ToString(name));
    parcBuffer_Release(&expectedName);
}

LONGBOW_TEST_CASE(JSON, parcJSON_GetValueByIndex)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCJSONValue *value = parcJSON_GetValueByIndex(data->json, 0);
    assertTrue(parcJSONValue_IsString(value),
               "Expected value to be type string");

    value = parcJSON_GetValueByIndex(data->json, 1);
    assertTrue(parcJSONValue_IsNull(value),
               "Expected value to be type string");

    value = parcJSON_GetValueByIndex(data->json, 2);
    assertTrue(parcJSONValue_IsBoolean(value),
               "Expected value to be type string");

    value = parcJSON_GetValueByIndex(data->json, 3);
    assertTrue(parcJSONValue_IsBoolean(value),
               "Expected value to be type string");

    value = parcJSON_GetValueByIndex(data->json, 4);
    assertTrue(parcJSONValue_IsNumber(value),
               "Expected value to be type string");

    value = parcJSON_GetValueByIndex(data->json, 5);
    assertTrue(parcJSONValue_IsNumber(value),
               "Expected value to be type string");

    value = parcJSON_GetValueByIndex(data->json, 6);
    assertTrue(parcJSONValue_IsJSON(value),
               "Expected value to be type string");

    value = parcJSON_GetValueByIndex(data->json, 7);
    assertTrue(parcJSONValue_IsArray(value),
               "Expected value to be type string");
}

LONGBOW_TEST_CASE(JSON, parcJSON_BuildString)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcJSON_BuildString(data->json, composer, false);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    assertTrue(strcmp(data->expected, actual) == 0, "Expected %s, actual %s", data->expected, actual);
    parcMemory_Deallocate((void **) &actual);

    composer = parcBufferComposer_Create();
    parcJSON_BuildString(data->json, composer, true);

    tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    assertTrue(strcmp(data->compactExpected, actual) == 0, "Expected %s, actual %s", data->compactExpected, actual);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(JSON, parcJSON_ToString)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    char *actual = parcJSON_ToString(data->json);
    assertTrue(strcmp(data->expected, actual) == 0, "Expected %s, actual %s", data->expected, actual);

    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(JSON, parcJSON_ToCompactString)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    char *actual = parcJSON_ToCompactString(data->json);
    assertTrue(strcmp(data->compactExpected, actual) == 0, "Expected %s, actual %s", data->expected, actual);

    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(JSON, parcJSON_GetByPath)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    PARCJSON *json = data->json;

    char *s = parcJSON_ToString(json);
    printf("%s\n", s);
    parcMemory_Deallocate((void **) &s);

    const PARCJSONValue *value = parcJSON_GetByPath(json, "/string");
    assertTrue(parcJSONValue_IsString(value), "Expected /string to be a string type.");
    value = parcJSON_GetByPath(json, "/null");
    assertTrue(parcJSONValue_IsNull(value), "Expected /null to be a null type.");
    value = parcJSON_GetByPath(json, "/true");
    assertTrue(parcJSONValue_IsBoolean(value), "Expected /true to be a boolean type.");
    value = parcJSON_GetByPath(json, "/integer");
    assertTrue(parcJSONValue_IsNumber(value), "Expected /integer to be a number type.");
    value = parcJSON_GetByPath(json, "/float");
    assertTrue(parcJSONValue_IsNumber(value), "Expected /float to be a number type.");
    value = parcJSON_GetByPath(json, "/array");
    assertTrue(parcJSONValue_IsArray(value), "Expected /array to be an array type.");
    value = parcJSON_GetByPath(json, "/nonexistent");
    assertNull(value, "Expected /nonexistent to be NULL");

    value = parcJSON_GetByPath(json, "/array/1");
    assertTrue(parcJSONValue_IsBoolean(value), "Expected /array/0 to be a boolean type.");

    value = parcJSON_GetByPath(json, "/array/5");
    assertTrue(parcJSONValue_IsArray(value), "Expected /array/5 to be an array type.");

    assertNotNull(value, "Expected non-null pair");
}

LONGBOW_TEST_CASE(JSON, parcJSON_GetByPath_BadArrayIndex)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const PARCJSONValue *value = parcJSON_GetByPath(data->json, "/array/100");
    assertNull(value, "Expected null value return from parcJSON_GetByPath");
}

LONGBOW_TEST_CASE(JSON, parcJSON_GetByPath_DeadEndPath)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const PARCJSONValue *value = parcJSON_GetByPath(data->json, "/string/foo");
    assertNull(value, "Expected null value return from parcJSON_GetByPath");
}

LONGBOW_TEST_CASE(JSON, parcJSON_Equals)
{
    PARCJSON *x = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");
    PARCJSON *y = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");
    PARCJSON *z = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");

    PARCJSON *notEqual1 = parcJSON_ParseString("{ \"string\" : \"string\" }");

    PARCJSON *notEqual2 = parcJSON_ParseString("{ \"string\" : \"xyzzy\", \"integer\" : 1 }");

    parcObjectTesting_AssertEqualsFunction(parcJSON_Equals, x, y, z, notEqual1, notEqual2);

    parcJSON_Release(&x);
    parcJSON_Release(&y);
    parcJSON_Release(&z);
    parcJSON_Release(&notEqual1);
    parcJSON_Release(&notEqual2);
}

LONGBOW_TEST_CASE(JSON, parcJSON_Display)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    parcJSON_Display(data->json, 0);
}

LONGBOW_TEST_CASE(JSON, parcJSON_ParseString)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCJSON *json = parcJSON_ParseString(data->expected);

    char *actual = parcJSON_ToString(json);

    assertTrue(strcmp(data->expected, actual) == 0, "Expected %s, actual %s", data->expected, actual);

    parcMemory_Deallocate((void **) &actual);

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSON, parcJSON_ParseBuffer_WithExcess)
{
    char *string = "{ \"string\" : \"string\", \"null\" : null, \"true\" : true, \"false\" : false, \"integer\" : 31415, \"float\" : 3.141500, \"array\" : [ null, false, true, 31415, \"string\", [ null, false, true, 31415, \"string\" ], {  } ] }Xhowdy";
    PARCBuffer *buffer = parcBuffer_WrapCString((char *) string);

    PARCJSON *json = parcJSON_ParseBuffer(buffer);

    char actual = parcBuffer_GetUint8(buffer);
    assertTrue(actual == 'X', "Expected buffer position to point to X, actual %x", actual);

    parcBuffer_Release(&buffer);
    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSON, parcJSON_AddString)
{
    PARCJSON *json = parcJSON_Create();

    char *expectedName = "string";
    char *expectedValue = "value";

    parcJSON_AddString(json, expectedName, expectedValue);

    const PARCJSONPair *pair = parcJSON_GetPairByName(json, "string");
    PARCBuffer *actualName = parcJSONPair_GetName(pair);
    PARCJSONValue *actualValue = parcJSONPair_GetValue(pair);

    assertTrue(strcmp(expectedName, parcBuffer_Overlay(actualName, 0)) == 0,
               "Expected name %s, actual %s",
               expectedName,
               parcBuffer_ToString(actualName));
    assertTrue(strcmp(expectedValue, parcBuffer_Overlay(parcJSONValue_GetString(actualValue), 0)) == 0,
               "Expected value %s, actual %s",
               expectedValue,
               parcJSONValue_ToString(actualValue));

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSON, parcJSON_AddObject)
{
    PARCJSON *json = parcJSON_Create();

    PARCJSON *expectedValue = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");
    parcJSON_AddObject(json, "object", expectedValue);

    char *expectedName = "object";
    const PARCJSONPair *pair = parcJSON_GetPairByName(json, expectedName);

    PARCBuffer *actualName = parcJSONPair_GetName(pair);
    PARCJSONValue *actualValue = parcJSONPair_GetValue(pair);

    assertTrue(strcmp(expectedName, parcBuffer_Overlay(actualName, 0)) == 0,
               "Expected name %s, actual %s", expectedName, (char *) parcBuffer_ToString(actualName));

    assertTrue(parcJSON_Equals(expectedValue, parcJSONValue_GetJSON(actualValue)),
               "Expected value did not match the actual value.");

    parcJSON_Release(&expectedValue);
    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSON, parcJSON_AddInteger)
{
    PARCJSON *json = parcJSON_Create();

    char *expectedName = "integer";
    uint64_t expectedValue = 12345;

    parcJSON_AddInteger(json, expectedName, expectedValue);

    const PARCJSONPair *pair = parcJSON_GetPairByName(json, expectedName);

    PARCBuffer *actualName = parcJSONPair_GetName(pair);
    PARCJSONValue *actualValue = parcJSONPair_GetValue(pair);

    assertTrue(strcmp(expectedName, parcBuffer_Overlay(actualName, 0)) == 0,
               "Expected name %s, actual %s", expectedName, (char *) parcBuffer_ToString(actualName));

    assertTrue(expectedValue == parcJSONValue_GetInteger(actualValue),
               "Expected %" PRIi64 "d actual %" PRIi64 "d", expectedValue, parcJSONValue_GetInteger(actualValue));

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSON, parcJSON_AddBoolean)
{
    PARCJSON *json = parcJSON_Create();

    char *expectedName = "boolean";
    bool expectedValue = true;

    parcJSON_AddBoolean(json, expectedName, expectedValue);

    const PARCJSONPair *pair = parcJSON_GetPairByName(json, expectedName);

    PARCBuffer *actualName = parcJSONPair_GetName(pair);
    PARCJSONValue *actualValue = parcJSONPair_GetValue(pair);

    assertTrue(strcmp(expectedName, parcBuffer_Overlay(actualName, 0)) == 0,
               "Expected name %s, actual %s", expectedName, (char *) parcBuffer_ToString(actualName));

    assertTrue(expectedValue == parcJSONValue_GetBoolean(actualValue),
               "Expected %d actual %d", expectedValue, parcJSONValue_GetBoolean(actualValue));

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSON, parcJSON_AddArray)
{
    PARCJSON *json = parcJSON_Create();

    char *expectedName = "array";

    PARCJSONArray *array = parcJSONArray_Create();
    PARCJSONValue *value = parcJSONValue_CreateFromCString("Some Pig");
    parcJSONArray_AddValue(array, value);

    parcJSON_AddArray(json, expectedName, array);
    parcJSONArray_Release(&array);

    const PARCJSONPair *pair = parcJSON_GetPairByName(json, expectedName);

    PARCBuffer *actualName = parcJSONPair_GetName(pair);
    PARCJSONValue *actualValue = parcJSONPair_GetValue(pair);
    assertTrue(strcmp(expectedName, parcBuffer_Overlay(actualName, 0)) == 0,
               "Expected name %s, actual %s", expectedName, (char *) parcBuffer_ToString(actualName));
    assertTrue(parcJSONValue_IsArray(actualValue), "Expect value to be type PARCJSONArray");
    array = parcJSONValue_GetArray(actualValue);
    PARCJSONValue *result = parcJSONArray_GetValue(array, 0);
    assertTrue(parcBuffer_Equals(parcJSONValue_GetString(result), parcJSONValue_GetString(value)),
               "Expected %s actual %s",
               parcJSONValue_ToString(value),
               parcJSONValue_ToString(result));

    parcJSONValue_Release(&value);

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSON, parcJSON_AddValue)
{
    PARCJSON *json = parcJSON_Create();

    char *expectedName = "value";

    PARCJSONValue *value = parcJSONValue_CreateFromCString("Some Pig");

    parcJSON_AddValue(json, expectedName, value);

    const PARCJSONPair *pair = parcJSON_GetPairByName(json, expectedName);

    PARCBuffer *actualName = parcJSONPair_GetName(pair);
    PARCJSONValue *actualValue = parcJSONPair_GetValue(pair);
    assertTrue(strcmp(expectedName, parcBuffer_Overlay(actualName, 0)) == 0,
               "Expected name %s, actual %s", expectedName, (char *) parcBuffer_ToString(actualName));
    assertTrue(parcJSONValue_IsString(actualValue), "Expect value to be type PARCJSONArray");
    assertTrue(parcBuffer_Equals(parcJSONValue_GetString(actualValue), parcJSONValue_GetString(value)),
               "Expected %s actual %s",
               parcJSONValue_ToString(value),
               parcJSONValue_ToString(actualValue));

    parcJSONValue_Release(&value);

    parcJSON_Release(&json);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_JSON);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
