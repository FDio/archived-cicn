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


#include "../cpi_CancelFlow.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>

#include <inttypes.h>



LONGBOW_TEST_RUNNER(cpi_CancelFlow)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(cpi_CancelFlow)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(cpi_CancelFlow)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, cpiCancelFlow_CreateRequest);
    LONGBOW_RUN_TEST_CASE(Global, cpiCancelFlow_NameFromControlMessage);
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

LONGBOW_TEST_CASE(Global, cpiCancelFlow_CreateRequest)
{
    const char truth_format[] = "{\"CPI_REQUEST\":{\"SEQUENCE\":%" PRIu64 ",\"CPI_CANCEL_FLOW\":{\"FLOW_NAME\":\"ccnx:/who/doesnt/like/pie\"}}}";

    CCNxName *name = ccnxName_CreateFromCString("lci:/who/doesnt/like/pie");
    PARCJSON *cpiRequest = cpiCancelFlow_CreateRequest(name);
    CCNxControl *controlRequest = ccnxControl_CreateCPIRequest(cpiRequest);

    PARCJSON *json = ccnxControl_GetJson(controlRequest);

    char buffer[1024];
    sprintf(buffer, truth_format, cpi_GetSequenceNumber(controlRequest));

    char *test_string = parcJSON_ToCompactString(json);
    assertTrue(strcmp(buffer, test_string) == 0, "Incorrect JSON, expected '%s' got '%s'", buffer, test_string);
    parcMemory_Deallocate((void **) &test_string);

    ccnxControl_Release(&controlRequest);
    parcJSON_Release(&cpiRequest);
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, cpiCancelFlow_NameFromControlMessage)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/who/doesnt/like/pie");
    PARCJSON *cpiRequest = cpiCancelFlow_CreateRequest(name);
    CCNxControl *controlRequest = ccnxControl_CreateCPIRequest(cpiRequest);

    CCNxName *test_name = cpiCancelFlow_NameFromControlMessage(controlRequest);
    assertTrue(ccnxName_Equals(test_name, name),
               "Expected %s actual %s",
               ccnxName_ToString(name),
               ccnxName_ToString(test_name));

    ccnxName_Release(&test_name);
    ccnxControl_Release(&controlRequest);
    parcJSON_Release(&cpiRequest);
    ccnxName_Release(&name);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(cpi_CancelFlow);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
