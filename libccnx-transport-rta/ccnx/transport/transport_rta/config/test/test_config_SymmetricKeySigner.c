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
#include "../config_SymmetricKeySigner.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>
#include "testrig_RtaConfigCommon.c"
#include <ccnx/transport/transport_rta/config/config_Signer.h>

LONGBOW_TEST_RUNNER(config_SymmetricKeySignerFileStore)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(config_SymmetricKeySignerFileStore)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(config_SymmetricKeySignerFileStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, symmetricKeySignerFileStore_ConnectionConfig);
    LONGBOW_RUN_TEST_CASE(Global, symmetricKeySignerFileStore_GetConnectionParams);
    LONGBOW_RUN_TEST_CASE(Global, symmetricKeySignerFileStore_GetName);
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

LONGBOW_TEST_CASE(Global, symmetricKeySignerFileStore_ConnectionConfig)
{
    CCNxConnectionConfig *connConfig = ccnxConnectionConfig_Create();
    const char *filename = "filename";
    const char *password = "password";
    symmetricKeySigner_ConnectionConfig(connConfig, filename, password);

    // make sure our stuff is in there
    testRtaConfiguration_ConnectionJsonKey(connConfig, symmetricKeySigner_GetName());

    // make sure the SIGNER parameter is in there
    testRtaConfiguration_ConnectionJsonKey(connConfig, signer_GetName());

    ccnxConnectionConfig_Destroy(&connConfig);
}

LONGBOW_TEST_CASE(Global, symmetricKeySignerFileStore_GetConnectionParams)
{
    CCNxConnectionConfig *connConfig = ccnxConnectionConfig_Create();
    const char *filename = "filename";
    const char *password = "password";
    symmetricKeySigner_ConnectionConfig(connConfig, filename, password);

    PARCJSON *json = ccnxConnectionConfig_GetJson(connConfig);
    struct symmetrickeysigner_params params;

    symmetricKeySigner_GetConnectionParams(json, &params);

    assertTrue(strncmp(params.filename, filename, strlen(filename)) == 0, "wrong filename, got %s expected %s", params.filename, filename);
    assertTrue(strncmp(params.password, password, strlen(password)) == 0, "wrong password, got %s expected %s", params.password, password);

    ccnxConnectionConfig_Destroy(&connConfig);
}

LONGBOW_TEST_CASE(Global, symmetricKeySignerFileStore_GetName)
{
    testRtaConfiguration_ComponentName(&symmetricKeySigner_GetName, name);
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
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(config_SymmetricKeySignerFileStore);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
