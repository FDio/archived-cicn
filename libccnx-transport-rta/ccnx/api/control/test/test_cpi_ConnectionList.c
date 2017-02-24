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

/*
 *
 */
#include "../cpi_ConnectionList.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>



LONGBOW_TEST_RUNNER(cpi_ConnectionList)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_ConnectionList)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_ConnectionList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionList_Append);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionList_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionList_FromJson);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionList_Equals);
    LONGBOW_RUN_TEST_CASE(Global, cpiConnectionList_ToJson);
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

static CPIConnection *
createConnectionObject(unsigned ifidx, int s_addr, uint16_t s_port, int d_addr, uint16_t d_port)
{
    return cpiConnection_Create(ifidx,
                                cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_family = PF_INET, .sin_addr.s_addr = s_addr, .sin_port = s_port }),
                                cpiAddress_CreateFromInet(&(struct sockaddr_in) { .sin_family = PF_INET, .sin_addr.s_addr = d_addr, .sin_port = d_port }),
                                cpiConnection_TCP);
}

LONGBOW_TEST_CASE(Global, cpiConnectionList_Append)
{
    CPIConnectionList *list = cpiConnectionList_Create();
    cpiConnectionList_Append(list, createConnectionObject(1, 2, 3, 4, 5));

    assertTrue(parcArrayList_Size(list->listOfConnections) == 1, "got wrong size, expected %u got %zu", 1, parcArrayList_Size(list->listOfConnections));

    cpiConnectionList_Destroy(&list);
}

LONGBOW_TEST_CASE(Global, cpiConnectionList_Create_Destroy)
{
    CPIConnectionList *list = cpiConnectionList_Create();
    cpiConnectionList_Destroy(&list);
    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance after create/destroy");
}

LONGBOW_TEST_CASE(Global, cpiConnectionList_FromJson)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char truth_string[] = "{\"ConnectionList\":[{\"Connection\":{\"IFIDX\":1,\"CONNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIDAAIAAAAAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIFAAQAAAAAAAAAAAAAAA==\"}}}]}";
#elif defined(__linux__)
    char truth_string[] = "{\"ConnectionList\":[{\"Connection\":{\"IFIDX\":1,\"CONNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgADAAIAAAAAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAFAAQAAAAAAAAAAAAAAA==\"}}}]}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif

    CPIConnectionList *truth_list = cpiConnectionList_Create();
    cpiConnectionList_Append(truth_list, createConnectionObject(1, 2, 3, 4, 5));

    PARCJSON *truth_json = parcJSON_ParseString(truth_string);
    CPIConnectionList *test_list = cpiConnectionList_FromJson(truth_json);

    assertTrue(cpiConnectionList_Equals(truth_list, test_list), "Lists do not match");

    cpiConnectionList_Destroy(&test_list);
    parcJSON_Release(&truth_json);
    cpiConnectionList_Destroy(&truth_list);
}

LONGBOW_TEST_CASE(Global, cpiConnectionList_Equals)
{
    CPIConnectionList *list_a = cpiConnectionList_Create();
    cpiConnectionList_Append(list_a, createConnectionObject(1, 2, 3, 4, 5));

    CPIConnectionList *list_b = cpiConnectionList_Create();
    cpiConnectionList_Append(list_b, createConnectionObject(1, 2, 3, 4, 5));

    CPIConnectionList *list_c = cpiConnectionList_Create();
    cpiConnectionList_Append(list_c, createConnectionObject(1, 2, 3, 4, 5));

    CPIConnectionList *unequal = cpiConnectionList_Create();
    cpiConnectionList_Append(unequal, createConnectionObject(99, 2, 3, 4, 5));
    cpiConnectionList_Append(unequal, createConnectionObject(1, 99, 3, 4, 5));
    cpiConnectionList_Append(unequal, createConnectionObject(1, 2, 99, 4, 5));
    cpiConnectionList_Append(unequal, createConnectionObject(1, 2, 3, 99, 5));
    cpiConnectionList_Append(unequal, createConnectionObject(1, 2, 3, 4, 99));

    assertEqualsContract(cpiConnectionList_Equals, list_a, list_b, list_c, unequal);

    cpiConnectionList_Destroy(&unequal);
    cpiConnectionList_Destroy(&list_a);
    cpiConnectionList_Destroy(&list_b);
    cpiConnectionList_Destroy(&list_c);
}

LONGBOW_TEST_CASE(Global, cpiConnectionList_ToJson)
{
    // The JSON representation depends on the system sockaddr_in format, which
    // varies platform to platform.
#if defined(__APPLE__)
    char truth_string[] = "{\"ConnectionList\":[{\"Connection\":{\"IFIDX\":1,\"CONNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIDAAIAAAAAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AAIFAAQAAAAAAAAAAAAAAA==\"}}}]}";
#elif defined(__linux__)
    char truth_string[] = "{\"ConnectionList\":[{\"Connection\":{\"IFIDX\":1,\"CONNTYPE\":\"TCP\",\"SRC\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgADAAIAAAAAAAAAAAAAAA==\"},\"DST\":{\"ADDRESSTYPE\":\"INET\",\"DATA\":\"AgAFAAQAAAAAAAAAAAAAAA==\"}}}]}";
#else
    // Case 1033
    testUnimplemented("Platform not supported");
    return;
#endif


    CPIConnectionList *list = cpiConnectionList_Create();
    cpiConnectionList_Append(list, createConnectionObject(1, 2, 3, 4, 5));

    PARCJSON *json = cpiConnectionList_ToJson(list);
    char *test = parcJSON_ToCompactString(json);
    assertTrue(strcmp(truth_string, test) == 0, "Got wrong JSON.\nexpected: %s\ngot %s\n", truth_string, test);
    parcMemory_Deallocate((void **) &test);

    parcJSON_Release(&json);
    cpiConnectionList_Destroy(&list);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_ConnectionList);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
