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
#include "../parc_KeyStore.c"
#include <parc/testing/parc_ObjectTesting.h>

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_KeyStore)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_KeyStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_KeyStore)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_Acquire);
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_CreateFile);
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_GetFileName);
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_GetPassWord);
//    LONGBOW_RUN_TEST_CASE(Global, parcKeyStore_Release);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

//LONGBOW_TEST_CASE(Global, parcKeyStore_Acquire)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    parcObjectTesting_AssertAcquireReleaseContract(parcKeyStore_Acquire, keyStoreFile);
//
//    parcKeyStore_Release(&keyStoreFile);
//}
//
//LONGBOW_TEST_CASE(Global, parcKeyStore_CreateFile)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    parcKeyStore_Release(&keyStoreFile);
//}
//
//LONGBOW_TEST_CASE(Global, parcKeyStore_GetFileName)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    assertEqualStrings(keystoreName, parcKeyStore_GetFileName(keyStoreFile));
//
//    parcKeyStore_Release(&keyStoreFile);
//}
//
//LONGBOW_TEST_CASE(Global, parcKeyStore_GetPassWord)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    assertEqualStrings(keystorePassword, parcKeyStore_GetPassWord(keyStoreFile));
//
//    parcKeyStore_Release(&keyStoreFile);
//}
//
//LONGBOW_TEST_CASE(Global, parcKeyStore_Release)
//{
//    const char *keystoreName = "test_rsa.p12";
//    const char *keystorePassword = "blueberry";
//
//    PARCKeyStore *keyStoreFile = parcKeyStore_CreateFile(keystoreName, keystorePassword);
//
//    assertNotNull(keyStoreFile, "Expected non-null");
//
//    parcKeyStore_Release(&keyStoreFile);
//    assertNull(keyStoreFile, "Key store File was not nulled out after Release()");
//}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_KeyStore);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
