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
#include "../rta_CommandOpenConnection.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

typedef struct test_data {
    int stackId;
    int apiNotifierFd;
    int transportNotifierFd;
    PARCJSON *config;

    RtaCommandOpenConnection *openConnection;
} TestData;

static TestData *
_createTestData(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));

    data->stackId = 7;
    data->apiNotifierFd = 11;
    data->transportNotifierFd = 10029;
    data->config = parcJSON_Create();

    data->openConnection = rtaCommandOpenConnection_Create(data->stackId, data->apiNotifierFd, data->transportNotifierFd, data->config);

    return data;
}

static void
_destroyTestData(TestData **dataPtr)
{
    TestData *data = *dataPtr;

    rtaCommandOpenConnection_Release(&data->openConnection);
    parcJSON_Release(&data->config);
    parcMemory_Deallocate((void **) &data);

    *dataPtr = NULL;
}

// =============================================================

LONGBOW_TEST_RUNNER(rta_CommandOpenConnection)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_CommandOpenConnection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_CommandOpenConnection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandOpenConnection_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandOpenConnection_Create);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandOpenConnection_GetApiNotifierFd);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandOpenConnection_GetConfig);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandOpenConnection_GetStackId);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandOpenConnection_GetTransportNotifierFd);
    LONGBOW_RUN_TEST_CASE(Global, rtaCommandOpenConnection_Release);
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

LONGBOW_TEST_CASE(Global, rtaCommandOpenConnection_Acquire)
{
    TestData *data = _createTestData();

    size_t firstRefCount = parcObject_GetReferenceCount(data->openConnection);

    RtaCommandOpenConnection *second = rtaCommandOpenConnection_Acquire(data->openConnection);
    size_t secondRefCount = parcObject_GetReferenceCount(second);

    assertTrue(secondRefCount == firstRefCount + 1, "Wrong refcount after acquire, got %zu expected %zu", secondRefCount, firstRefCount + 1);

    rtaCommandOpenConnection_Release(&second);
    _destroyTestData(&data);
}

LONGBOW_TEST_CASE(Global, rtaCommandOpenConnection_Create)
{
    TestData *data = _createTestData();
    assertNotNull(data->openConnection, "Got null from create");
    assertTrue(data->openConnection->stackId == data->stackId, "Internal stackId wrong, got %d expected %d",
               data->openConnection->stackId, data->stackId);
    assertTrue(data->openConnection->apiNotifierFd == data->apiNotifierFd, "Internal apiNotifierFd wrong, got %d expected %d",
               data->openConnection->apiNotifierFd, data->apiNotifierFd);
    assertTrue(data->openConnection->transportNotifierFd == data->transportNotifierFd, "Internal transportNotifierFd wrong, got %d expected %d",
               data->openConnection->transportNotifierFd, data->transportNotifierFd);
    assertTrue(parcJSON_Equals(data->openConnection->config, data->config), "JSON configs are not equal");
    _destroyTestData(&data);
}

LONGBOW_TEST_CASE(Global, rtaCommandOpenConnection_GetApiNotifierFd)
{
    TestData *data = _createTestData();

    int testApiFd = rtaCommandOpenConnection_GetApiNotifierFd(data->openConnection);
    assertTrue(testApiFd == data->apiNotifierFd, "Wrong value, got %d expected %d", testApiFd, data->apiNotifierFd);

    _destroyTestData(&data);
}

LONGBOW_TEST_CASE(Global, rtaCommandOpenConnection_GetConfig)
{
    TestData *data = _createTestData();

    PARCJSON *testConfig = rtaCommandOpenConnection_GetConfig(data->openConnection);
    assertTrue(parcJSON_Equals(data->config, testConfig), "Configurations do not match");

    _destroyTestData(&data);
}

LONGBOW_TEST_CASE(Global, rtaCommandOpenConnection_GetStackId)
{
    TestData *data = _createTestData();

    int testStackId = rtaCommandOpenConnection_GetStackId(data->openConnection);
    assertTrue(testStackId == data->stackId, "Wrong value, got %d expected %d", testStackId, data->stackId);

    _destroyTestData(&data);
}

LONGBOW_TEST_CASE(Global, rtaCommandOpenConnection_GetTransportNotifierFd)
{
    TestData *data = _createTestData();

    int testTransportFd = rtaCommandOpenConnection_GetTransportNotifierFd(data->openConnection);
    assertTrue(testTransportFd == data->transportNotifierFd, "Wrong value, got %d expected %d", testTransportFd, data->transportNotifierFd);

    _destroyTestData(&data);
}

LONGBOW_TEST_CASE(Global, rtaCommandOpenConnection_Release)
{
    TestData *data = _createTestData();

    RtaCommandOpenConnection *second = rtaCommandOpenConnection_Acquire(data->openConnection);
    size_t secondRefCount = parcObject_GetReferenceCount(second);

    rtaCommandOpenConnection_Release(&second);
    size_t thirdRefCount = parcObject_GetReferenceCount(data->openConnection);

    assertTrue(thirdRefCount == secondRefCount - 1, "Wrong refcount after release, got %zu expected %zu", thirdRefCount, secondRefCount - 1);

    _destroyTestData(&data);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_CommandOpenConnection);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
