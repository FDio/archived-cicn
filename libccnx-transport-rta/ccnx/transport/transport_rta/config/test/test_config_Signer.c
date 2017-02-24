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
#include "../config_Signer.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>
#include "testrig_RtaConfigCommon.c"

LONGBOW_TEST_RUNNER(config_Signer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(config_Signer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(config_Signer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, signer_GetImplementationType_PublicKey);
    LONGBOW_RUN_TEST_CASE(Global, signer_GetImplementationType_SymmetricKey);
    LONGBOW_RUN_TEST_CASE(Global, signer_GetImplementationType_Unknown);
    LONGBOW_RUN_TEST_CASE(Global, signer_GetName);
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

LONGBOW_TEST_CASE(Global, signer_GetImplementationType_PublicKey)
{
    CCNxConnectionConfig *connConfig = ccnxConnectionConfig_Create();
    const char *filename = "filename";
    const char *password = "password";
    publicKeySigner_ConnectionConfig(connConfig, filename, password);

    PARCJSON *json = ccnxConnectionConfig_GetJson(connConfig);

    SignerType type = signer_GetImplementationType(json);
    assertTrue(type == SignerType_PublicKeySigner, "Got wrong signer type, got %d expected %d", type, SignerType_PublicKeySigner);

    ccnxConnectionConfig_Destroy(&connConfig);
}

LONGBOW_TEST_CASE(Global, signer_GetImplementationType_SymmetricKey)
{
    CCNxConnectionConfig *connConfig = ccnxConnectionConfig_Create();
    const char *filename = "filename";
    const char *password = "password";
    symmetricKeySigner_ConnectionConfig(connConfig, filename, password);

    PARCJSON *json = ccnxConnectionConfig_GetJson(connConfig);

    SignerType type = signer_GetImplementationType(json);
    assertTrue(type == SignerType_SymmetricKeySigner, "Got wrong signer type, got %d expected %d", type, SignerType_SymmetricKeySigner);

    ccnxConnectionConfig_Destroy(&connConfig);
}

LONGBOW_TEST_CASE(Global, signer_GetImplementationType_Unknown)
{
    char *bogusSignerString = "{\"SIGNER\":\"BogusSigner\",\"BogusSigner\":{}}";

    PARCJSON *json = parcJSON_ParseString(bogusSignerString);

    SignerType type = signer_GetImplementationType(json);
    assertTrue(type == SignerType_Unknown, "Got wrong signer type, got %d expected %d", type, SignerType_Unknown);

    parcJSON_Release(&json);
}

LONGBOW_TEST_CASE(Global, signer_GetName)
{
    testRtaConfiguration_ComponentName(&signer_GetName, param_SIGNER);
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(config_Signer);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
