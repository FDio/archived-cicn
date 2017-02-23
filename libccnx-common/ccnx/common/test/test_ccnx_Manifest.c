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
#include "../ccnx_Manifest.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

#include <parc/testing/parc_ObjectTesting.h>

#include <ccnx/common/ccnx_Interest.h>

typedef struct test_data {
    CCNxManifest *object;
    CCNxManifest *manifestWithNamelessGroup;
    CCNxManifest *nameless;
    PARCLinkedList *interestListFromGroupLocator;
    PARCLinkedList *interestListFromManifestLocator;
    PARCLinkedList *interestListFromOverrideLocator;
    CCNxName *overrideLocator;
} ManifestTestData;

static ManifestTestData *
_commonSetup(void)
{
    ManifestTestData *data = parcMemory_AllocateAndClear(sizeof(ManifestTestData));

    data->overrideLocator = ccnxName_CreateFromCString("ccnx:/override");

    CCNxName *name = ccnxName_CreateFromCString("ccnx:/my/manifest");
    CCNxManifest *manifest = ccnxManifest_Create(name);
    CCNxManifest *nameless = ccnxManifest_CreateNameless();
    CCNxManifest *manifestWithNamelessGroup = ccnxManifest_Create(name);

    data->interestListFromGroupLocator = parcLinkedList_Create();
    data->interestListFromManifestLocator = parcLinkedList_Create();
    data->interestListFromOverrideLocator = parcLinkedList_Create();
    data->object = manifest;
    data->nameless = nameless;
    data->manifestWithNamelessGroup = manifestWithNamelessGroup;

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    CCNxManifestHashGroup *namelessGroup = ccnxManifestHashGroup_Create();

    CCNxName *locator = ccnxName_CreateFromCString("ccnx:/locator");
    ccnxManifestHashGroup_SetLocator(group, locator);

    // Create pointers for the pieces of data
    PARCBuffer *digest1 = parcBuffer_Allocate(32);
    PARCBuffer *digest2 = parcBuffer_Allocate(32);
    ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Data, digest1);
    ccnxManifestHashGroup_AppendPointer(namelessGroup, CCNxManifestHashGroupPointerType_Data, digest1);
    ccnxManifestHashGroup_AppendPointer(group, CCNxManifestHashGroupPointerType_Manifest, digest2);
    ccnxManifestHashGroup_AppendPointer(namelessGroup, CCNxManifestHashGroupPointerType_Data, digest2);

    // Create the corresponding interests based on the three locator cases
    // 1. The locator is inherited from the hash group
    // 2. The locator is inherited from the manifest
    // 3. The locator is overridden
    CCNxInterest *interest1 = ccnxInterest_CreateSimple(locator);
    ccnxInterest_SetContentObjectHashRestriction(interest1, digest1);
    parcLinkedList_Append(data->interestListFromGroupLocator, interest1);
    ccnxInterest_Release(&interest1);

    interest1 = ccnxInterest_CreateSimple(name);
    ccnxInterest_SetContentObjectHashRestriction(interest1, digest1);
    parcLinkedList_Append(data->interestListFromManifestLocator, interest1);
    ccnxInterest_Release(&interest1);

    interest1 = ccnxInterest_CreateSimple(data->overrideLocator);
    ccnxInterest_SetContentObjectHashRestriction(interest1, digest1);
    parcLinkedList_Append(data->interestListFromOverrideLocator, interest1);
    ccnxInterest_Release(&interest1);

    CCNxInterest *interest2 = ccnxInterest_CreateSimple(locator);
    ccnxInterest_SetContentObjectHashRestriction(interest2, digest2);
    parcLinkedList_Append(data->interestListFromGroupLocator, interest2);
    ccnxInterest_Release(&interest2);

    interest2 = ccnxInterest_CreateSimple(name);
    ccnxInterest_SetContentObjectHashRestriction(interest2, digest2);
    parcLinkedList_Append(data->interestListFromManifestLocator, interest2);
    ccnxInterest_Release(&interest2);

    interest2 = ccnxInterest_CreateSimple(data->overrideLocator);
    ccnxInterest_SetContentObjectHashRestriction(interest2, digest2);
    parcLinkedList_Append(data->interestListFromOverrideLocator, interest2);
    ccnxInterest_Release(&interest2);

    ccnxName_Release(&name);
    ccnxName_Release(&locator);
    parcBuffer_Release(&digest1);
    parcBuffer_Release(&digest2);

    ccnxManifest_AddHashGroup(manifest, group);
    ccnxManifest_AddHashGroup(manifestWithNamelessGroup, namelessGroup);
    ccnxManifest_AddHashGroup(nameless, namelessGroup);

    ccnxManifestHashGroup_Release(&group);
    ccnxManifestHashGroup_Release(&namelessGroup);

    return data;
}

