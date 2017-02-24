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

#include "../parc_JSONValue.c"
#include <LongBow/unit-test.h>

#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>

#include "../parc_List.h"
#include "../parc_ArrayList.h"
#include "../parc_SafeMemory.h"
#include "../parc_Memory.h"

#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_JSONValue)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(JSONValue_CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(JSONValue);
    LONGBOW_RUN_TEST_FIXTURE(JSONValueParsing);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_JSONValue)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_JSONValue)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(JSONValue_CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(JSONValue_CreateAcquireRelease, _createValue);
    LONGBOW_RUN_TEST_CASE(JSONValue_CreateAcquireRelease, parcJSONValue_AcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(JSONValue_CreateAcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(JSONValue_CreateAcquireRelease)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(JSONValue_CreateAcquireRelease, _createValue)
{
    PARCJSONValue *result = parcJSONValue_CreateFromNULL();

    assertNotNull(result, "Expected non-null return value from _createValue");
    assertTrue(parcJSONValue_IsNull(result),
               "Expected PARCJSONValueType_Null");

    parcJSONValue_Release(&result);
    assertNull(result, "Expected parcJSONValue_Release to NULL the instance pointer.");
}


LONGBOW_TEST_CASE(JSONValue_CreateAcquireRelease, parcJSONValue_AcquireRelease)
{
    PARCJSONValue *result = parcJSONValue_CreateFromNULL();

    assertNotNull(result, "Expected non-null return value from _createValue");
    assertTrue(parcJSONValue_IsNull(result),
               "Expected PARCJSONValueType_Null");

    PARCJSONValue *actual = parcJSONValue_Acquire(result);
    assertTrue(result == actual, "Expected parcJSONValue_Acquire return value to be same as the original.");

    parcJSONValue_Release(&actual);
    parcJSONValue_Release(&result);
    assertNull(result, "Expected parcJSONValue_Release to NULL the instance pointer.");
}


LONGBOW_TEST_FIXTURE(JSONValue)
{
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_NULL);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_Boolean);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_Float);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_Integer);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_String);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_JSON);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_Array);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_Timespec);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Create_Timeval);

    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_IsValid);

    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Display);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_BuildString);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_ToString_NULL);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_ToString_Array);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_ToString_Boolean);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_ToString_Float);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_ToString_Integer);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_ToString_String);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_CreateCString);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_ToString_JSON);

    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Equals_NULL);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Equals_Boolean);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Equals_Integer);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Equals_Float);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Equals_String);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Equals_Object);
    LONGBOW_RUN_TEST_CASE(JSONValue, parcJSONValue_Equals_Array);
}

