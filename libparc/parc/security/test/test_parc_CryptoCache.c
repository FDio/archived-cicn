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

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_CryptoCache.c"
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/security/parc_CryptoHashType.h>

LONGBOW_TEST_RUNNER(parc_CryptoCache)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Allocate);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_CryptoCache)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_CryptoCache)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Allocate)
{
    LONGBOW_RUN_TEST_CASE(Allocate, parcCryptoCache_Create_Destroy);
}

LONGBOW_TEST_FIXTURE_SETUP(Allocate)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Allocate)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Allocate, parcCryptoCache_Create_Destroy)
{
    PARCCryptoCache *cache = parcCryptoCache_Create();
    parcCryptoCache_Destroy(&cache);
}

static PARCCryptoCache *cache_under_test;

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoCache_AddGetKey);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoCache_GetMissingKey);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoCache_GetWrongKey);
    LONGBOW_RUN_TEST_CASE(Global, parcCryptoCache_RemoveKey);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    cache_under_test = parcCryptoCache_Create();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    parcCryptoCache_Destroy(&cache_under_test);

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcCryptoCache_AddGetKey)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, bb_key);

    parcCryptoCache_AddKey(cache_under_test, key);

    const PARCKey *test = parcCryptoCache_GetKey(cache_under_test, keyid);

    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);

    assertTrue(parcKey_Equals(key, test), "did not return expected key from cache");
    parcKey_Release(&key);
}

LONGBOW_TEST_CASE(Global, parcCryptoCache_GetMissingKey)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    const PARCKey *test = parcCryptoCache_GetKey(cache_under_test, keyid);

    assertNull(test, "Get missing key returned something!");

    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_CASE(Global, parcCryptoCache_GetWrongKey)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCBuffer *bb_id2 = parcBuffer_Wrap("not here!", 9, 0, 9);

    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCKeyId *keyid2 = parcKeyId_Create(bb_id2);
    parcBuffer_Release(&bb_id2);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, bb_key);

    parcCryptoCache_AddKey(cache_under_test, key);

    const PARCKey *test = parcCryptoCache_GetKey(cache_under_test, keyid2);
    assertNull(test, "Get missing key returned something!");

    parcKey_Release(&key);
    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
    parcKeyId_Release(&keyid2);
}

/**
 * Add in 2 keys, remove 1, fetch the other
 */
LONGBOW_TEST_CASE(Global, parcCryptoCache_RemoveKey)
{
    PARCBufferComposer *composer1 = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer1, "quack quack");
    PARCBuffer *bb_key1 = parcBufferComposer_ProduceBuffer(composer1);

    PARCBufferComposer *composer2 = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer2, "Come with me and you'll be");
    PARCBuffer *bb_key2 = parcBufferComposer_ProduceBuffer(composer2);

    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCBuffer *bb_id2 = parcBuffer_Wrap("not here!", 9, 0, 9);

    PARCKeyId *keyid1 = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);
    PARCKeyId *keyid2 = parcKeyId_Create(bb_id2);
    parcBuffer_Release(&bb_id2);

    PARCKey *key1 = parcKey_CreateFromDerEncodedPublicKey(keyid1, PARCSigningAlgorithm_RSA, bb_key1);
    PARCKey *key2 = parcKey_CreateFromDerEncodedPublicKey(keyid2, PARCSigningAlgorithm_RSA, bb_key2);
    const PARCKey *test;

    parcCryptoCache_AddKey(cache_under_test, key1);
    parcCryptoCache_AddKey(cache_under_test, key2);

    test = parcCryptoCache_GetKey(cache_under_test, keyid1);
    assertTrue(parcKey_Equals(key1, test), "Got wrong key");

    test = parcCryptoCache_GetKey(cache_under_test, keyid2);
    assertTrue(parcKey_Equals(key2, test), "Got wrong key");

    // remove will free the key, so make a copy of it before removing
    PARCKeyId *keyid1_copy = parcKeyId_Copy(keyid1);
    parcCryptoCache_RemoveKey(cache_under_test, keyid1);

    test = parcCryptoCache_GetKey(cache_under_test, keyid1_copy);
    assertNull(test, "Get of deleted key returned non-null");

    test = parcCryptoCache_GetKey(cache_under_test, keyid2);
    assertTrue(parcKey_Equals(key2, test), "Got wrong key");


    parcKey_Release(&key1);
    parcKey_Release(&key2);

    parcBuffer_Release(&bb_key1);
    parcBufferComposer_Release(&composer1);
    parcKeyId_Release(&keyid1);
    parcBuffer_Release(&bb_key2);
    parcBufferComposer_Release(&composer2);
    parcKeyId_Release(&keyid2);

    parcKeyId_Release(&keyid1_copy);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_CryptoCache);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
