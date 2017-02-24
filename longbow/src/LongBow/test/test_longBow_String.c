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
#include <stdio.h>
#include <inttypes.h>

#include "../private/longBow_String.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(longBow_String)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(longBow_String)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(longBow_String)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, longBowString_Create);
    LONGBOW_RUN_TEST_CASE(Global, longBowString_Append);
    LONGBOW_RUN_TEST_CASE(Global, longBowString_Append_Append);
    LONGBOW_RUN_TEST_CASE(Global, longBowString_Format);

    LONGBOW_RUN_TEST_CASE(Global, longBowString_StartsWith_True);
    LONGBOW_RUN_TEST_CASE(Global, longBowString_StartsWith_False);
    LONGBOW_RUN_TEST_CASE(Global, longBowString_Tokenise);
    LONGBOW_RUN_TEST_CASE(Global, longBowString_Tokenise_empty);
    LONGBOW_RUN_TEST_CASE(Global, longBowString_Tokenise_NULL);
    LONGBOW_RUN_TEST_CASE(Global, longBowString_CoreDump);
}

static uint64_t _outstandingAllocations;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _outstandingAllocations = longBowMemory_OutstandingAllocations();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (longBowMemory_OutstandingAllocations() > _outstandingAllocations) {
        printf("%s: memory leak\n", longBowTestCase_GetName(testCase));
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, longBowString_Create)
{
    LongBowString *string = longBowString_Create(128);
    assertNotNull(string, "Expected non-NULL result from longBowString_Create");

    longBowString_Destroy(&string);
    assertNull(string, "Expected the instance pointer to be NULL after longBowString_Destroy");
}

LONGBOW_TEST_CASE(Global, longBowString_Append)
{
    char *expected = "Hello World";
    LongBowString *string = longBowString_Create(0);
    longBowString_Append(string, expected);

    assertTrue(strcmp(expected, string->buffer) == 0,
               "Expected buffer to contain '%s', actual '%s'", expected, string->buffer);
    longBowString_Destroy(&string);
}

LONGBOW_TEST_CASE(Global, longBowString_Append_Append)
{
    char *expected = "Hello World";
    LongBowString *string = longBowString_Create(0);
    longBowString_Append(string, "Hello");
    longBowString_Append(string, " ");
    longBowString_Append(string, "World");

    assertTrue(strcmp(expected, string->buffer) == 0,
               "Expected buffer to contain '%s', actual '%s'", expected, string->buffer);
    longBowString_Destroy(&string);
}

LONGBOW_TEST_CASE(Global, longBowString_Format)
{
    char *expected = "Hello World";
    LongBowString *string = longBowString_Create(0);
    longBowString_Format(string, "%s", expected);

    assertTrue(strcmp(expected, string->buffer) == 0,
               "Expected buffer to contain '%s', actual '%s'", expected, string->buffer);
    longBowString_Destroy(&string);
}

LONGBOW_TEST_CASE(Global, longBowString_StartsWith_True)
{
    bool actual = longBowString_StartsWith("abcde", "abc");
    assertTrue(actual, "Expected true");
}

LONGBOW_TEST_CASE(Global, longBowString_StartsWith_False)
{
    bool actual = longBowString_StartsWith("abcde", "ayz");

    assertFalse(actual, "Expected false");
}

LONGBOW_TEST_CASE(Global, longBowString_Tokenise)
{
    LongBowArrayList *actual = longBowString_Tokenise("--t.x=10", "-=");

    assertTrue(strcmp("t.x", longBowArrayList_Get(actual, 0)) == 0,
               "Expected first token to be t.x, actual %s", (char *) longBowArrayList_Get(actual, 0));
    assertTrue(strcmp("10", longBowArrayList_Get(actual, 1)) == 0,
               "Expected first token to be 10, actual %s", (char *) longBowArrayList_Get(actual, 1));

    longBowArrayList_Destroy(&actual);
}

LONGBOW_TEST_CASE(Global, longBowString_Tokenise_empty)
{
    LongBowArrayList *actual = longBowString_Tokenise("", "-=");

    assertTrue(longBowArrayList_Length(actual) == 0, "Expected zero length LongBowArrayList, actual %zd", longBowArrayList_Length(actual));

    longBowArrayList_Destroy(&actual);
}

LONGBOW_TEST_CASE(Global, longBowString_Tokenise_NULL)
{
    LongBowArrayList *actual = longBowString_Tokenise(NULL, "-=");

    assertTrue(longBowArrayList_Length(actual) == 0, "Expected zero length LongBowArrayList, actual %zd", longBowArrayList_Length(actual));

    longBowArrayList_Destroy(&actual);
}

LONGBOW_TEST_CASE(Global, longBowString_CoreDump)
{
    //assertFalse(true, "foo");
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
    LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(longBow_String);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
