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

#include <ccnx/common/ccnx_Link.c>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(ccnx_Link)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_Link)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_Link)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_Create_Full);
    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_Create_EmptyKeyID);
    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_Create_EmptyContentObjectHash);
    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_Create_EmptyBoth);
    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_AcquireRelease);

    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_GetName);
    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_GetKeyID);
    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_GetContentObjectHash);

    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_Create_ToString);

    LONGBOW_RUN_TEST_CASE(Global, ccnxLink_Equals);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxLink_Create_Full)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = parcBuffer_Allocate(10);
    PARCBuffer *contentObjectHash = parcBuffer_Allocate(10);

    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);

    assertNotNull(object, "Expected non-null return value.");

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);
}

LONGBOW_TEST_CASE(Global, ccnxLink_Create_EmptyKeyID)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = NULL;
    PARCBuffer *contentObjectHash = parcBuffer_Allocate(10);

    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);

    assertNotNull(object, "Expected non-null return value.");

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
    parcBuffer_Release(&contentObjectHash);
}

LONGBOW_TEST_CASE(Global, ccnxLink_Create_EmptyContentObjectHash)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = parcBuffer_Allocate(10);
    PARCBuffer *contentObjectHash = NULL;

    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);

    assertNotNull(object, "Expected non-null return value.");

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
}

LONGBOW_TEST_CASE(Global, ccnxLink_Create_EmptyBoth)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = NULL;
    PARCBuffer *contentObjectHash = NULL;

    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);

    assertNotNull(object, "Expected non-null return value.");

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxLink_AcquireRelease)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = parcBuffer_Allocate(10);
    PARCBuffer *contentObjectHash = parcBuffer_Allocate(10);

    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);
    parcObjectTesting_AssertAcquireReleaseContract(ccnxLink_Acquire, object);

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);
}

LONGBOW_TEST_CASE(Global, ccnxLink_GetName)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = parcBuffer_Allocate(10);
    PARCBuffer *contentObjectHash = parcBuffer_Allocate(10);

    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);

    const CCNxName *actualName = ccnxLink_GetName(object);
    assertNotNull(actualName, "Expected non-null return value.");
    assertTrue(ccnxName_Equals(name, actualName), "Expected the same name back");

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);
}

LONGBOW_TEST_CASE(Global, ccnxLink_GetKeyID)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = parcBuffer_Allocate(10);
    PARCBuffer *contentObjectHash = parcBuffer_Allocate(20);

    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);

    PARCBuffer *buffer = ccnxLink_GetKeyID(object);
    assertNotNull(buffer, "Expected non-null return value.");
    assertTrue(parcBuffer_Capacity(buffer) == 10, "Expected the same buffer size back");

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);
}

LONGBOW_TEST_CASE(Global, ccnxLink_GetContentObjectHash)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = parcBuffer_Allocate(10);
    PARCBuffer *contentObjectHash = parcBuffer_Allocate(20);

    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);

    PARCBuffer *buffer = ccnxLink_GetContentObjectHash(object);
    assertNotNull(buffer, "Expected non-null return value.");
    assertTrue(parcBuffer_Capacity(buffer) == 20, "Expected the same buffer size back");

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);
}

LONGBOW_TEST_CASE(Global, ccnxLink_Equals)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = parcBuffer_Allocate(10);
    PARCBuffer *contentObjectHash = parcBuffer_Allocate(20);
    CCNxLink *x = ccnxLink_Create(name, keyId, contentObjectHash);
    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);

    name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    keyId = parcBuffer_Allocate(10);
    contentObjectHash = parcBuffer_Allocate(20);
    CCNxLink *y = ccnxLink_Create(name, keyId, contentObjectHash);
    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);

    name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    keyId = parcBuffer_Allocate(10);
    contentObjectHash = parcBuffer_Allocate(20);
    CCNxLink *z = ccnxLink_Create(name, keyId, contentObjectHash);
    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);

    name = ccnxName_CreateFromCString("lci:/foo/bar/othername");
    keyId = parcBuffer_Allocate(10);
    contentObjectHash = parcBuffer_Allocate(20);
    CCNxLink *unequal1 = ccnxLink_Create(name, keyId, contentObjectHash);
    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);

    name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    keyId = NULL;
    contentObjectHash = parcBuffer_Allocate(20);
    CCNxLink *unequal2 = ccnxLink_Create(name, keyId, contentObjectHash);
    ccnxName_Release(&name);
    parcBuffer_Release(&contentObjectHash);

    name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    keyId = parcBuffer_Allocate(10);
    contentObjectHash = NULL;
    CCNxLink *unequal3 = ccnxLink_Create(name, keyId, contentObjectHash);
    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);

    assertEqualsContract(ccnxLink_Equals, x, y, z, unequal1, unequal2, unequal3);

    ccnxLink_Release(&x);
    ccnxLink_Release(&y);
    ccnxLink_Release(&z);
    ccnxLink_Release(&unequal1);
    ccnxLink_Release(&unequal2);
    ccnxLink_Release(&unequal3);
}

LONGBOW_TEST_CASE(Global, ccnxLink_Create_ToString)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar/name");
    PARCBuffer *keyId = parcBuffer_Allocate(10);
    PARCBuffer *contentObjectHash = parcBuffer_Allocate(20);
    CCNxLink *object = ccnxLink_Create(name, keyId, contentObjectHash);

    char *string = ccnxLink_ToString(object);
    assertNotNull(string, "Expected non-null string.");
    parcMemory_Deallocate((void **) &string);

    ccnxLink_Release(&object);

    ccnxName_Release(&name);
    parcBuffer_Release(&keyId);
    parcBuffer_Release(&contentObjectHash);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_Link);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