LONGBOW_TEST_FIXTURE_SETUP(JSONValue)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(JSONValue)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_IsValid)
{
    bool actual = parcJSONValue_IsValid(NULL);
    assertFalse(actual, "Expected a NULL value to be invalid");

    PARCJSONValue *value = parcJSONValue_CreateFromNULL();

    actual = parcJSONValue_IsValid(value);
    parcJSONValue_Release(&value);
    assertTrue(actual, "Expected a NULL value to be invalid");
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_JSON)
{
    PARCJSON *json = parcJSON_Create();
    PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);

    assertTrue(parcJSONValue_IsJSON(value),
               "Expected PARCJSONValueType_JSON");

    assertTrue(parcJSONValue_GetJSON(value) == json,
               "Expected parcJSONValue_GetJSON to return the original instance pointer.");
    parcJSONValue_Release(&value);
    assertNull(value, "Expected NULL pointer.");
    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_Timeval)
{
    struct timeval timeval = { .tv_sec = 42, .tv_usec = 23 };
    PARCJSONValue *value = parcJSONValue_CreateFromTimeval(&timeval);

    assertTrue(parcJSONValue_IsJSON(value),
               "Expected PARCJSONValueType_JSON");

    struct timeval actual;
    parcJSONValue_GetTimeval(value, &actual);
    assertTrue(timeval.tv_sec == actual.tv_sec, "Expected seconds to be equal.");
    assertTrue(timeval.tv_usec == actual.tv_usec, "Expected seconds to be equal.");

    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_Timespec)
{
    struct timespec timespec = { .tv_sec = 42, .tv_nsec = 23 };
    PARCJSONValue *value = parcJSONValue_CreateFromTimespec(&timespec);

    assertTrue(parcJSONValue_IsJSON(value),
               "Expected PARCJSONValueType_JSON");

    struct timespec testTS;
    parcJSONValue_GetTimespec(value, &testTS);
    assertTrue(memcmp(&timespec, &testTS, sizeof(struct timespec)) == 0,
               "Expected parcJSONValue_GetTimespec to return the original instance pointer.");
    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_NULL)
{
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();

    assertTrue(value->type == PARCJSONValueType_Null,
               "Expected PARCJSONValueType_Null, actual %d", value->type);
    assertTrue(parcJSONValue_IsNull(value),
               "Expected PARCJSONValueType_Null");
    parcJSONValue_Release(&value);
    assertNull(value, "Expected NULL pointer.");
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_Boolean)
{
    bool expected = true;
    PARCJSONValue *value = parcJSONValue_CreateFromBoolean(expected);

    assertTrue(value->type == PARCJSONValueType_Boolean,
               "Expected PARCJSONValueType_BooleanON_VALUE_BOOLEAN, actual %d", value->type);
    assertTrue(value->value.boolean == expected, "Expected %d actual %d", expected, value->value.boolean);

    assertTrue(parcJSONValue_IsBoolean(value),
               "Expected PARCJSONValueType_Boolean");
    assertTrue(parcJSONValue_GetBoolean(value), "Expected value to be true");
    parcJSONValue_Release(&value);
    assertNull(value, "Expected NULL pointer.");
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_Float)
{
    double expected = 3.1415;
    PARCJSONValue *value = parcJSONValue_CreateFromFloat(expected);

    assertTrue(parcJSONValue_IsNumber(value),
               "Expected parcJSONValue_IsNumber to be true.");
    assertTrue(parcJSONValue_GetFloat(value) == expected,
               "Expected %g, actual %Lg", expected, parcJSONValue_GetFloat(value));

    char *expectedString = "3.141500";
    char *actualString = parcJSONValue_ToString(value);
    assertTrue(strcmp(expectedString, actualString) == 0, "Exepcted %s, actual %s", expectedString, actualString);
    parcMemory_Deallocate((void **) &actualString);

    parcJSONValue_Release(&value);
    assertNull(value, "Expected NULL pointer.");
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_Integer)
{
    int expected = 31415;
    PARCJSONValue *value = parcJSONValue_CreateFromInteger(expected);

    assertTrue(parcJSONValue_IsNumber(value),
               "Expected parcJSONValue_IsNumber");
    int64_t actual = parcJSONValue_GetInteger(value);
    assertTrue(expected == actual, "Expected %d, actual %" PRIi64 "", expected, actual);
    parcJSONValue_Release(&value);
    assertNull(value, "Expected NULL pointer.");
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_String)
{
    PARCBuffer *expected = parcBuffer_WrapCString("31415");
    PARCJSONValue *value = parcJSONValue_CreateFromString(expected);

    assertTrue(value->type == PARCJSONValueType_String,
               "Expected parcJSONValueType.String, actual %d", value->type);
    assertTrue(parcBuffer_Equals(value->value.string, expected),
               "Expected %s actual %s", parcBuffer_ToString(expected), parcBuffer_ToString(value->value.string));
    assertTrue(parcJSONValue_IsString(value),
               "Expected PARCJSONValueType_String");

    assertTrue(parcBuffer_Equals(parcJSONValue_GetString(value), expected), "Expected value did not match actual value");
    parcJSONValue_Release(&value);
    assertNull(value, "Expected NULL pointer.");
    parcBuffer_Release(&expected);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_CreateCString)
{
    PARCBuffer *expected = parcBuffer_WrapCString("31415");

    PARCJSONValue *value = parcJSONValue_CreateFromCString("31415");

    assertTrue(value->type == PARCJSONValueType_String,
               "Expected parcJSONValueType.String, actual %d", value->type);

    assertTrue(parcBuffer_Equals(parcJSONValue_GetString(value), expected), "Assert:")
    {
        char *expectedString = parcBuffer_ToString(expected);
        char *actualString = parcBuffer_ToString(parcJSONValue_GetString(value));
        printf("Expected '%s', actual '%s'", expectedString, actualString);
        parcMemory_Deallocate((void **) &expectedString);
        parcMemory_Deallocate((void **) &actualString);
    }

    parcJSONValue_Release(&value);
    assertNull(value, "Expected NULL pointer.");
    parcBuffer_Release(&expected);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Create_Array)
{
    PARCJSONArray *array = parcJSONArray_Create();

    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromJSONArray(array);

    parcJSONValue_Release(&value);

    parcJSONArray_Release(&array);
    assertNull(value, "Expected NULL pointer.");
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_BuildString)
{
    PARCJSONArray *array = parcJSONArray_Create();

    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromBoolean(false);
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromBoolean(true);
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromInteger(31415);
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    PARCBuffer *stringValue = parcBuffer_WrapCString("stringA/stringB");
    value = parcJSONValue_CreateFromString(stringValue);
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);
    parcBuffer_Release(&stringValue);

    value = parcJSONValue_CreateFromJSONArray(array);
    parcJSONArray_Release(&array);

    // Uncompacted
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcJSONValue_BuildString(value, composer, false);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    char *expected = "[ null, false, true, 31415, \"stringA\\/stringB\" ]";

    assertTrue(strcmp(actual, expected) == 0,
               "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);

    // Compacted
    composer = parcBufferComposer_Create();
    parcJSONValue_BuildString(value, composer, true);

    tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    actual = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    expected = "[null,false,true,31415,\"stringA/stringB\"]";

    assertTrue(strcmp(actual, expected) == 0,
               "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);


    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_ToString_Array)
{
    PARCJSONArray *array = parcJSONArray_Create();

    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromBoolean(false);
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromBoolean(true);
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromInteger(31415);
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);

    PARCBuffer *stringValue = parcBuffer_WrapCString("stringA/stringB");
    value = parcJSONValue_CreateFromString(stringValue);
    parcJSONArray_AddValue(array, value);
    parcJSONValue_Release(&value);
    parcBuffer_Release(&stringValue);

    value = parcJSONValue_CreateFromJSONArray(array);
    parcJSONArray_Release(&array);

    char *expected = "[ null, false, true, 31415, \"stringA\\/stringB\" ]";
    char *actual = parcJSONValue_ToString(value);

    assertTrue(strcmp(actual, expected) == 0,
               "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Display)
{
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();
    parcJSONValue_Display(value, 0);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromBoolean(true);
    parcJSONValue_Display(value, 0);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromCString("hello");
    parcJSONValue_Display(value, 0);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromFloat(3.14);
    parcJSONValue_Display(value, 0);
    parcJSONValue_Release(&value);

    value = parcJSONValue_CreateFromInteger(314);
    parcJSONValue_Display(value, 0);
    parcJSONValue_Release(&value);

    PARCJSONArray *array = parcJSONArray_Create();
    value = parcJSONValue_CreateFromJSONArray(array);
    parcJSONValue_Display(value, 0);
    parcJSONValue_Release(&value);
    parcJSONArray_Release(&array);

    PARCJSON *json = parcJSON_Create();
    value = parcJSONValue_CreateFromJSON(json);
    parcJSONValue_Display(value, 0);
    parcJSONValue_Release(&value);
    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_ToString_NULL)
{
    char *expected = "null";
    PARCJSONValue *value = parcJSONValue_CreateFromNULL();

    char *actual = parcJSONValue_ToString(value);
    assertTrue(strcmp(actual, expected) == 0, "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_ToString_Boolean)
{
    char *expected = "true";
    PARCJSONValue *value = parcJSONValue_CreateFromBoolean(expected);

    char *actual = parcJSONValue_ToString(value);
    assertTrue(strcmp(actual, expected) == 0, "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_ToString_Float)
{
    struct test_values {
        char *string;
        long double value;
        long double error;
    } successful[] = {
        { "-0.0415e-12", -0.0415e-12, 0.00001e-12 },
        { "-0.0415e12",  -0.0415e12,  0.00001e12  },
        { "-0.0415",     -0.0415,     0.00001     },
        { "-3.0415",     -3.0415,     0.00001     },
        { "123.456",     123.456,     0.0001      },
        { "123.456e78",  123.456e78,  0.0001e78   },
        { "123.456e-78", 123.456e-78, 0.0001e-78  },
        { "123.456e-78", 123.456e-78, 0.0001e-78  },
        { "4e1",         40.0,        0.0001e-78  },
        { NULL },
    };

    for (int i = 0; successful[i].string != NULL; i++) {
        PARCBuffer *buffer = parcBuffer_WrapCString(successful[i].string);
        PARCJSONParser *parser = parcJSONParser_Create(buffer);
        parcBuffer_Release(&buffer);

        PARCJSONValue *expected = _parcJSONValue_NumberParser(parser);

        char *string = parcJSONValue_ToString(expected);
        assertTrue(strcmp(successful[i].string, string) == 0,
                   "Expected %s, actual %s", successful[i].string, string);
        parcMemory_Deallocate((void **) &string);

        parcJSONValue_Release(&expected);
        parcJSONParser_Release(&parser);
    }
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_ToString_Integer)
{
    char *expected = "31415";
    PARCJSONValue *value = parcJSONValue_CreateFromInteger(31415);

    char *actual = parcJSONValue_ToString(value);
    assertTrue(strcmp(actual, expected) == 0,
               "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_ToString_String)
{
    char *input = "31415\b";
    char *expected = "\"31415\\b\"";

    PARCBuffer *stringValue = parcBuffer_WrapCString(input);
    PARCJSONValue *value = parcJSONValue_CreateFromString(stringValue);
    parcBuffer_Release(&stringValue);

    char *actual = parcJSONValue_ToString(value);
    assertTrue(strcmp(actual, expected) == 0, "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_ToString_JSON)
{
    char *expected = "{  }";
    PARCJSON *json = parcJSON_Create();
    PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
    parcJSON_Release(&json);

    char *actual = parcJSONValue_ToString(value);
    assertTrue(strcmp(actual, expected) == 0, "Expected '%s', actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);
    parcJSONValue_Release(&value);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Equals_NULL)
{
    PARCJSONValue *example = parcJSONValue_CreateFromNULL();

    PARCJSONValue *equal1 = parcJSONValue_CreateFromNULL();
    PARCJSONValue *equal2 = parcJSONValue_CreateFromNULL();

    PARCBuffer *stringBuffer = parcBuffer_AllocateCString("Hello");
    PARCJSONValue *string = parcJSONValue_CreateFromString(stringBuffer);
    parcBuffer_Release(&stringBuffer);

    parcObjectTesting_AssertEqualsFunction(parcJSONValue_Equals, example, equal1, equal2, string);

    parcJSONValue_Release(&string);
    parcJSONValue_Release(&equal2);
    parcJSONValue_Release(&equal1);
    parcJSONValue_Release(&example);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Equals_Boolean)
{
    PARCJSONValue *example = parcJSONValue_CreateFromBoolean(true);

    PARCJSONValue *equal1 = parcJSONValue_CreateFromBoolean(true);
    PARCJSONValue *equal2 = parcJSONValue_CreateFromBoolean(true);

    PARCJSONValue *unequal1 = parcJSONValue_CreateFromBoolean(false);

    PARCBuffer *stringBuffer = parcBuffer_AllocateCString("Hello");
    PARCJSONValue *string = parcJSONValue_CreateFromString(stringBuffer);
    parcBuffer_Release(&stringBuffer);

    parcObjectTesting_AssertEqualsFunction(parcJSONValue_Equals, example, equal1, equal2, unequal1, string);

    parcJSONValue_Release(&string);
    parcJSONValue_Release(&unequal1);
    parcJSONValue_Release(&equal2);
    parcJSONValue_Release(&equal1);
    parcJSONValue_Release(&example);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Equals_Integer)
{
    PARCJSONValue *example = parcJSONValue_CreateFromInteger(31415);

    PARCJSONValue *equal1 = parcJSONValue_CreateFromInteger(31415);
    PARCJSONValue *equal2 = parcJSONValue_CreateFromInteger(31415);

    PARCJSONValue *unequal1 = parcJSONValue_CreateFromInteger(4);

    PARCBuffer *stringBuffer = parcBuffer_AllocateCString("Hello");
    PARCJSONValue *string = parcJSONValue_CreateFromString(stringBuffer);
    parcBuffer_Release(&stringBuffer);

    parcObjectTesting_AssertEqualsFunction(parcJSONValue_Equals, example, equal1, equal2, unequal1, string);

    parcJSONValue_Release(&string);
    parcJSONValue_Release(&unequal1);
    parcJSONValue_Release(&equal2);
    parcJSONValue_Release(&equal1);
    parcJSONValue_Release(&example);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Equals_Float)
{
    PARCJSONValue *example = parcJSONValue_CreateFromFloat(3.1415);

    PARCJSONValue *equal1 = parcJSONValue_CreateFromFloat(3.1415);
    PARCJSONValue *equal2 = parcJSONValue_CreateFromFloat(3.1415);

    PARCJSONValue *unequal1 = parcJSONValue_CreateFromFloat(4.0);

    PARCBuffer *stringBuffer = parcBuffer_AllocateCString("Hello");
    PARCJSONValue *string = parcJSONValue_CreateFromString(stringBuffer);
    parcBuffer_Release(&stringBuffer);

    parcObjectTesting_AssertEqualsFunction(parcJSONValue_Equals, example, equal1, equal2, unequal1, string);

    parcJSONValue_Release(&string);
    parcJSONValue_Release(&unequal1);
    parcJSONValue_Release(&equal2);
    parcJSONValue_Release(&equal1);
    parcJSONValue_Release(&example);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Equals_String)
{
    PARCBuffer *stringBuffer = parcBuffer_AllocateCString("Hello");
    PARCJSONValue *example = parcJSONValue_CreateFromString(stringBuffer);
    parcBuffer_Release(&stringBuffer);

    stringBuffer = parcBuffer_AllocateCString("Hello");
    PARCJSONValue *equal1 = parcJSONValue_CreateFromString(stringBuffer);
    parcBuffer_Release(&stringBuffer);

    stringBuffer = parcBuffer_AllocateCString("Hello");
    PARCJSONValue *equal2 = parcJSONValue_CreateFromString(stringBuffer);
    parcBuffer_Release(&stringBuffer);

    PARCJSONValue *unequal1 = parcJSONValue_CreateFromFloat(4.0);

    stringBuffer = parcBuffer_AllocateCString("World");
    PARCJSONValue *string = parcJSONValue_CreateFromString(stringBuffer);
    parcBuffer_Release(&stringBuffer);

    parcObjectTesting_AssertEqualsFunction(parcJSONValue_Equals, example, equal1, equal2, unequal1, string);

    parcJSONValue_Release(&string);
    parcJSONValue_Release(&unequal1);
    parcJSONValue_Release(&equal2);
    parcJSONValue_Release(&equal1);
    parcJSONValue_Release(&example);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Equals_Object)
{
    char *string = "{ \"name\" : 1, \"name2\" : 2 }";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);
    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);

    PARCJSONValue *x = parcJSONValue_ObjectParser(parser);
    parcJSONParser_Release(&parser);

    assertTrue(parcJSONValue_IsJSON(x), "Expected a JSON Object value.");

    buffer = parcBuffer_WrapCString(string);
    parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);

    PARCJSONValue *y = parcJSONValue_ObjectParser(parser);
    parcJSONParser_Release(&parser);

    buffer = parcBuffer_WrapCString(string);
    parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);

    PARCJSONValue *z = parcJSONValue_ObjectParser(parser);
    parcJSONParser_Release(&parser);

    PARCJSONValue *unequal1 = parcJSONValue_CreateFromFloat(4.0);

    PARCJSON *json = parcJSON_Create();
    PARCJSONValue *unequal2 = parcJSONValue_CreateFromJSON(json);
    parcJSON_Release(&json);

    parcObjectTesting_AssertEqualsFunction(parcJSONValue_Equals, x, y, z, unequal1, unequal2);

    parcJSONValue_Release(&x);
    parcJSONValue_Release(&y);
    parcJSONValue_Release(&z);
    parcJSONValue_Release(&unequal1);
    parcJSONValue_Release(&unequal2);
}

LONGBOW_TEST_CASE(JSONValue, parcJSONValue_Equals_Array)
{
    char *string = "[ \"name\", 1, true, false, null, [ ], { } ]";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);
    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);

    PARCJSONValue *x = _parcJSONValue_ArrayParser(parser);
    parcJSONParser_Release(&parser);

    assertTrue(parcJSONValue_IsArray(x), "Expected a JSON Array value.");

    buffer = parcBuffer_WrapCString(string);
    parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);

    PARCJSONValue *y = _parcJSONValue_ArrayParser(parser);
    parcJSONParser_Release(&parser);

    buffer = parcBuffer_WrapCString(string);
    parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);

    PARCJSONValue *z = _parcJSONValue_ArrayParser(parser);
    parcJSONParser_Release(&parser);

    PARCJSONValue *unequal1 = parcJSONValue_CreateFromFloat(4.0);

    PARCJSONArray *array = parcJSONArray_Create();
    PARCJSONValue *unequal2 = parcJSONValue_CreateFromJSONArray(array);
    parcJSONArray_Release(&array);

    parcObjectTesting_AssertEqualsFunction(parcJSONValue_Equals, x, y, z, unequal1, unequal2);

    parcJSONValue_Release(&x);
    parcJSONValue_Release(&y);
    parcJSONValue_Release(&z);
    parcJSONValue_Release(&unequal1);
    parcJSONValue_Release(&unequal2);
}

LONGBOW_TEST_FIXTURE(JSONValueParsing)
{
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, _parcJSONValue_NullParser);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, _parcJSONValue_NullParser_Bad);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, _parcJSONValue_TrueParser);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, _parcJSONValue_TrueParser_Bad);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, _parcJSONValue_FalseParser);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, _parcJSONValue_FalseParser_Bad);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, _parcJSONValue_StringParser);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_ObjectParser);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_ObjectParser_Bad_Pair);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_ObjectParser_Bad_Pair2);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_ArrayParser);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, _parcJSONValue_StringParser_BAD);

    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_NumberParser_BatchedFloat);

    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_Comma);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_CloseBracket);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_Null);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_True);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_False);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_String);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_Array);
    LONGBOW_RUN_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_Object);
}

