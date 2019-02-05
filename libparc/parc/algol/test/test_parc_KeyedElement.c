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

#include "../parc_KeyedElement.c"

#include <LongBow/unit-test.h>
#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(parc_KeyedElement)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_KeyedElement)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_KeyedElement)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcKeyedElement_CreateDestroy);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyedElement_GetData);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyedElement_GetKey);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyedElement_SetData);
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

LONGBOW_TEST_CASE(Global, parcKeyedElement_CreateDestroy)
{
    char *key1 = "key001";
    size_t keylen1 = 7;
    PARCKeyedElement *keyedElement = parcKeyedElement_Create("Some data", key1, keylen1);
    parcKeyedElement_Destroy(&keyedElement);
}

LONGBOW_TEST_CASE(Global, parcKeyedElement_GetData)
{
    char *initial_data = "some data here";
    char *key1 = "key001";
    size_t keylen1 = 7;
    PARCKeyedElement *keyedElement = parcKeyedElement_Create(initial_data, key1, keylen1);
    char *data = parcKeyedElement_GetData(keyedElement);
    assertTrue(initial_data == data, "We got different data from element");
    parcKeyedElement_Destroy(&keyedElement);
}

LONGBOW_TEST_CASE(Global, parcKeyedElement_GetKey)
{
    char *initial_data = "some data here";
    char *key1 = "key001";
    size_t keylen1 = 7;
    PARCKeyedElement *keyedElement = parcKeyedElement_Create(initial_data, key1, keylen1);
    char *thekey = parcKeyedElement_GetKey(keyedElement);
    size_t thekeylen = parcKeyedElement_GetKeyLen(keyedElement);
    assertTrue(keylen1 == thekeylen, "We got different key size?");
    assertTrue(memcmp(key1, thekey, keylen1) == 0, "We got different keys?");
    parcKeyedElement_Destroy(&keyedElement);
}

LONGBOW_TEST_CASE(Global, parcKeyedElement_SetData)
{
    char *key1 = "key001";
    size_t keylen1 = 7;
    char *initial_data = "some data here";

    PARCKeyedElement *keyedElement = parcKeyedElement_Create("Hello World", key1, keylen1);

    parcKeyedElement_SetData(keyedElement, initial_data);
    char *data = parcKeyedElement_GetData(keyedElement);
    assertTrue(initial_data == data, "We got different data from element");
    parcKeyedElement_Destroy(&keyedElement);
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
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_KeyedElement);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
