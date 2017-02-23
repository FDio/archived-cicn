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

#include "../cpi_RouteEntryList.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(cpi_RouteEntryList)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_RouteEntryList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_RouteEntryList)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntryList_Append);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntryList_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntryList_FromJson);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntryList_FromJson_EmptyList);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntryList_Equals);
    LONGBOW_RUN_TEST_CASE(Global, cpiRouteEntryList_ToJson);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, cpiRouteEntryList_Append)
{
    CPIRouteEntryList *list = cpiRouteEntryList_Create();

    CPIRouteEntry *entry = cpiRouteEntry_Create(ccnxName_CreateFromCString("ccnx:/hello"), 1, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    cpiRouteEntryList_Append(list, entry);

    assertTrue(parcArrayList_Size(list->listOfRouteEntries) == 1, "got wrong size, expected %u got %zu", 1, parcArrayList_Size(list->listOfRouteEntries));

    cpiRouteEntryList_Destroy(&list);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntryList_Create_Destroy)
{
    CPIRouteEntryList *list = cpiRouteEntryList_Create();
    cpiRouteEntryList_Destroy(&list);
    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance after create/destroy");
}

LONGBOW_TEST_CASE(Global, cpiRouteEntryList_FromJson)
{
    char truth_string[] = "{\"Routes\":[{\"PREFIX\":\"ccnx:/hello\",\"INTERFACE\":1,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":1}]}";

    CPIRouteEntryList *truth_list = cpiRouteEntryList_Create();
    CPIRouteEntry *entry = cpiRouteEntry_Create(ccnxName_CreateFromCString("ccnx:/hello"), 1, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    cpiRouteEntryList_Append(truth_list, entry);

    PARCJSON *truth_json = parcJSON_ParseString(truth_string);
    CPIRouteEntryList *test_list = cpiRouteEntryList_FromJson(truth_json);

    assertTrue(cpiRouteEntryList_Equals(truth_list, test_list), "Lists do not match");

    cpiRouteEntryList_Destroy(&test_list);
    parcJSON_Release(&truth_json);
    cpiRouteEntryList_Destroy(&truth_list);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntryList_FromJson_EmptyList)
{
    char truth_string[] = "{\"Routes\":[]}";

    CPIRouteEntryList *truth_list = cpiRouteEntryList_Create();

    PARCJSON *truth_json = parcJSON_ParseString(truth_string);
    CPIRouteEntryList *test_list = cpiRouteEntryList_FromJson(truth_json);

    assertTrue(cpiRouteEntryList_Equals(truth_list, test_list), "Lists do not match");

    cpiRouteEntryList_Destroy(&test_list);
    parcJSON_Release(&truth_json);
    cpiRouteEntryList_Destroy(&truth_list);
}


LONGBOW_TEST_CASE(Global, cpiRouteEntryList_Equals)
{
    CPIRouteEntryList *list_a = cpiRouteEntryList_Create();
    CPIRouteEntry *entry_a = cpiRouteEntry_Create(ccnxName_CreateFromCString("ccnx:/hello"), 1, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    cpiRouteEntryList_Append(list_a, entry_a);

    CPIRouteEntryList *list_b = cpiRouteEntryList_Create();
    CPIRouteEntry *entry_b = cpiRouteEntry_Create(ccnxName_CreateFromCString("ccnx:/hello"), 1, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    cpiRouteEntryList_Append(list_b, entry_b);

    CPIRouteEntryList *list_c = cpiRouteEntryList_Create();
    CPIRouteEntry *entry_c = cpiRouteEntry_Create(ccnxName_CreateFromCString("ccnx:/hello"), 1, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    cpiRouteEntryList_Append(list_c, entry_c);

    CPIRouteEntryList *unequal_length = cpiRouteEntryList_Create();
    CPIRouteEntry *entry_d = cpiRouteEntry_Create(ccnxName_CreateFromCString("ccnx:/hello"), 1, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    CPIRouteEntry *entry_e = cpiRouteEntry_Create(ccnxName_CreateFromCString("ccnx:/hello"), 1, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    cpiRouteEntryList_Append(unequal_length, entry_d);
    cpiRouteEntryList_Append(unequal_length, entry_e);

    CPIRouteEntryList *unequal_value = cpiRouteEntryList_Create();
    CPIRouteEntry *entry_f = cpiRouteEntry_Create(ccnxName_CreateFromCString("ccnx:/hello"), 2, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    cpiRouteEntryList_Append(unequal_value, entry_f);

    assertEqualsContract(cpiRouteEntryList_Equals, list_a, list_b, list_c, unequal_length, unequal_value);

    cpiRouteEntryList_Destroy(&unequal_value);
    cpiRouteEntryList_Destroy(&unequal_length);

    cpiRouteEntryList_Destroy(&list_a);
    cpiRouteEntryList_Destroy(&list_b);
    cpiRouteEntryList_Destroy(&list_c);
}

LONGBOW_TEST_CASE(Global, cpiRouteEntryList_ToJson)
{
    char truth_string[] = "{\"Routes\":[{\"PREFIX\":\"ccnx:/hello\",\"INTERFACE\":1,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":1}]}";

    CPIRouteEntryList *list = cpiRouteEntryList_Create();

    CPIRouteEntry *entry = cpiRouteEntry_Create(ccnxName_CreateFromCString("lci:/hello"), 1, NULL, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, NULL, 1);
    cpiRouteEntryList_Append(list, entry);

    PARCJSON *json = cpiRouteEntryList_ToJson(list);
    char *test = parcJSON_ToCompactString(json);
    assertTrue(strcmp(truth_string, test) == 0, "Got wrong JSON.\nexpected: %s\ngot %s\n", truth_string, test);
    parcMemory_Deallocate((void **) &test);

    parcJSON_Release(&json);
    cpiRouteEntryList_Destroy(&list);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_RouteEntryList);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