LONGBOW_TEST_FIXTURE_SETUP(JSONValueParsing)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(JSONValueParsing)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(JSONValueParsing, _parcJSONValue_NullParser)
{
    char *string = "null";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = _parcJSONValue_NullParser(parser);

    assertTrue(parcJSONValue_IsNull(actual), "Expected a JSON Null value.");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, _parcJSONValue_NullParser_Bad)
{
    char *string = "nulx";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = _parcJSONValue_NullParser(parser);

    assertNull(actual, "Expected a NULL return value");

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, _parcJSONValue_TrueParser)
{
    char *string = "true";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = _parcJSONValue_TrueParser(parser);

    assertTrue(parcJSONValue_IsBoolean(actual), "Expected a JSON Boolean value.");
    assertTrue(parcJSONValue_GetBoolean(actual), "Expected true.");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, _parcJSONValue_TrueParser_Bad)
{
    char *string = "trux";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = _parcJSONValue_TrueParser(parser);

    assertNull(actual, "Expected a NULL return value");

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, _parcJSONValue_FalseParser)
{
    char *string = "false";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = _parcJSONValue_FalseParser(parser);

    assertTrue(parcJSONValue_IsBoolean(actual), "Expected a JSON Boolean value.");
    assertFalse(parcJSONValue_GetBoolean(actual), "Expected false.");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, _parcJSONValue_FalseParser_Bad)
{
    char *string = "falsx";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = _parcJSONValue_FalseParser(parser);

    assertNull(actual, "Expected a NULL return value");

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, _parcJSONValue_StringParser)
{
    char *parserInput = "\"\\\" \\\\ \\b \\f \\n \\r \\t \\/\"";
    PARCBuffer *buffer = parcBuffer_WrapCString(parserInput);
    PARCBuffer *expected = parcBuffer_AllocateCString("\" \\ \b \f \n \r \t /");

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = _parcJSONValue_StringParser(parser);

    assertTrue(parcJSONValue_IsString(actual),
               "Expected a JSON String value.");

    assertTrue(parcBuffer_Equals(expected, actual->value.string),
               "Expected '%s' actual '%s'", parcBuffer_ToString(expected), parcBuffer_ToString(actual->value.string))
    {
        parcBuffer_Display(expected, 0);
        parcBuffer_Display(actual->value.string, 0);
    }

    char *string = parcJSONValue_ToString(actual);
    assertTrue(strcmp(parserInput, string) == 0,
               "Expected %s, actual %s", parserInput, string);

    parcMemory_Deallocate((void **) &string);

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, _parcJSONValue_StringParser_BAD)
{
    char *bad[] = {
        "\"\t\"",
        "\"",
        NULL
    };

    for (int i = 0; bad[i] != NULL; i++) {
        char *parserInput = bad[i];
        PARCBuffer *buffer = parcBuffer_WrapCString(parserInput);
        PARCJSONParser *parser = parcJSONParser_Create(buffer);
        PARCJSONValue *actual = _parcJSONValue_StringParser(parser);

        assertNull(actual, "Expected failure");
        parcBuffer_Release(&buffer);
        parcJSONParser_Release(&parser);
    }
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_ObjectParser)
{
    char *string = "{ \"name\" : 1, \"name2\" : 2 }";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_ObjectParser(parser);

    assertTrue(parcJSONValue_IsJSON(actual), "Expected a JSON Object value.");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_ObjectParser_Bad_Pair)
{
    char *string = "{ \"name\" , \"name2\" : 2 }";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_ObjectParser(parser);

    assertNull(actual, "Expected parcJSONValue_ObjectParser to return NULL indicating failure");

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_ObjectParser_Bad_Pair2)
{
    char *string = "{ 2 }";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_ObjectParser(parser);

    assertNull(actual, "Expected parcJSONValue_ObjectParser to return NULL indicating failure");

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_ArrayParser)
{
    char *string = "[ \"name\", 1, true, false, null, [ ], { } ]";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = _parcJSONValue_ArrayParser(parser);

    assertTrue(parcJSONValue_IsArray(actual), "Expected a JSON Array value.");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_NumberParser_BatchedFloat)
{
    struct test_values {
        char *string;
        char *expectedString;
        long double floatValue;
        int64_t integerValue;
        long double floatTolerance;
    } successful[] = {
        { "0",           "0",           0.0,         0,              0           },
        { " 1",          "1",           1.0,         1,              0           },
        { "-1",          "-1",          -1.0,        -1,             0           },
        { "1e1",         "1e1",         1.0e1,       10,             0           },
        { "-2e1",        "-2e1",        -2.0e1,      -2e1,           0           },
        { "-2e+1",       "-2e1",        -2.0e+1,     -2e+1,          0           },
        { " 1.0",        "1",           1.0,         1,              0           },
        { "3e-1",        "3e-1",        3e-1,        0,              0.01e-1     },
        { "100e-2",      "100e-2",      100e-2,      100e-2,         0.0001      },
        { "123.456e11",  "123.456e11",  123.456e11,  12345600000000, 0.0001e11   },
        { "-0.0415e-12", "-0.0415e-12", -0.0415e-12, 0,              0.00001e-12 },
        { "-0.0415e12",  "-0.0415e12",  -0.0415e12,  -41500000000,   0.00001e12  },
        { "-0.0415",     "-0.0415",     -0.0415,     0,              0.00001     },
        { "-3.0415",     "-3.0415",     -3.0415,     -3,             0.00001     },
        { "123.456",     "123.456",     123.456,     123,            0.0001      },
        { "123.456e+11", "123.456e11",  123.456e+11, 12345600000000, 0.0001e+11  },
        { "123.456e-11", "123.456e-11", 123.456e-11, 0,              0.0001e-11  },
        { "1e-1",        "1e-1",        1e-1,        0,              0.1e-1      },
        { NULL },
    };

    for (int i = 0; successful[i].string != NULL; i++) {
        PARCBuffer *buffer = parcBuffer_WrapCString(successful[i].string);
        PARCJSONParser *parser = parcJSONParser_Create(buffer);
        parcBuffer_Release(&buffer);

        PARCJSONValue *expected = _parcJSONValue_NumberParser(parser);
        assertNotNull(expected, "_parcJSONValue_NumberParser returned NULL");

        long double floatValue = parcJSONValue_GetFloat(expected);

        assertTrue(fabsl(floatValue - successful[i].floatValue) <= successful[i].floatTolerance,
                   "Expected %Lf actual %Lf", successful[i].floatValue, floatValue);

        char *string = parcJSONValue_ToString(expected);
        assertTrue(strcmp(successful[i].expectedString, string) == 0,
                   "Expected %s actual %s", successful[i].expectedString, string);
        parcMemory_Deallocate((void **) &string);

        int64_t integerValue = parcJSONValue_GetInteger(expected);
        assertTrue(integerValue == (int64_t) successful[i].integerValue,
                   "Expected %" PRIi64 " actual %" PRIi64 "", (int64_t) successful[i].integerValue, integerValue);

        parcJSONValue_Release(&expected);
        parcJSONParser_Release(&parser);
    }
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_Comma)
{
    char *string = ", null";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_Parser(parser);

    assertNull(actual, "Expected parcJSONValue_Parser to return NULL when encountering a comma");

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_CloseBracket)
{
    char *string = "], null";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_Parser(parser);

    assertNull(actual, "Expected parcJSONValue_Parser to return NULL when encountering a ]");

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_Null)
{
    char *string = " null";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_Parser(parser);

    assertTrue(parcJSONValue_IsNull(actual),
               "Expected parcJSONValue_Parser to return a Null JSON value when encountering 'null'");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_True)
{
    char *string = " true";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_Parser(parser);

    assertTrue(parcJSONValue_IsBoolean(actual),
               "Expected parcJSONValue_Parser to return a boolean JSON value when encountering 'true'");
    assertTrue(parcJSONValue_GetBoolean(actual),
               "Expected true");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_False)
{
    char *string = " false";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_Parser(parser);

    assertTrue(parcJSONValue_IsBoolean(actual),
               "Expected parcJSONValue_Parser to return a boolean JSON value when encountering 'false'");
    assertFalse(parcJSONValue_GetBoolean(actual),
                "Expected true");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_String)
{
    char *string = " \"string\"";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_Parser(parser);

    assertTrue(parcJSONValue_IsString(actual),
               "Expected parcJSONValue_Parser to return a string JSON value");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_Array)
{
    char *string = " [ ]";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *value = parcJSONValue_Parser(parser);

    assertTrue(parcJSONValue_IsArray(value),
               "Expected parcJSONValue_Parser to return a array JSON value");

    PARCJSONArray *array = parcJSONValue_GetArray(value);
    assertNotNull(array, "Expected a non-null pointer to a PARCJSONArray");

    parcJSONValue_Release(&value);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONValueParsing, parcJSONValue_Parser_Object)
{
    char *string = " { }";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    PARCJSONValue *actual = parcJSONValue_Parser(parser);

    assertTrue(parcJSONValue_IsJSON(actual),
               "Expected parcJSONValue_Parser to return a JSON object value");

    parcJSONValue_Release(&actual);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_FIXTURE(Static)
{
    LONGBOW_RUN_TEST_CASE(Static, _parseSign_Negative);
    LONGBOW_RUN_TEST_CASE(Static, _parseSign_NotASign);
    LONGBOW_RUN_TEST_CASE(Static, _parseSign_Nil);

    LONGBOW_RUN_TEST_CASE(Static, _parseWholeNumber);
    LONGBOW_RUN_TEST_CASE(Static, _parseOptionalFraction);
    LONGBOW_RUN_TEST_CASE(Static, _parseOptionalExponent);
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Static, _parseSign_Negative)
{
    char *string = "-";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);
    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);
    int sign;
    bool result = _parseSign(parser, &sign);

    assertTrue(result, "Expected true from _parseSign()");

    parcJSONParser_Release(&parser);
}

