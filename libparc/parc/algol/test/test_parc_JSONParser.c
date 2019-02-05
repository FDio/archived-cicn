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

#include <stdio.h>
#include <fcntl.h>
#include <math.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.

#include "../parc_JSONParser.c"

#include <parc/algol/parc_JSONValue.h>

#include <parc/algol/parc_List.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_StdlibMemory.h>
#include <parc/algol/parc_Memory.h>

LONGBOW_TEST_RUNNER(parc_JSONParser)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(JSONParse_CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(JSONParse);
//    LONGBOW_RUN_TEST_FIXTURE(Performance);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_JSONParser)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_JSONParser)
{
    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_FIXTURE(JSONParse_CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(JSONParse_CreateAcquireRelease, parcJSONParser_Create);
    LONGBOW_RUN_TEST_CASE(JSONParse_CreateAcquireRelease, parcJSONParser_AcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(JSONParse_CreateAcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(JSONParse_CreateAcquireRelease)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(JSONParse_CreateAcquireRelease, parcJSONParser_Create)
{
    char *string = "\"string\"";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
    assertNull(parser,
               "Expected parcJSONParser_Release to set the reference pointer to NULL");
}

LONGBOW_TEST_CASE(JSONParse_CreateAcquireRelease, parcJSONParser_AcquireRelease)
{
    char *string = "\"string\"";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *expected = parcJSONParser_Create(buffer);

    PARCJSONParser *actual = parcJSONParser_Acquire(expected);
    assertTrue(actual == expected,
               "Expected the acquired reference to be the same as the original instance.");

    parcJSONParser_Release(&actual);
    assertNull(actual,
               "Expected parcJSONParser_Release to set the reference pointer to NULL");

    parcJSONParser_Release(&expected);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_FIXTURE(JSONParse)
{
    LONGBOW_RUN_TEST_CASE(JSONParse, parcJSONString_Parser);
    LONGBOW_RUN_TEST_CASE(JSONParse, parcJSONParser_RequireString_Fail);
    LONGBOW_RUN_TEST_CASE(JSONParse, parcJSONString_Parser_Quoted);
    LONGBOW_RUN_TEST_CASE(JSONParse, parcJSON_Parse);

    LONGBOW_RUN_TEST_CASE(JSONParse, parcJSON_ParseFile);
    LONGBOW_RUN_TEST_CASE(JSONParse, parcJSON_ParseFileToString);
}

LONGBOW_TEST_FIXTURE_SETUP(JSONParse)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(JSONParse)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("Errors %s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(JSONParse, parcJSONString_Parser)
{
    char *string = "\"\\\" \\\\ \\b \\f \\n \\r \\t \\/\"";
    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);

    PARCBuffer *expected = parcBuffer_AllocateCString("\" \\ \b \f \n \r \t /");
    PARCBuffer *actual = parcJSONParser_ParseString(parser);

    assertTrue(parcBuffer_Equals(expected, actual), "Expected string");

    parcBuffer_Release(&actual);
    parcBuffer_Release(&expected);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONParse, parcJSONParser_RequireString_Fail)
{
    char *string = "\"string\"";
    char *requiredString = "foo";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);

    bool actual = parcJSONParser_RequireString(parser, requiredString);

    assertFalse(actual, "Expected parcJSONParser_RequireString to fail");

    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONParse, parcJSONString_Parser_Quoted)
{
    char *string = "\"str\\\"ing\"";

    PARCBuffer *buffer = parcBuffer_WrapCString(string);

    PARCJSONParser *parser = parcJSONParser_Create(buffer);

    PARCBuffer *expected = parcBuffer_WrapCString("str\"ing");
    PARCBuffer *actual = parcJSONParser_ParseString(parser);

    assertTrue(parcBuffer_Equals(expected, actual), "Expected string");

    parcBuffer_Release(&actual);
    parcBuffer_Release(&expected);
    parcJSONParser_Release(&parser);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(JSONParse, parcJSON_Parse)
{
    char *expected = "{ \"string\" : \"string\", \"null\" : null, \"true\" : true, \"false\" : false, \"integer\" : 31415, \"float\" : 3.141500, \"array\" : [ null, false, true, 31415, \"string\", [ null, false, true, 31415, \"string\" ], {  } ] }";

    expected = "{ \"integer\" : 31415 }";

    expected = "{ \"string\" : \"string\", \"null\" : null, \"true\" : true, \"false\" : false, \"integer\" : 31415, \"array\" : [ null, false, true, 31415, \"string\", [ null, false, true, 31415, \"string\" ], { \"string\" : \"string\" } ] }";

    PARCJSON *json = parcJSON_ParseString(expected);
    assertNotNull(json, "Parse error for %s", expected);

    char *actual = parcJSON_ToString(json);

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);

    printf("%s\n", actual);
    parcMemory_Deallocate((void **) &actual);

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSONParse, parcJSON_ParseFile)
{
    char *string = NULL;
    size_t nread = longBowDebug_ReadFile("data.json", &string);
    assertTrue(nread != -1, "Cannot read '%s'", "data.json");

    PARCJSON *json = parcJSON_ParseString(string);

    assertNotNull(json, "parcJSON_ParseString failed");

    //    assertTrue(longBowDebug_WriteFile("/tmp/test_parc_JSON.json", actual, strlen(actual)) != 0,
    //               "Can't write file");

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(JSONParse, parcJSON_ParseFileToString)
{
    char *string = NULL;
    size_t nread = longBowDebug_ReadFile("data.json", &string);
    assertTrue(nread != -1, "Cannot read '%s'", "data.json");

    PARCJSON *json = parcJSON_ParseString(string);

    assertNotNull(json, "parcJSON_ParseString failed");

    char *actual = parcJSON_ToString(json);

    //    assertTrue(longBowDebug_WriteFile("/tmp/test_parc_JSON.json", actual, strlen(actual)) != 0,
    //               "Can't write file");

    parcMemory_Deallocate((void **) &actual);

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

LONGBOW_TEST_FIXTURE(Performance)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcJSON_ParseFileToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    parcMemory_SetInterface(&PARCStdlibMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Performance, parcJSON_ParseFileToString)
{
    char *string = NULL;
    size_t nread = longBowDebug_ReadFile("citylots.json", &string);
    assertTrue(nread != -1, "Cannot read '%s'", "citylots.json");

    PARCJSON *json = parcJSON_ParseString(string);

    assertNotNull(json, "parcJSON_ParseString failed");

    char *actual = parcJSON_ToString(json);

    //    assertTrue(longBowDebug_WriteFile("/tmp/test_parc_JSON.json", actual, strlen(actual)) != 0,
    //               "Can't write file");

    parcMemory_Deallocate((void **) &actual);

    parcJSON_Release(&json);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_JSONParser);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
