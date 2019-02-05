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
#include "../parc_Key.c"

#include <LongBow/unit-test.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_Key)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Errors);
}

LONGBOW_TEST_RUNNER_SETUP(parc_Key)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(parc_Key)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcKey_CreateFromDerEncodedPublicKey);
    LONGBOW_RUN_TEST_CASE(Global, parcKey_CreateFromSymmetricKey);
    LONGBOW_RUN_TEST_CASE(Global, parcKey_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcKey_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcKey_GetKey);
    LONGBOW_RUN_TEST_CASE(Global, parcKey_GetKeyId);
    LONGBOW_RUN_TEST_CASE(Global, parcKey_GetSigningAlgorithm);
    LONGBOW_RUN_TEST_CASE(Global, parcKey_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcKey_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcKey_Copy)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);

    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, bb_key);
    parcKey_AssertValid(key);

    PARCKey *copy = parcKey_Copy(key);
    parcKey_AssertValid(copy);

    assertTrue(parcKey_Equals(key, copy), "Expected the original key instance and its copy to be equal");

    parcKey_Release(&copy);
    parcKey_Release(&key);
    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_CASE(Global, parcKey_CreateFromDerEncodedPublicKey)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);

    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, bb_key);

    parcKey_AssertValid(key);

    parcKey_Release(&key);
    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_CASE(Global, parcKey_CreateFromSymmetricKey)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromSymmetricKey(keyid, PARCSigningAlgorithm_HMAC, bb_key);

    parcKey_AssertValid(key);

    parcKey_Release(&key);
    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_CASE(Global, parcKey_Equals)
{
    PARCBuffer *bb_id_1 = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid_1 = parcKeyId_Create(bb_id_1);
    parcBuffer_Release(&bb_id_1);

    PARCBuffer *bb_id_2 = parcBuffer_Wrap("chugga chugga", 13, 0, 13);
    PARCKeyId *keyid_2 = parcKeyId_Create(bb_id_2);
    parcBuffer_Release(&bb_id_2);

    PARCBufferComposer *composer1 = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer1, "quack quack");
    PARCBuffer *bb_key_1 = parcBufferComposer_ProduceBuffer(composer1);

    PARCKey *x = parcKey_CreateFromDerEncodedPublicKey(keyid_1, PARCSigningAlgorithm_RSA, bb_key_1);
    PARCKey *y = parcKey_CreateFromDerEncodedPublicKey(keyid_1, PARCSigningAlgorithm_RSA, bb_key_1);
    PARCKey *z = parcKey_CreateFromDerEncodedPublicKey(keyid_1, PARCSigningAlgorithm_RSA, bb_key_1);

    PARCBufferComposer *composer2 = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer2, "mew mew");
    PARCBuffer *bb_key_2 = parcBufferComposer_ProduceBuffer(composer2);
    PARCKey *u1 = parcKey_CreateFromDerEncodedPublicKey(keyid_1, PARCSigningAlgorithm_RSA, bb_key_2);
    PARCKey *u2 = parcKey_CreateFromDerEncodedPublicKey(keyid_2, PARCSigningAlgorithm_RSA, bb_key_1);

    parcObjectTesting_AssertEqualsFunction(parcKey_Equals, x, y, z, u1, u2);

    parcBuffer_Release(&bb_key_1);
    parcBufferComposer_Release(&composer1);
    parcBuffer_Release(&bb_key_2);
    parcBufferComposer_Release(&composer2);
    parcKeyId_Release(&keyid_1);
    parcKeyId_Release(&keyid_2);

    parcKey_Release(&x);
    parcKey_Release(&y);
    parcKey_Release(&z);
    parcKey_Release(&u1);
    parcKey_Release(&u2);
}