LONGBOW_TEST_CASE(Static, _parseSign_NotASign)
{
    char *string = "asd";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);
    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);
    int sign;
    bool result = _parseSign(parser, &sign);

    assertFalse(result, "Expected true from _parseSign()");

    parcJSONParser_Release(&parser);
}

LONGBOW_TEST_CASE(Static, _parseSign_Nil)
{
    char *string = "";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);
    PARCJSONParser *parser = parcJSONParser_Create(buffer);
    parcBuffer_Release(&buffer);
    int sign;
    bool result = _parseSign(parser, &sign);

    assertTrue(result, "Expected true from _parseSign()");

    parcJSONParser_Release(&parser);
}

LONGBOW_TEST_CASE(Static, _parseWholeNumber)
{
    struct test_values {
        char *string;
        long double value;
    } successful[] = {
        { "0",   0   },
        { "1",   1   },
        { "123", 123 },
        { NULL },
    };

    for (int i = 0; successful[i].string != NULL; i++) {
        PARCBuffer *buffer = parcBuffer_WrapCString(successful[i].string);
        PARCJSONParser *parser = parcJSONParser_Create(buffer);
        parcBuffer_Release(&buffer);

        int64_t value = 0;

        bool actual = _parseWholeNumber(parser, &value);

        assertTrue(actual, "Expected true from _parseNumber()");
        assertTrue(value == successful[i].value,
                   "Expected %Lf actual %" PRIi64 "", successful[i].value, value);
        parcJSONParser_Release(&parser);
    }
}

