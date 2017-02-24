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

/**
 */
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include <parc/security/parc_KeyId.c>

#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Security.h>
#include <parc/testing/parc_ObjectTesting.h>

const char *testStr = "hello world";

LONGBOW_TEST_RUNNER(test_parc_KeyId)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(test_parc_KeyId)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_KeyId)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcKeyId_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyId_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyId_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyId_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyId_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyId_HashCodeFromVoid);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyId_GetKeyId);
    LONGBOW_RUN_TEST_CASE(Global, parcKeyId_ToString);
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

LONGBOW_TEST_CASE(Global, parcKeyId_Create)
{
    PARCBuffer *buffer = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *keyId = parcKeyId_Create(buffer);
    parcBuffer_Release(&buffer);

    assertNotNull(keyId, "Expected non-null");

    parcKeyId_Release(&keyId);
    assertNull(keyId, "Release did not null the pointer.");
}

LONGBOW_TEST_CASE(Global, parcKeyId_Acquire)
{
    PARCBuffer *buffer = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *keyId = parcKeyId_Create(buffer);
    parcBuffer_Release(&buffer);

    assertNotNull(keyId, "Expected non-null");
    parcObjectTesting_AssertAcquireReleaseContract(parcKeyId_Acquire, keyId);

    parcKeyId_Release(&keyId);
    assertNull(keyId, "Release did not null the pointer.");
}

LONGBOW_TEST_CASE(Global, parcKeyId_Copy)
{
    PARCBuffer *buffer = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *keyId = parcKeyId_Create(buffer);
    parcBuffer_Release(&buffer);

    assertNotNull(keyId, "Expected non-null");

    PARCKeyId *copy = parcKeyId_Copy(keyId);
    parcKeyId_AssertValid(keyId);
    parcKeyId_AssertValid(copy);

    parcKeyId_Release(&keyId);
    assertNull(keyId, "parcKeyId_Release did not null the keyId pointer.");

    parcKeyId_AssertValid(copy);
    parcKeyId_Release(&copy);
    assertNull(keyId, "parcKeyId_Release did not null the keyId copy pointer.");
}

LONGBOW_TEST_CASE(Global, parcKeyId_Equals)
{
    PARCBuffer *buffer1 = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *x = parcKeyId_Create(buffer1);
    parcBuffer_Release(&buffer1);

    PARCBuffer *buffer2 = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *y = parcKeyId_Create(buffer2);
    parcBuffer_Release(&buffer2);

    PARCBuffer *buffer3 = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *z = parcKeyId_Create(buffer3);
    parcBuffer_Release(&buffer3);

    PARCBuffer *buffer4 = parcBuffer_Wrap("hello worlx", 11, 0, 11);
    PARCKeyId *u1 = parcKeyId_Create(buffer4);
    parcBuffer_Release(&buffer4);

    parcObjectTesting_AssertEqualsFunction(parcKeyId_Equals, x, y, z, u1);

    parcKeyId_Release(&x);
    parcKeyId_Release(&y);
    parcKeyId_Release(&z);
    parcKeyId_Release(&u1);

    assertNull(x, "Release did not null the pointer.");
    assertNull(y, "Release did not null the pointer.");
    assertNull(z, "Release did not null the pointer.");
    assertNull(u1, "Release did not null the pointer.");
}

LONGBOW_TEST_CASE(Global, parcKeyId_HashCode)
{
    PARCBuffer *buffer = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *keyId = parcKeyId_Create(buffer);

    assertNotNull(keyId, "Expected non-null");

    assertTrue(parcKeyId_HashCode(keyId) == parcBuffer_HashCode(buffer), "Expected hash codes to be equal");

    parcBuffer_Release(&buffer);
    parcKeyId_Release(&keyId);
    assertNull(keyId, "Release did not null the pointer.");
}


LONGBOW_TEST_CASE(Global, parcKeyId_HashCodeFromVoid)
{
    PARCBuffer *buffer = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *keyId = parcKeyId_Create(buffer);

    assertNotNull(keyId, "Expected non-null");

    assertTrue(parcKeyId_HashCodeFromVoid((void *) keyId) == parcBuffer_HashCode(buffer), "Expected hash codes to be equal");

    parcBuffer_Release(&buffer);
    parcKeyId_Release(&keyId);
    assertNull(keyId, "Release did not null the pointer.");
}

LONGBOW_TEST_CASE(Global, parcKeyId_GetKeyId)
{
    PARCBuffer *buffer = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *keyId = parcKeyId_Create(buffer);

    assertNotNull(keyId, "Expected non-null");

    PARCBuffer *rawBuffer = (PARCBuffer *) parcKeyId_GetKeyId(keyId);
    assertTrue(parcBuffer_Equals(rawBuffer, buffer), "Expected the raw key buffers to be equal");

    parcBuffer_Release(&buffer);
    parcKeyId_Release(&keyId);
    assertNull(keyId, "Release did not null the pointer.");
}

LONGBOW_TEST_CASE(Global, parcKeyId_ToString)
{
    PARCBuffer *buffer = parcBuffer_Wrap((void *) testStr, strlen(testStr), 0, strlen(testStr));
    PARCKeyId *keyId = parcKeyId_Create(buffer);

    parcBuffer_Release(&buffer);

    assertNotNull(keyId, "Expected non-null");

    char *string = parcKeyId_ToString(keyId);
    printf("Hello: %s\n", string);
    parcMemory_Deallocate((void **) &string);

    parcKeyId_Release(&keyId);
    assertNull(keyId, "Release did not null the pointer.");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_KeyId);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