LONGBOW_TEST_CASE(Global, parcKey_GetKey)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, bb_key);

    parcKey_AssertValid(key);

    PARCBuffer *rawKey = parcKey_GetKey(key); // reference count is not incremented
    assertTrue(parcBuffer_Equals(rawKey, bb_key), "Expected the raw key buffers to be equal");

    parcKey_Release(&key);
    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_CASE(Global, parcKey_GetKeyId)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, bb_key);

    parcKey_AssertValid(key);

    PARCKeyId *rawKeyId = parcKey_GetKeyId(key); // reference count is not incremented
    assertTrue(parcKeyId_Equals(rawKeyId, keyid), "Expected the raw KeyID buffers to be equal");

    parcKey_Release(&key);
    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_CASE(Global, parcKey_GetSigningAlgorithm)
{
    // Check for PARCSigningAlgorithm_RSA value
    PARCBuffer *bb_id_1 = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid_1 = parcKeyId_Create(bb_id_1);
    parcBuffer_Release(&bb_id_1);

    PARCBufferComposer *composer1 = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer1, "quack quack");
    PARCBuffer *bb_key_1 = parcBufferComposer_ProduceBuffer(composer1);
    PARCKey *key_1 = parcKey_CreateFromDerEncodedPublicKey(keyid_1, PARCSigningAlgorithm_RSA, bb_key_1);

    parcKey_AssertValid(key_1);

    assertTrue((parcKey_GetSigningAlgorithm(key_1) == PARCSigningAlgorithm_RSA), "Signing Algorithms don't match");

    parcBuffer_Release(&bb_key_1);
    parcBufferComposer_Release(&composer1);
    parcKeyId_Release(&keyid_1);
    parcKey_Release(&key_1);

    // Check for PARCSigningAlgorithm_HMAC value
    PARCBuffer *bb_id_2 = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid_2 = parcKeyId_Create(bb_id_2);
    parcBuffer_Release(&bb_id_2);

    PARCBufferComposer *composer2 = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer2, "quack quack");
    PARCBuffer *bb_key_2 = parcBufferComposer_ProduceBuffer(composer2);
    PARCKey *key_2 = parcKey_CreateFromSymmetricKey(keyid_2, PARCSigningAlgorithm_HMAC, bb_key_2);

    parcKey_AssertValid(key_2);

    assertTrue((parcKey_GetSigningAlgorithm(key_2) == PARCSigningAlgorithm_HMAC), "Signing Algorithms don't match");

    parcBuffer_Release(&bb_key_2);
    parcBufferComposer_Release(&composer2);
    parcKeyId_Release(&keyid_2);
    parcKey_Release(&key_2);
}

LONGBOW_TEST_CASE(Global, parcKey_Acquire)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, bb_key);

    parcKey_AssertValid(key);

    parcObjectTesting_AssertAcquireReleaseContract(parcKey_Acquire, key);
    parcKey_Release(&key);
    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_CASE(Global, parcKey_ToString)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_RSA, bb_key);

    char *keyString = parcKey_ToString(key);

    assertNotNull(keyString, "Expected non-null key representation string");
    assertTrue(strlen(keyString) > 0, "Expected non-null key representation string");

    parcKey_Release(&key);
    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_FIXTURE(Errors)
{
    LONGBOW_RUN_TEST_CASE(Errors, parcKey_CreateFromDerEncodedPublicKey_InvalidAlgorithm);
    LONGBOW_RUN_TEST_CASE(Errors, parcKey_CreateFromSymmetricKey_InvalidAlgorithm);
}

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcKey_CreateFromDerEncodedPublicKey_InvalidAlgorithm, .event = &LongBowTrapIllegalValue)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyid, PARCSigningAlgorithm_HMAC, bb_key);

    assertNull(key, "This should not be reached"); //To avoid a warning

    // HMAC is an illegal value for this constructor

    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

LONGBOW_TEST_CASE_EXPECTS(Errors, parcKey_CreateFromSymmetricKey_InvalidAlgorithm, .event = &LongBowTrapIllegalValue)
{
    PARCBuffer *bb_id = parcBuffer_Wrap("choo choo", 9, 0, 9);
    PARCKeyId *keyid = parcKeyId_Create(bb_id);
    parcBuffer_Release(&bb_id);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "quack quack");
    PARCBuffer *bb_key = parcBufferComposer_ProduceBuffer(composer);
    PARCKey *key = parcKey_CreateFromSymmetricKey(keyid, PARCSigningAlgorithm_RSA, bb_key);

    assertNull(key, "This should not be reached"); //To avoid a warning

    // RSA/DSA are illegal values for this constructor

    parcBuffer_Release(&bb_key);
    parcBufferComposer_Release(&composer);
    parcKeyId_Release(&keyid);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Key);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