LONGBOW_TEST_CASE(Static, _parseOptionalFraction)
{
    struct test_values {
        char *string;
        long double value;
        bool correct;
    } successful[] = {
        { ".0",   0, true  },
        { ".",    0, false },
        { ".1",   1, true  },
        { "crap", 0, false },
        { "}",    0, true  },
        { NULL },
    };

    for (int i = 0; successful[i].string != NULL; i++) {
        PARCBuffer *buffer = parcBuffer_WrapCString(successful[i].string);
        PARCJSONParser *parser = parcJSONParser_Create(buffer);
        parcBuffer_Release(&buffer);

        int64_t value = 0;
        int log10OfFraction;

        bool actual = _parseOptionalFraction(parser, &value, &log10OfFraction);

        assertTrue(actual == successful[i].correct, "Expected true from _parseNumber()");
        assertTrue(value == successful[i].value,
                   "Expected %Lf actual %" PRIi64 "", successful[i].value, value);
        parcJSONParser_Release(&parser);
    }
}

LONGBOW_TEST_CASE(Static, _parseOptionalExponent)
{
    struct test_values {
        char *string;
        long double value;
        bool correct;
    } successful[] = {
        { "e",   0,  false },
        { "ex",  0,  false },
        { "e-1", -1, true  },
        { "e1",  1,  true  },
        { "e+1", 1,  true  },
        { "x",   0,  false },
        { NULL },
    };

    for (int i = 0; successful[i].string != NULL; i++) {
        PARCBuffer *buffer = parcBuffer_WrapCString(successful[i].string);
        PARCJSONParser *parser = parcJSONParser_Create(buffer);
        parcBuffer_Release(&buffer);

        int64_t value = 0;

        bool actual = _parseOptionalExponent(parser, &value);

        assertTrue(actual == successful[i].correct, "Expected true from _parseNumber()");
        assertTrue(value == successful[i].value,
                   "Expected %Lf actual %" PRIi64 "", successful[i].value, value);
        parcJSONParser_Release(&parser);
    }
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_JSONValue);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