static void
_commonTeardown(ManifestTestData *data)
{
    ccnxManifest_Release(&data->object);
    ccnxManifest_Release(&data->nameless);
    ccnxManifest_Release(&data->manifestWithNamelessGroup);

    parcLinkedList_Release(&data->interestListFromGroupLocator);
    parcLinkedList_Release(&data->interestListFromManifestLocator);
    parcLinkedList_Release(&data->interestListFromOverrideLocator);

    ccnxName_Release(&data->overrideLocator);
    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_RUNNER(ccnx_Manifest)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_Manifest)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_Manifest)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_Create);
    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_AddHashGroup);
    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_GetHashGroup);
    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_GetNumberOfHashGroups);
    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_GetName);
    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_ToString);

    LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_CreateInterestList_OverrideLocator);
    // LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_CreateInterestList_ManifestLocator);
    // LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_CreateInterestList_GroupLocator);
    // LONGBOW_RUN_TEST_CASE(Global, ccnxManifest_CreateInterestList_NoLocator);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxManifest_AcquireRelease)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/my/manifest");
    CCNxManifest *manifest = ccnxManifest_Create(name);
    ccnxName_Release(&name);

    parcObjectTesting_AssertAcquireReleaseContract(ccnxManifest_Acquire, manifest);

    ccnxManifest_Release(&manifest);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_Create)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/my/manifest");
    CCNxManifest *manifest = ccnxManifest_Create(name);

    assertNotNull(manifest, "Expected the Manifest to be created without errors.");
    const CCNxName *copy = ccnxManifest_GetName(manifest);

    assertTrue(ccnxName_Equals(name, copy), "Expected name to equal %s, got %s", ccnxName_ToString(name), ccnxName_ToString(copy));

    ccnxName_Release(&name);
    ccnxManifest_Release(&manifest);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_AddHashGroup)
{
    ManifestTestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxManifest *manifest = data->object;

    size_t numGroups = ccnxManifest_GetNumberOfHashGroups(manifest);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    ccnxManifest_AddHashGroup(manifest, group);

    size_t expected = numGroups + 1;
    size_t actual = ccnxManifest_GetNumberOfHashGroups(manifest);
    assertTrue(actual == expected, "Expected %zu, got %zu", expected, actual);

    ccnxManifestHashGroup_Release(&group);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_GetHashGroup)
{
    ManifestTestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxManifest *manifest = data->object;

    CCNxManifestHashGroup *group = ccnxManifest_GetHashGroupByIndex(manifest, 0);
    CCNxName *expected = ccnxName_CreateFromCString("ccnx:/locator");
    const CCNxName *actual = ccnxManifestHashGroup_GetLocator(group);
    assertTrue(ccnxName_Equals(expected, actual), "Expected %s, got %s", ccnxName_ToString(expected), ccnxName_ToString(actual));

    ccnxName_Release(&expected);
    ccnxManifestHashGroup_Release(&group);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_GetNumberOfHashGroups)
{
    ManifestTestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxManifest *manifest = data->object;

    size_t before = ccnxManifest_GetNumberOfHashGroups(manifest);
    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    ccnxManifest_AddHashGroup(manifest, group);

    size_t actual = ccnxManifest_GetNumberOfHashGroups(manifest);
    size_t expected = before + 1;

    assertTrue(expected == actual, "Expected %zu, got %zu", expected, actual);

    ccnxManifestHashGroup_Release(&group);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_CreateInterestList_NoLocator)
{
    ManifestTestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxManifest *nameless = data->nameless;

    PARCLinkedList *interestList = ccnxManifest_CreateInterestList(nameless, NULL);
    assertTrue(parcLinkedList_Size(interestList) == 0, "Expected the interest list to be empty since there was no valid locator");
    parcLinkedList_Release(&interestList);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_CreateInterestList_GroupLocator)
{
    ManifestTestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxManifest *manifest = data->object;

    PARCLinkedList *interestList = ccnxManifest_CreateInterestList(manifest, NULL);
    assertTrue(parcLinkedList_Equals(interestList, data->interestListFromGroupLocator), "Expected the interest lists to be equal");
    parcLinkedList_Release(&interestList);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_CreateInterestList_ManifestLocator)
{
    ManifestTestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxManifest *manifestWithNamelessGroup = data->manifestWithNamelessGroup;

    PARCLinkedList *interestList = ccnxManifest_CreateInterestList(manifestWithNamelessGroup, NULL);
    assertTrue(parcLinkedList_Equals(interestList, data->interestListFromManifestLocator), "Expected the interest lists to be equal");
    parcLinkedList_Release(&interestList);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_CreateInterestList_OverrideLocator)
{
    ManifestTestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxManifest *nameless = data->nameless;
    CCNxName *overrideLocator = data->overrideLocator;

    PARCLinkedList *interestList = ccnxManifest_CreateInterestList(nameless, overrideLocator);
    assertTrue(parcLinkedList_Equals(interestList, data->interestListFromOverrideLocator), "Expected the interest lists to be equal");
    parcLinkedList_Release(&interestList);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_GetName)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/my/manifest");
    CCNxManifest *manifest = ccnxManifest_Create(name);

    assertNotNull(manifest, "Expected the Manifest to be created without errors.");
    const CCNxName *copy = ccnxManifest_GetName(manifest);

    assertTrue(ccnxName_Equals(name, copy), "Expected name to equal %s, got %s", ccnxName_ToString(name), ccnxName_ToString(copy));

    ccnxName_Release(&name);
    ccnxManifest_Release(&manifest);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_Equals)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/my/manifest");
    CCNxManifest *x = ccnxManifest_Create(name);
    CCNxManifest *y = ccnxManifest_Create(name);
    CCNxManifest *z = ccnxManifest_Create(name);

    CCNxName *name1 = ccnxName_CreateFromCString("ccnx:/not/my/manifest");
    CCNxManifest *u1 = ccnxManifest_Create(name1);

    parcObjectTesting_AssertEqualsFunction(ccnxManifest_Equals, x, y, z, u1, NULL);

    ccnxName_Release(&name);
    ccnxName_Release(&name1);
    ccnxManifest_Release(&x);
    ccnxManifest_Release(&y);
    ccnxManifest_Release(&z);
    ccnxManifest_Release(&u1);
}

LONGBOW_TEST_CASE(Global, ccnxManifest_ToString)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/my/manifest");
    CCNxManifest *manifest = ccnxManifest_Create(name);

    assertNotNull(manifest, "Expected the Manifest to be created without errors.");
    const CCNxName *copy = ccnxManifest_GetName(manifest);

    assertTrue(ccnxName_Equals(name, copy), "Expected name to equal %s, got %s", ccnxName_ToString(name), ccnxName_ToString(copy));

    ccnxName_Release(&name);
    ccnxManifest_Release(&manifest);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_Manifest);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
